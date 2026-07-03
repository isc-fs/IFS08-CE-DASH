# Informe de descartes del fallo de ghosting

Fecha: `2026-03-10`

## Conclusión raíz

La causa principal del `ghosting` no estaba en TouchGFX ni en la lógica de invalidación como origen primario, sino en `LTDC FIFO underrun` por falta de ancho de banda de lectura desde `SDRAM`.

La evidencia decisiva fue:

- aparición repetida de `HAL_LTDC_ERROR_FU`
- contador `g_ltdc_fifo_underrun_count` creciendo durante las transiciones
- mejora clara del patrón visual al bajar el `pixel clock` del LTDC

Cambio que sí ha atacado la raíz:

- [ltdc.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/ltdc.c)
  - `PLL3R`: de `10` a `12` y después a `14`

## Cambios probados y descartados como causa raíz

### 1. Despertar del panel (`LTDC_STDBY`)

Archivo:
- [main.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/main.c)

Cambio probado:
- forzar `LTDC_STDBY` a `HIGH`

Resultado:
- era necesario para que la pantalla dejara de verse negra
- no era la causa raíz del `ghosting`

Motivo del descarte:
- resolvió el arranque visual del panel, no los artefactos durante transición

### 2. Cambios de polaridad LTDC

Archivo:
- [ltdc.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/ltdc.c)

Cambios probados:
- `DEPolarity`
- `PCPolarity`

Resultado:
- se probaron variantes de `DE` y `PCLK`
- no eliminaron el ghosting

Motivo del descarte:
- el patrón del fallo cambió con el ancho de banda, no con la polaridad
- el ajuste estable quedó finalmente en:
  - `DEPolarity = LTDC_DEPOLARITY_AH`
  - `PCPolarity = LTDC_PCPOLARITY_IPC`

### 3. Ajustes de geometría/timings de panel tomados del `.ioc`

Archivo:
- [ltdc.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/ltdc.c)

Cambios probados:
- `AccumulatedHBP`
- `AccumulatedVBP`
- `AccumulatedActiveW`
- `AccumulatedActiveH`

Resultado:
- llegaron a descentrar la imagen
- no resolvieron el ghosting

Motivo del descarte:
- el problema real persistía incluso con geometría correcta
- no explicaban los `FIFO underrun`

### 4. Cambios en la HAL TouchGFX para doble buffer

Archivo:
- [TouchGFXGeneratedHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp)

Cambios probados:
- usar `VBR` en lugar de `IMR`
- usar `IMR` en lugar de `VBR`
- quitar `swapFrameBuffers()` de la ISR
- restaurar `swapFrameBuffers()` en la ISR
- introducir `currentFrameBuffer/pendingFrameBuffer`
- hacer que `getTFTFrameBuffer()` no leyera `CFBAR` directamente
- volver luego al comportamiento estándar del generador

Resultado:
- el ghosting persistió
- algunos cambios alteraban el patrón, pero no eliminaban el problema

Motivo del descarte:
- el fallo seguía incluso cuando TouchGFX sí renderizaba correctamente
- la evidencia final apuntó a starvation de LTDC, no a semántica incorrecta de swap como causa principal

### 5. Prueba de `single buffer` real

Archivo:
- [TouchGFXGeneratedHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp)
- [TouchGFXGeneratedHAL.hpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/TouchGFXGeneratedHAL.hpp)

Cambio probado:
- forzar un solo framebuffer
- estrategia `REFRESH_STRATEGY_OPTIM_SINGLE_BUFFER_TFT_CTRL`
- `getTFTCurrentLine()`

Resultado:
- empeoró
- aparecieron más cubos/artefactos

Motivo del descarte:
- demostró que el problema no era simplemente “usar doble buffer”
- aumentó la visibilidad del problema, no lo resolvió

### 6. Invalidación más agresiva de la UI

Archivo:
- [Screen1View.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/src/screen1_screen/Screen1View.cpp)

Cambios probados:
- invalidar solo el cuadrado
- invalidar `__background` + `box1`
- invalidar toda la pantalla (`invalidate()`) durante varios ticks

Resultado:
- invalidar fondo completo eliminó un cuadrado residual lateral
- pero el ghosting principal siguió
- invalidación total lo hizo incluso más visible/temprano

Motivo del descarte:
- había un problema secundario de invalidez parcial
- pero no era la causa raíz del artefacto principal

### 7. Desactivar/alterar DMA2D

Archivos:
- [TouchGFXConfiguration.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/TouchGFXConfiguration.cpp)
- [STM32DMA.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/STM32DMA.cpp)
- [TouchGFXHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/TouchGFXHAL.cpp)

Cambios probados:
- desactivar aceleración DMA2D
- ajustar rutas `RGB565`
- añadir barrera `flushDMA()` antes de `endFrame()`

Resultado:
- no resolvieron el ghosting de fondo
- la hipótesis DMA2D perdió fuerza cuando aparecieron `FIFO underrun`

Motivo del descarte:
- el problema real seguía existiendo con evidencia directa de starvation del LTDC

### 8. Instrumentación de `QSPI` / assets externos

Archivo:
- [main.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/main.c)

Cambios probados:
- `QSPI_EnableMemoryMappedMode()`
- verificación de assets

Resultado:
- útil para coherencia del arranque
- no relacionado con el ghosting del cuadrado de prueba

Motivo del descarte:
- el cuadrado de prueba no dependía de bitmaps externos

### 9. Reorganización RTOS / tarea TouchGFX

Archivo:
- [freertos.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/freertos.c)

Cambios probados:
- separar `TouchGFXTask`
- validar creación/entrada de tareas
- hooks de stack overflow / malloc failed

Resultado:
- confirmó que la tarea GUI corría bien
- no resolvió el ghosting

Motivo del descarte:
- FreeRTOS no era la causa raíz principal del artefacto

## Qué sí explicaba parte del síntoma, pero no la raíz

### Invalidación parcial

Sí existía un problema secundario:

- al invalidar solo el cuadrado aparecía un cuadrado residual en otra zona

Eso significa:

- había suciedad de contenido previo o refresco parcial insuficiente

Pero:

- no explicaba por sí solo el patrón principal
- no explicaba los `FIFO underrun`

## Resumen final

Elementos modificados y descartados como causa raíz:

- polaridades LTDC
- geometría/timings alternativos del panel
- variantes de `swapFrameBuffers()`/`VBR`/`IMR`
- pruebas de `single buffer`
- invalidación total o ampliada
- cambios de DMA2D / `flushDMA`
- reorganización RTOS
- integración QSPI/assets

Elemento que sí explicó el fallo:

- falta de ancho de banda LTDC/SDRAM, observada como `FIFO underrun`

Acción efectiva:

- bajar `pixel clock` del LTDC desde código en [ltdc.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/ltdc.c)
