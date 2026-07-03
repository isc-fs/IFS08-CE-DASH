# Informe de estado ante regeneracion

Fecha: `2026-03-24`

## Alcance

Este informe resume el estado actual del proyecto pensando en futuras regeneraciones desde:

- `TouchGFX Designer`
- `CubeMX/.ioc`

La inspeccion se ha hecho sobre el codigo actual del workspace.

Estado observado:

- `git status` limpio
- no hay cambios pendientes frente al estado actual del proyecto

Eso significa que el informe se basa en la estructura actual del codigo, no en un diff temporal.

## Resumen ejecutivo

El proyecto esta mejor que en iteraciones anteriores, pero todavia tiene varias piezas importantes que no son igual de robustas frente a regeneracion.

### Estado actual

- La correccion del parpadeo tras `reset` esta bien colocada en archivos de usuario de TouchGFX.
- La telemetria de CAN y el modelo de datos estan en zonas razonablemente seguras frente a CubeMX.
- La instrumentacion del callback de linea del LTDC sigue dentro de un archivo generado de TouchGFX y sigue siendo fragil.
- La salida de la `SplashScreen` esta gobernada por logica generada del Designer.
- Los ajustes criticos de clocks, LTDC y SDRAM siguen viviendo en archivos `Core/Src/*.c` fuera de zonas `USER CODE`, asi que CubeMX puede pisarlos.

### Conclusiones rapidas

1. Frente a `TouchGFX Designer`, lo mas delicado sigue siendo:
   - `TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp`
   - `TouchGFX/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp`

2. Frente a `CubeMX/.ioc`, lo mas delicado sigue siendo:
   - `Core/Src/main.c`
   - `Core/Src/ltdc.c`
   - `Core/Src/fmc.c`
   - `Core/Src/main.c::SystemClock_Config()`

3. La prioridad real ahora no es tanto "arreglar un bug", sino consolidar la configuracion para no depender de parches manuales despues de cada regeneracion.

## Estado actual frente a TouchGFX Designer

## 1. Lo que esta bien colocado

### `InfoView` y arreglo del parpadeo post-reset

Archivos:

- `TouchGFX/gui/include/gui/info_screen/InfoView.hpp`
- `TouchGFX/gui/src/info_screen/InfoView.cpp`

Estado:

- la proteccion de repintado inicial esta en archivos de usuario
- `fullRefreshFramesPending` vive en `InfoView`
- el repintado completo inicial tambien vive en `InfoView`

Consecuencia:

- esta parte deberia sobrevivir bien a regeneraciones de Designer

### Modelo de telemetria

Archivos:

- `TouchGFX/gui/include/gui/model/Model.hpp`
- `TouchGFX/gui/src/model/Model.cpp`
- `TouchGFX/gui/src/info_screen/InfoPresenter.cpp`

Estado:

- el modelo y el presenter estan en archivos de usuario
- el enlace `Model -> Presenter -> View` esta en sitios correctos

Consecuencia:

- no deberia perderse al regenerar desde Designer

### HAL custom "generated once"

Archivo:

- `TouchGFX/target/TouchGFXHAL.cpp`

Estado:

- `endFrame()` sigue llamando a `flushDMA()` antes de delegar

Consecuencia:

- este es el mejor punto para meter personalizacion de HAL que quieras conservar

## 2. Lo que sigue siendo fragil

### Callback LTDC de TouchGFX dentro de archivo generado

Archivo:

- `TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp`

Estado actual:

- incluye `memorymap.h`
- llama a:
  - `DisplayDiag_OnVSync()`
  - `DisplayDiag_OnSwap()`
  - `DisplayDiag_OnFrontPorch()`

Problema:

- ese archivo lleva cabecera `Please, do not edit!`
- el Designer lo puede reescribir

Riesgo:

- perder instrumentacion de `VSYNC/swap/front porch`
- volver a tener que parchear a mano tras regeneracion

### Logica temporal de la splash dentro del generado

Archivo:

- `TouchGFX/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp`

Estado actual:

- la pantalla de inicio cambia a `InfoScreen` con una interaccion basada en ticks generada por Designer

Problema:

- para una splash simple esta bien
- pero si quieres salir cuando haya "sistema listo", primera telemetria valida, timeout inteligente, etc., esa logica no deberia quedarse aqui

Riesgo:

- cualquier comportamiento mas complejo de la splash acabaria atado al generado

## Recomendaciones para TouchGFX Designer

### Cambio 1. Minimizar `TouchGFXGeneratedHAL.cpp`

Objetivo:

- dejarlo lo mas cerca posible del generado original

Accion recomendada:

- mover todo lo que no sea estrictamente imprescindible fuera de `TouchGFXGeneratedHAL.cpp`
- si mantienes la instrumentacion, asumir que es temporal o que habra que revalidarla tras regenerar

Observacion:

- en este punto no es obligatorio moverla ya si el objetivo es solo diagnostico
- pero si la quieres conservar a largo plazo, no deberia vivir ahi

### Cambio 2. Sacar la logica de salida de splash del generado si va a crecer

Objetivo:

- que la splash no dependa solo de un contador del Designer

Accion recomendada:

- si la splash va a seguir siendo solo "espera N ticks y cambia", se puede dejar en Designer
- si va a depender de estado real del sistema, mover la logica a:
  - `TouchGFX/gui/src/screen1_screen/Screen1View.cpp`
  - o a `Model`

## Estado actual frente a CubeMX/.ioc

## 1. Lo que esta razonablemente bien

### Telemetria y diagnostico de aplicacion

Archivos:

- `Core/Inc/display_telemetry.h`
- `Core/Inc/memorymap.h`
- `Core/Src/memorymap.c`
- `Core/Src/freertos.c`

Estado:

- el grueso de la telemetria CAN y del diagnostico esta en zonas `USER CODE` o en archivos propios

Consecuencia:

- CubeMX no deberia romperlo facilmente si no cambia las firmas externas

### Callback de error LTDC

Archivo:

- `Core/Src/ltdc.c`

Estado:

- `HAL_LTDC_ErrorCallback()` esta dentro de `/* USER CODE BEGIN 1 */`

Consecuencia:

- el conteo de `FIFO underrun` esta bien situado y deberia sobrevivir

## 2. Lo que sigue siendo fragil frente a CubeMX

### Clock tree del sistema

Archivo:

- `Core/Src/main.c`

Estado actual:

- `SystemClock_Config()` usa configuracion manual:
  - `PWR_REGULATOR_VOLTAGE_SCALE1`
  - `PLL1` encendida
  - `PLLM = 8`
  - `PLLN = 60`
  - `PLLP = 2`
  - `PLLQ = 4`
  - `PLLR = 2`
  - `AHBCLKDivider = RCC_HCLK_DIV2`

Problema:

- todo eso esta fuera de bloques `USER CODE`

Riesgo:

- CubeMX puede reescribir completamente esta funcion

### Ajustes LTDC que no estan en USER CODE

Archivo:

- `Core/Src/ltdc.c`

Estado actual:

- geometria/polaridades dentro de `MX_LTDC_Init()`
- `PCPolarity = LTDC_PCPOLARITY_IPC`
- `PeriphClkInitStruct.PLL3.*` ajustado con macros custom
- `GPIO_InitStruct.Speed = LTDC_GPIO_SPEED`

Problema:

- las macros estan en `USER CODE 0`, pero su uso real en la configuracion LTDC esta en zona generada

Riesgo:

- CubeMX puede volver a poner otros valores de `PLL3`
- CubeMX puede volver a poner velocidad `LOW` en GPIO LTDC
- CubeMX puede cambiar polaridades o geometria si el `.ioc` no refleja el estado actual

### Ajustes SDRAM/FMC fuera de zonas seguras

Archivo:

- `Core/Src/fmc.c`

Estado actual:

- timings y perfiles definidos en `USER CODE 0`
- pero los valores se aplican en la configuracion principal de `hsdram1.Init` y `SdramTiming`
- la fuente de reloj FMC esta como `RCC_FMCCLKSOURCE_D1HCLK`

Problema:

- el uso efectivo de esos valores vive en secciones generadas

Riesgo:

- CubeMX puede devolver:
  - otros timings
  - otro refresh
  - otro `ReadPipe`
  - otro `ReadBurst`
  - otro clock source FMC

### Secuencia de arranque de panel/TouchGFX fuera de USER CODE

Archivo:

- `Core/Src/main.c`

Estado actual:

- despues de los `MX_*_Init()` se hacen varias acciones importantes:
  - comprobar SDRAM/LTDC
  - `Panel_ClearFramebuffers()`
  - `Panel_ExitStandby()`
  - `QSPI_EnableMemoryMappedMode()`
  - `TouchGFX_AssetsPresent()`
  - `MX_TouchGFX_PreOSInit()`
  - `MX_TouchGFX_Init()`

Problema:

- estas llamadas estan en la parte generada del flujo principal, antes de `USER CODE BEGIN 2`

Riesgo:

- CubeMX puede recolocar o borrar esa secuencia al regenerar `main.c`

## Que deberia cambiarse

## Prioridad alta

### 1. Reflejar en el `.ioc` los clocks y timings validados

Cambios que deberian existir en `.ioc` para no depender de parche manual:

- `SystemClock_Config`
  - `VOS1`
  - `PLL1` y divisores actuales
  - `AHB = HCLK/2`

- LTDC
  - `PLL3` coherente con el valor validado actual
  - `PCPolarity = IPC`
  - geometria actual de panel

- FMC/SDRAM
  - fuente `FMC = D1HCLK`
  - timings SDRAM validados
  - `ReadBurst`, `ReadPipe`, refresh y modo de burst

Motivo:

- esto es lo que mas te protege frente a CubeMX

### 2. Sacar la secuencia de arranque custom a una zona segura

Archivo afectado:

- `Core/Src/main.c`

Accion recomendada:

- mover la secuencia custom de arranque a una funcion propia, por ejemplo:
  - `Boot_DisplayBringUpSequence()`
- invocarla desde una zona `USER CODE` estable

Objetivo:

- no dejar `Panel_ClearFramebuffers()`, `Panel_ExitStandby()` y la secuencia TouchGFX colgando de una zona generada

### 3. Decidir si la instrumentacion LTDC/TouchGFX sigue siendo temporal

Archivo afectado:

- `TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp`

Opciones:

- si ya no es necesaria, retirarla
- si sigue siendo importante, dejar documentado que habra que revisar ese archivo tras cada regeneracion

## Prioridad media

### 4. Mover la telemetria CAN fuera de `freertos.c`

Archivo actual:

- `Core/Src/freertos.c`

Estado:

- hoy es seguro frente a regeneracion, porque esta en `USER CODE`

Pero:

- mezcla RTOS, telemetria, parsing CAN y logs

Accion recomendada:

- crear modulo propio:
  - `Core/Inc/display_telemetry.h`
  - `Core/Src/display_telemetry.c`

Beneficio:

- menor friccion en futuras regeneraciones y mejor mantenimiento

### 5. Sacar la logica avanzada de la splash a archivos de usuario

Solo si quieres evolucionarla.

Si la splash se queda como:

- "espera N ticks y cambia"

entonces puede quedarse en Designer sin problema.

Si quieres:

- timeout real
- espera a primera trama CAN
- espera a panel listo
- espera a assets listos

entonces conviene moverla fuera del generado.

## Prioridad baja

### 6. Mantener `InfoView` como punto unico del arreglo visual

Archivo:

- `TouchGFX/gui/src/info_screen/InfoView.cpp`

Estado actual:

- el arreglo post-reset esta en el sitio correcto

Recomendacion:

- mantener ahi toda logica visual puntual
- evitar tocar `InfoViewBase.cpp` salvo para diseño puro desde Designer

## Checklist practica de regeneracion

## Si regeneras desde TouchGFX Designer

Revisar:

1. `TouchGFX/target/generated/TouchGFXGeneratedHAL.cpp`
   - sigue llamando a `DisplayDiag_OnVSync()`
   - sigue llamando a `DisplayDiag_OnSwap()`
   - sigue llamando a `DisplayDiag_OnFrontPorch()`

2. `TouchGFX/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp`
   - la interaccion de la splash sigue como esperas

3. `TouchGFX/gui/src/info_screen/InfoView.cpp`
   - sigue `fullRefreshFramesPending = 2U`
   - siguen los `invalidate()` iniciales

## Si regeneras desde CubeMX/.ioc

Revisar:

1. `Core/Src/main.c`
   - clock tree sigue igual
   - secuencia de arranque de panel sigue igual
   - `Panel_ClearFramebuffers()` y `Panel_ExitStandby()` siguen en el flujo correcto

2. `Core/Src/ltdc.c`
   - `PCPolarity = LTDC_PCPOLARITY_IPC`
   - `PLL3` sigue con el valor validado
   - GPIO LTDC sigue en `VERY_HIGH`

3. `Core/Src/fmc.c`
   - SDRAM timings siguen iguales
   - refresh y burst siguen iguales
   - clock source FMC sigue correcto

4. `Core/Src/freertos.c`
   - `defaultTask` sigue leyendo CAN y publicando telemetria

## Recomendacion final

El siguiente paso correcto para endurecer el proyecto frente a regeneracion es este:

1. Pasar al `.ioc` los ajustes validados de clocks, LTDC y SDRAM.
2. Mover la secuencia custom de arranque de `main.c` a una funcion llamada desde `USER CODE`.
3. Decidir si la instrumentacion en `TouchGFXGeneratedHAL.cpp` se elimina o se acepta como parche temporal revisable.
4. Dejar la logica visual de `InfoView` exactamente donde esta, porque ahora mismo esta bien ubicada.

## Resumen corto

Hoy el proyecto es:

- razonablemente robusto frente a `TouchGFX Designer` en la parte visual de usuario
- poco robusto frente a `CubeMX/.ioc` en clocks, LTDC, SDRAM y secuencia de arranque
- todavia fragil en la HAL generada de TouchGFX por la instrumentacion metida ahi

Si solo haces una cosa ahora, que sea esta:

- alinear `.ioc` con `main.c`, `ltdc.c` y `fmc.c`

Ese es el cambio que mas reduce trabajo manual despues de cada regeneracion.
