# Informe de Integridad Tras Regeneracion

## Alcance

Este informe resume el estado actual del proyecto despues de las regeneraciones de `CubeMX/.ioc` y `TouchGFX Designer`, identifica que codigo custom sigue presente, que partes siguen siendo fragiles frente a nuevas regeneraciones y que se puede mover a archivos aparte para no perderlo.

Nota:

- `git status` esta limpio, asi que no hay diff pendiente contra control de versiones.
- La lista de abajo esta hecha por inspeccion del codigo actual, no por diff git.
- No se ha tocado el `.ioc` en este paso.

## Conclusiones rapidas

- La causa raiz del ghosting ya quedo acotada: `LTDC FIFO underrun`.
- La solucion actualmente activa esta en codigo, no en el `.ioc`: `PLL3R = 14` en [Core/Src/ltdc.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/ltdc.c).
- Hay varias personalizaciones que sobreviven bien a regeneracion si se mantienen dentro de `USER CODE` o en archivos `generated once`.
- La parte mas fragil sigue siendo [TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp), porque `TouchGFX Designer` la sobreescribe.

## Archivos con codigo custom relevante

### 1. Regenerados por CubeMX, pero relativamente seguros si se respetan USER CODE

- [Core/Src/main.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/main.c)
  - `#include <string.h>`
  - `Debug_SetStage()`, `Debug_LogUart5()`, `Debug_SpinDelay()`
  - helpers QSPI/SDRAM/LTDC de diagnostico
  - secuencia de arranque con logs UART
  - clear de framebuffers
  - `LTDC_STDBY` forzado a `HIGH`
  - orden `MX_TouchGFX_PreOSInit()` -> `MX_TouchGFX_Init()`
  - comprobacion de assets QSPI memory-mapped

- [Core/Inc/main.h](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Inc/main.h)
  - `Debug_LogUart5()`
  - `extern` de contadores TouchGFX/LTDC FIFO underrun

- [Core/Src/ltdc.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/ltdc.c)
  - `ltdc_init_ok`, `ltdc_init_fail_step`
  - contadores `g_ltdc_fifo_underrun_*`
  - `HAL_LTDC_ErrorCallback()` con conteo y log de `FIFO underrun`
  - ajuste de clock estable actual:
    - `PeriphClkInitStruct.PLL3.PLL3R = 14`

- [Core/Src/freertos.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/freertos.c)
  - logs de creacion de tareas
  - sonda de framebuffer
  - snapshots LTDC `snap1/snap2`
  - hooks `vApplicationStackOverflowHook()` y `vApplicationMallocFailedHook()`

- [Core/Src/stm32h7xx_it.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/stm32h7xx_it.c)
  - fault blink/log por UART
  - log del primer `LTDC IRQ`
  - mantenimiento del `SysTick` sin doble llamada anomala

### 2. Archivos TouchGFX que son "generated once" y por tanto razonablemente seguros

- [TouchGFX/App/app_touchgfx.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/App/app_touchgfx.c)
  - logs `MX_TouchGFX_PreOSInit`, `MX_TouchGFX_Init`, `TouchGFX_Task enter`
  - segun su cabecera, este archivo se genera una sola vez

- [TouchGFX/target/TouchGFXHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/TouchGFXHAL.cpp)
  - `flushDMA();` antes de `TouchGFXGeneratedHAL::endFrame();`
  - este es el mejor sitio para meter logica custom de HAL que no quieras perder

### 3. Archivos TouchGFX de usuario, seguros frente a Designer

- [TouchGFX/gui/src/model/Model.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/src/model/Model.cpp)
  - tick de prueba para alternar color

- [TouchGFX/gui/include/gui/model/ModelListener.hpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/include/gui/model/ModelListener.hpp)
  - `onTick()`

- [TouchGFX/gui/src/screen1_screen/Screen1Presenter.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/src/screen1_screen/Screen1Presenter.cpp)
  - propaga `onTick()` a la vista

- [TouchGFX/gui/include/gui/screen1_screen/Screen1View.hpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/include/gui/screen1_screen/Screen1View.hpp)
  - `updateColor()`
  - `colorIndex`

- [TouchGFX/gui/src/screen1_screen/Screen1View.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/src/screen1_screen/Screen1View.cpp)
  - alternancia de color del cuadrado
  - `__background.invalidate();`
  - `box1.invalidate();`

### 4. Archivo fragil frente a TouchGFX Designer

- [TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp)
  - logs de HAL
  - contadores `g_tgfx_line_active_count`, `g_tgfx_line_porch_count`
  - logs `framebuffer0/framebuffer1`
  - logs `setFB`
  - log `first VSYNC`
  - callback `HAL_LTDC_LineEventCallback()`

Este archivo se sobreescribe al regenerar desde TouchGFX Designer. Cualquier cambio aqui debe considerarse temporal o debe migrarse a otra capa.

## Que se puede mover a archivos aparte

### Se puede mover bien

- Helpers de log y formateo UART de `main.c`
- Helpers de diagnostico LTDC/FIFO
- Helpers de snapshot de framebuffer/LTDC
- Logica de prueba de UI y refresco de color

Propuesta razonable:

- `Core/Inc/boot_debug.h`
- `Core/Src/boot_debug.c`
  - `Debug_LogUart5()`
  - `Debug_SetStage()`
  - logs de boot

- `Core/Inc/display_diag.h`
- `Core/Src/display_diag.c`
  - contadores `g_ltdc_fifo_underrun_*`
  - `HAL_LTDC_ErrorCallback()`
  - formateo de snapshots LTDC

- `Core/Inc/panel_runtime.h`
- `Core/Src/panel_runtime.c`
  - `Panel_ExitStandby()`
  - `Panel_ClearFramebuffers()`

- `TouchGFX/target/TouchGFXHAL.cpp`
  - mantener aqui toda override que sea posible:
    - `endFrame()`
    - `flushFrameBuffer()`
    - `setTFTFrameBuffer()`
    - `getTFTFrameBuffer()`
    - `enableLCDControllerInterrupt()`
  - esto es mucho mejor que tocar el `generated`

### Se puede mover parcialmente

- Logica del callback LTDC de TouchGFX

Problema:

- `HAL_LTDC_LineEventCallback()` hoy esta definido en [TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp).
- Si el generador vuelve a emitir ese simbolo, no puedes definir otro igual en un archivo custom sin conflicto.

Solucion practica:

- minimizar cambios dentro de `TouchGFXGeneratedHAL.cpp`
- si hace falta logica custom, moverla a helpers no generados y dejar en el generado solo una llamada fina
- pero esa llamada fina habra que revisarla tras cada regeneracion

### No merece la pena sacar sin redisenar el flujo

- `PLL3R = 14` dentro de [Core/Src/ltdc.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/ltdc.c)

Se puede reconfigurar clocks fuera del archivo generado, pero ya no seria una solucion limpia. Mientras no quieras volver a tocar el `.ioc`, este ajuste debe considerarse un parche de codigo a verificar tras cada regeneracion CubeMX.

## Integridad tras regenerar desde CubeMX/.ioc

Checklist minima:

1. [Core/Src/ltdc.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/ltdc.c)
   - confirmar `PLL3R = 14`
   - confirmar `DEPolarity = LTDC_DEPOLARITY_AH`
   - confirmar `PCPolarity = LTDC_PCPOLARITY_IPC`
   - confirmar que sigue existiendo `HAL_LTDC_ErrorCallback()` con conteo de `FIFO underrun`

2. [Core/Src/main.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/main.c)
   - confirmar `LTDC_STDBY` puesto a `HIGH`
   - confirmar clear de framebuffers antes de activar panel
   - confirmar orden `MX_TouchGFX_PreOSInit()` antes de `MX_TouchGFX_Init()`
   - confirmar que sigue `QSPI_EnableMemoryMappedMode()` si se usa esa ruta

3. [Core/Src/freertos.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/freertos.c)
   - confirmar creacion de `TouchGFXTask`
   - confirmar hooks de `stack overflow` y `malloc failed`
   - decidir si se mantienen o se eliminan las sondas `snap1/snap2`

4. [Core/Src/stm32h7xx_it.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/stm32h7xx_it.c)
   - confirmar fault blink/log
   - confirmar que `SysTick_Handler()` no llame dos veces a `xPortSysTickHandler()`

5. [Core/Inc/main.h](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Inc/main.h)
   - confirmar `extern` de contadores y `Debug_LogUart5()`

## Integridad tras regenerar desde TouchGFX Designer

Checklist minima:

1. [TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp)
   - verificar si se han perdido:
     - contadores `g_tgfx_line_active_count`, `g_tgfx_line_porch_count`
     - logs de HAL
     - logs `setFB`
     - log `first VSYNC`
   - verificar que el callback `HAL_LTDC_LineEventCallback()` sigue con la secuencia esperada

2. [TouchGFX/target/TouchGFXHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/TouchGFXHAL.cpp)
   - confirmar que sigue `flushDMA();` en `endFrame()`

3. [TouchGFX/App/app_touchgfx.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/App/app_touchgfx.c)
   - confirmar logs `PreOSInit`, `Init`, `TouchGFX_Task enter`

4. Archivos GUI de usuario
   - [TouchGFX/gui/src/model/Model.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/src/model/Model.cpp)
   - [TouchGFX/gui/src/screen1_screen/Screen1View.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/src/screen1_screen/Screen1View.cpp)
   - [TouchGFX/gui/src/screen1_screen/Screen1Presenter.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/src/screen1_screen/Screen1Presenter.cpp)
   - estos no deberian tocarse, pero conviene verificar que siguen compilando con cualquier cambio de nombres generados

## Recomendacion practica

### Mantener donde esta

- `PLL3R = 14` en `ltdc.c`
- `LTDC_STDBY high` en `main.c`
- `flushDMA()` en `TouchGFXHAL.cpp`
- logica de demo UI en `TouchGFX/gui/src/...`

### Conviene mover a helper propio

- toda telemetria UART de boot y LTDC
- contadores y formateadores de diagnostico
- helpers de panel/framebuffer

### Conviene minimizar en generado

- todo lo que hoy vive en `TouchGFXGeneratedHAL.cpp`

Objetivo:

- dejar ahi solo lo imprescindible que el generador necesita
- sacar el resto a `TouchGFXHAL.cpp` o a helpers no generados

## Resumen ejecutivo

Lo realmente delicado no es CubeMX, sino `TouchGFXGeneratedHAL.cpp`.

Si vuelves a regenerar:

- CubeMX puede pisar `main.c`, `ltdc.c`, `freertos.c`, `stm32h7xx_it.c`, pero ahi puedes sobrevivir bien si mantienes todo en `USER CODE`.
- TouchGFX Designer te va a pisar `TouchGFXGeneratedHAL.cpp` seguro.

Por eso, si quieres robustez real frente a futuras regeneraciones:

1. Mueve diagnostico y helpers de panel a archivos propios en `Core/Src`.
2. Mantén las overrides de HAL en [TouchGFX/target/TouchGFXHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/TouchGFXHAL.cpp).
3. Deja [TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp) lo mas fino posible y asume que hay que revalidarlo tras cada regeneracion de Designer.
