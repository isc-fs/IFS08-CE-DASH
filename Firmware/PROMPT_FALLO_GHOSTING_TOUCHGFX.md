# Prompt de diagnóstico: ghosting/refresco anómalo en STM32H743 + LTDC + TouchGFX

Necesito ayuda para diagnosticar un fallo visual en una placa custom con `STM32H743`, `SDRAM`, `LTDC`, `DMA2D`, `FreeRTOS` y `TouchGFX`.

## Contexto de hardware

- MCU: `STM32H743`
- Display: `Newhaven NHD-5.0-800480TF-ATXL-T`
- Resolución: `800x480`
- Interfaz display: `Parallel RGB (LTDC)`
- Framebuffer: `RGB565`
- SDRAM externa para framebuffer
- QSPI externa para assets
- Proyecto con `STM32CubeIDE + CubeMX + TouchGFX Designer`

## Contexto de firmware

- LTDC en `RGB565`
- TouchGFX con `double buffer`
- FreeRTOS con tarea TouchGFX separada
- `LTDC_STDBY` del panel tiene que ponerse a `HIGH` al arrancar o la pantalla queda negra
- La ruta actual relevante está en:
  - `Core/Src/main.c`
  - `Core/Src/ltdc.c`
  - `TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp`
  - `TouchGFX/gui/src/model/Model.cpp`
  - `TouchGFX/gui/src/screen1_screen/Screen1View.cpp`

## Síntoma actual

La pantalla ya funciona y TouchGFX ya dibuja, pero al cambiar el color de un cuadrado en pantalla aparece `ghosting`.

Comportamiento observado:

- Tras `reset`, el primer cambio de color sale limpio.
- A partir del segundo cambio aparece rastro del color anterior.
- En vídeo se observa una columna vertical discontinua en la misma `X` del cuadrado durante la transición.
- El frame final queda bien, pero la transición deja artefactos.
- Cuando se probó `single buffer` real, empeoró y aparecieron más cubos/artefactos.

## Pruebas ya realizadas

### 1. LTDC / panel base

- Se probó cambio directo de framebuffer completo fuera de TouchGFX.
- El cambio completo de color funciona limpio.
- Eso descarta un fallo básico de:
  - reloj LTDC
  - SDRAM como framebuffer
  - panel totalmente mal configurado

### 2. TouchGFX sí renderiza

Se instrumentó por UART y se comprobó:

- `TouchGFX_Task` entra correctamente.
- `first VSYNC` ocurre.
- `setFB` se ejecuta.
- Hay contenido correcto en framebuffer.

Ejemplo real observado:

- `fb0(0,0)=0414`
- `fb0(400,240)=099D`
- `fb1(0,0)=0000`
- `fb1(400,240)=F800`

Eso demuestra que TouchGFX sí dibuja en memoria correctamente.

### 3. LTDC sí conmuta al framebuffer

Snapshots reales medidos:

```text
[LTDC] snap1 act=30 porch=29 cfbar=C00BB800 srcr=00000000 isr=00000000 ier=00000005 cdsr=00000000 cpsr=00C60144 lipcr=000001E9
[LTDC] snap2 act=146 porch=145 cfbar=C00BB800 srcr=00000000 isr=00000000 ier=00000005 cdsr=00000002 cpsr=03FB00AD lipcr=000001E9
```

Eso indica:

- `LineEvent` activo/porch está funcionando
- `CFBAR` cambia
- `SRCR` queda a `0`, o sea la recarga no se queda pendiente

### 4. Invalidación parcial

Cuando se invalidaba solo el cuadrado, aparecía además un cuadrado residual en otra zona.

Al invalidar también el fondo completo:

- desapareció el cuadrado lateral residual
- pero siguió el ghosting del color anterior durante la transición

Eso sugiere que había un problema de invalidez parcial, pero no era la causa completa del fallo.

### 5. Doble buffer vs single buffer

- Se probó `single buffer` real.
- El resultado fue peor: aparecieron más cubos/artefactos.
- Por tanto, el problema no parece ser simplemente “usar doble buffer”.

## Configuración relevante conocida

### LTDC

- `DEPolarity = LTDC_DEPOLARITY_AH`
- `PCPolarity = LTDC_PCPOLARITY_IPC`
- `HSYNC = Active Low`
- `VSYNC = Active Low`
- `800x480`
- timings actuales:
  - `HSync = 1`
  - `HBP = 10`
  - `HFP = 246`
  - `VSync = 1`
  - `VBP = 10`
  - `VFP = 35`

### TouchGFX HAL

La HAL generada usa la secuencia típica:

- `vSync()`
- `OSWrappers::signalVSync()`
- `swapFrameBuffers()`
- `frontPorchEntered()`

en `TouchGFXGeneratedHAL.cpp`.

### MPU / memoria

- SDRAM general cacheable
- región de framebuffer no cacheable
- QSPI y SDRAM con direcciones correctas en linker

## Qué está descartado

Con bastante confianza, ya está descartado:

- pantalla completamente en standby
- TouchGFX sin ejecutar
- FreeRTOS como causa principal
- framebuffer sin dibujar
- LTDC completamente muerto
- problema puramente de texto o de un widget concreto
- fallo simple de “no hay segundo buffer”

## Hipótesis actuales

Las hipótesis más plausibles son:

1. Problema en la presentación del frame durante transiciones, aunque el contenido final del framebuffer sea correcto.
2. Interacción entre invalidez parcial, `DMA2D` y presentación del frame.
3. Alguna incoherencia residual entre lo que TouchGFX considera framebuffer visible y lo que LTDC está mostrando realmente.
4. Alguna condición de limpieza/flush del framebuffer que deja residuos del color anterior durante el cambio.

## Lo que necesito de ti

Analiza este caso como un problema de `ghosting` en un sistema `LTDC + TouchGFX`, pero **sin volver a sugerir pasos ya descartados** como:

- “revisa si TouchGFX corre”
- “revisa si LTDC está inicializado”
- “prueba doble buffer”
- “prueba single buffer”
- “revisa si el framebuffer está en SDRAM”

Quiero una respuesta centrada en:

1. Qué mecanismo concreto puede explicar:
   - primer frame limpio tras reset
   - ghosting a partir del segundo cambio
   - contenido final correcto
   - artefacto vertical durante transición
2. Qué parte exacta del pipeline `TouchGFX + LTDC + DMA2D + invalidación` encaja mejor con ese patrón.
3. Qué cambios concretos y mínimos harías en:
   - `TouchGFXGeneratedHAL.cpp`
   - estrategia de invalidez
   - `DMA2D`
   - política de limpieza/flush
4. Qué telemetría adicional sería realmente útil, evitando pruebas genéricas ya agotadas.

Si propones cambios, quiero que estén priorizados:

- `más probable`
- `más diagnóstica`
- `menos invasiva`

## Nota final

El objetivo no es “hacer que se vea algo”; eso ya está conseguido. El objetivo es eliminar el `ghosting` en las transiciones de color de un cuadrado TouchGFX que ya se renderiza y se muestra.
