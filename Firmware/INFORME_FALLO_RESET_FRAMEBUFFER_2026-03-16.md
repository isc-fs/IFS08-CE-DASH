# Informe del fallo visual tras `reset`

Fecha: `2026-03-16`

## Resumen

El fallo que quedaba al final no era el mismo `ghosting` original por `LTDC FIFO underrun`.

La causa mas probable del problema que seguia apareciendo era:

- estado visual incoherente entre los dos `framebuffers` de TouchGFX despues de un `reset`
- reutilizacion de contenido viejo en SDRAM al volver a arrancar
- actualizacion parcial de widgets sobre un buffer ya correcto y otro buffer todavia "sucio"

En la practica, el valor central se movia fluido, pero el resto de elementos parecia parpadear o desaparecer por zonas porque no ambos buffers contenian la misma pantalla completa.

## Sintoma observado

- En arranque en frio, conectando alimentacion desde cero, la pantalla se veia bien.
- El fallo aparecia sobre todo al pulsar `reset`.
- El valor principal (`Accel`) cambiaba de forma fluida.
- El resto de cajas y textos laterales parpadeaban.

Este patron encaja mejor con un problema de inicializacion/siembra de `double buffer` que con falta de ancho de banda.

## Evidencia que llevo al diagnostico

### 1. La telemetria y la UI si estaban corriendo

Por UART se vio un patron estable como:

```text
[DISP] 1s fu=0(+0) vs=52 sw=52 fp=52 uiTick=52 uiPush=10 ...
```

Eso significa:

- `fu=0(+0)`: sin `FIFO underrun`
- `vs/sw/fp` estables: LTDC y swap funcionando
- `uiTick` cercano a `vs`: TouchGFX acompasado con el refresco
- `uiPush` bajo pero correcto: el dato nuevo llegaba a la vista sin atascar la pantalla

### 2. El fallo dependia de `reset`, no del encendido en frio

Ese fue el dato decisivo:

- si fuera un problema principal de `FMC/LTDC` o de reloj, deberia verse tambien en cold boot
- al fallar solo tras `reset`, la sospecha se desplazo a estado retenido en SDRAM / contenido previo de framebuffer / repintado parcial al reentrar en pantalla

### 3. El video mostraba alternancia de contenido, no un flicker uniforme

Visualmente, no parecia un parpadeo homogéneo del panel completo.

Parecia mas bien:

- un buffer con la UI completa
- otro buffer con solo algunas zonas actualizadas

Cuando TouchGFX hacia `swap`, se veia una alternancia entre ambos estados.

## Causa probable

La causa practica del fallo fue que, tras `reset`, la pantalla entraba con contenido retenido o incompleto entre los dos `framebuffers`, y la politica de invalidacion parcial solo repintaba los widgets que cambiaban.

Eso dejaba un escenario asi:

- buffer A: pantalla ya bien pintada
- buffer B: pantalla con restos previos o sin todos los widgets sembrados

Como el valor principal se actualizaba continuamente, ese widget si se redibujaba y se veia bien.
Los widgets estaticos o menos actualizados no siempre quedaban redibujados en ambos buffers, y por eso "parpadeaban" al alternar.

Ademas, varias cajas laterales tenian `alpha=254`, lo que podia amplificar visualmente artefactos de composicion al arrancar con contenido no totalmente coherente.

## Cambios que resolvieron el problema

Se aplicaron solo cambios de codigo, sin tocar el `.ioc`.

### 1. Siembra de ambos buffers al entrar en `InfoView`

Archivo:

- `TouchGFX/gui/src/info_screen/InfoView.cpp`

Se forzo:

- un `invalidate()` completo al entrar en pantalla
- dos repintados completos adicionales durante los primeros `ticks`

Objetivo:

- asegurarse de que ambos `framebuffers` queden pintados con la misma pantalla completa antes de volver a depender de invalidaciones parciales

### 2. Reinicio explicito del estado visual de la vista

Archivos:

- `TouchGFX/gui/include/gui/info_screen/InfoView.hpp`
- `TouchGFX/gui/src/info_screen/InfoView.cpp`

Se reiniciaron al entrar:

- `lastTelemetry`
- `hasLastTelemetry`
- contador `fullRefreshFramesPending`

Objetivo:

- evitar que la vista arrastre cache interna de la sesion anterior tras `reset`

### 3. Cajas laterales opacas

Archivo:

- `TouchGFX/gui/src/info_screen/InfoView.cpp`

Se ajustaron varias `BoxWithBorder` de `alpha=254` a `alpha=255`.

Objetivo:

- eliminar una pequeña fuente adicional de artefactos visuales en el arranque

## Archivos relacionados con el diagnostico

Para separar este fallo del problema anterior de ancho de banda, se añadieron contadores por UART en:

- `Core/Src/memorymap.c`
- `Core/Src/freertos.c`
- `Core/Src/ltdc.c`
- `TouchGFX/gui/src/model/Model.cpp`
- `TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp`

Esos logs permitieron confirmar que:

- no habia `FIFO underrun`
- el `swap` seguia funcionando
- el refresco y los `ticks` eran coherentes

## Diferencia respecto al problema anterior

Es importante separar ambos fallos:

### Problema anterior

- causa raiz: `LTDC FIFO underrun`
- mitigacion principal: bajar `pixel clock` y mejorar margen de `FMC/SDRAM`

### Problema final de esta iteracion

- causa practica: buffers desincronizados o no sembrados igual tras `reset`
- mitigacion principal: repintado completo inicial de la pantalla y estado visual limpio al entrar

## Estado actual

A priori, el problema ha quedado resuelto:

- en arranque normal funciona
- tras `reset` deja de reproducirse el parpadeo que quedaba

Si reapareciera en el futuro, el siguiente paso natural seria:

- copiar explicitamente el framebuffer activo al alterno durante el arranque de TouchGFX

Pero por ahora no parece necesario.
