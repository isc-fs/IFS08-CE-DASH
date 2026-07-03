# Informe de Auditoria para Regeneracion del IOC

Fecha: 2026-03-24

## Objetivo

Este informe resume las diferencias relevantes entre el estado actual del codigo y la configuracion actual de `TFG.ioc`, con el objetivo de preparar una regeneracion limpia desde CubeMX sin perder los ajustes que hoy estan funcionando.

No se ha modificado ningun archivo de configuracion para generar este informe.

## Resumen ejecutivo

El proyecto no esta listo para regenerar el `.ioc` sin revisar antes varios puntos.

Los desajustes principales estan en:

- arbol de reloj `RCC`
- reloj y polaridad de `LTDC`
- timings de `FMC/SDRAM`
- configuracion GPIO de botones, sobre todo `PH6`

La geometria basica del panel `800x480` si esta razonablemente alineada.

## Estado actual del codigo

### 1. RCC y clocks del sistema

El codigo actual usa `PLL1` desde `HSI` y ya no esta trabajando con el arbol viejo de `64 MHz`.

Estado observado en codigo:

- `PLL source = HSI`
- `PLLM = 8`
- `PLLN = 60`
- `PLLP = 2`
- `PLLQ = 4`
- `PLLR = 2`
- `SYSCLK source = PLLCLK`
- `SYSCLK divider = /1`
- `AHB divider = /2`
- `APB1 = /2`
- `APB2 = /2`
- `APB3 = /2`
- `APB4 = /2`
- `voltage scale = VOS1`

Resultado funcional esperado:

- `SYSCLK = 240 MHz`
- `AHB = 120 MHz`
- `APB1/2/3/4 = 60 MHz`

Referencias:

- `Core/Src/main.c`

### 2. LTDC

El codigo actual usa `PLL3` dedicado para el pixel clock del panel.

Estado observado en codigo:

- `PLL3M = 4`
- `PLL3N = 20`
- `PLL3P = 2`
- `PLL3Q = 2`
- `PLL3R = 11`
- pixel clock aproximado `~29.1 MHz`
- `PCPolarity = LTDC_PCPOLARITY_IPC`
- `HSPolarity = AL`
- `VSPolarity = AL`
- `DEPolarity = AH`

Geometria actual:

- `Active = 800 x 480`
- `AccumulatedHBP = 10`
- `AccumulatedVBP = 10`
- `AccumulatedActiveW = 810`
- `AccumulatedActiveH = 490`
- `TotalWidth = 1056`
- `TotalHeigh = 525`

Ademas, el codigo fuerza las lineas LTDC a:

- `GPIO_SPEED_FREQ_VERY_HIGH`

Referencias:

- `Core/Src/ltdc.c`

### 3. FMC / SDRAM

El codigo actual esta afinado para trabajar con `FMC clock = D1HCLK = 120 MHz` y `SDRAM clock = 60 MHz`.

Estado observado en codigo:

- `FmcClockSelection = D1HCLK`
- `CAS latency = 3`
- `SDClockPeriod = 2`
- `ReadBurst = enable`
- `ReadPipeDelay = 1`
- `LoadToActive = 2`
- `ExitSelfRefresh = 5`
- `SelfRefreshTime = 3`
- `RowCycleDelay = 4`
- `WriteRecovery = 2`
- `RPDelay = 2`
- `RCDDelay = 2`

Secuencia manual relevante:

- `mode register` configurado con `burst length = 8`
- `refresh count = 918`

Referencias:

- `Core/Src/fmc.c`

### 4. GPIO de botones

Estado observado en codigo:

- `PH6 = DOWN_SW = input`
- `PH7 = SELECT_SW = input`
- `PH8 = UP_SW = input`
- `PH9 = MENU_SW = input`
- sin `pull-up` ni `pull-down` en el estado actual

Referencias:

- `Core/Src/gpio.c`
- `Core/Inc/main.h`
- `Core/Src/ui_buttons.c`

## Estado actual del IOC

### 1. RCC en `TFG.ioc`

El `.ioc` todavia refleja el arbol viejo.

Valores observados:

- `SYSCLKFreq_VALUE = 64000000`
- `AHB12Freq_Value = 64000000`
- `APB1Freq_Value = 32000000`
- `APB2Freq_Value = 32000000`
- `APB3Freq_Value = 32000000`
- `APB4Freq_Value = 32000000`
- `FMCFreq_Value = 64000000`
- `LTDCFreq_Value = 32000000`

Conclusión:

- si regeneras ahora, CubeMX te puede devolver `main.c`, `fmc.c` y parte de los relojes a un estado incompatible con lo probado en hardware.

### 2. LTDC en `TFG.ioc`

La geometria basica esta alineada, pero hay diferencias relevantes.

Valores observados:

- `ActiveW = 800`
- `ImageWidth = 800`
- `ImageHeight = 480`
- `HBP = 10`
- `HFP = 246`
- `HSync = 1`
- `VBP = 10`
- `VFP = 35`
- `VSync = 1`
- `DEPolarity = AH`
- `PCPolarity = IIPC`

Conclusión:

- la geometria esta bien
- la polaridad de pixel clock no coincide con el codigo actual
- el reloj `LTDCFreq = 32 MHz` tampoco coincide con el ajuste funcional actual `~29.1 MHz`

### 3. FMC / SDRAM en `TFG.ioc`

Parte de la configuracion coincide, pero los timings criticos no.

Valores observados:

- `CASLatency1 = 3`
- `SDClockPeriod1 = 2`
- `ReadBurst1 = enable`
- `ReadPipeDelay1 = 1`
- `LoadToActiveDelay1 = 2`
- `ExitSelfRefreshDelay1 = 3`
- `SelfRefreshTime1 = 3`
- `RowCycleDelay1 = 3`
- `WriteRecoveryTime1 = 2`
- `RPDelay1 = 2`
- `RCDDelay1 = 2`

Conclusión:

- `ExitSelfRefreshDelay` y `RowCycleDelay` no coinciden con el codigo actual
- el `refresh count = 918` no aparece reflejado aqui como ajuste equivalente visible
- el `burst length = 8` del mode register debe revisarse tras regenerar

### 4. GPIO en `TFG.ioc`

Valores observados:

- `PH6 = DOWN_SW = GPIO_Output`
- `PH7 = GPIO_Input`
- `PH8 = UP_SW = GPIO_Input`
- `PH9 = MENU_SW = GPIO_Input`

Conclusión:

- `PH6` esta mal respecto al estado actual del codigo
- `PH7` no esta etiquetado como `SELECT_SW`

## Diferencias criticas antes de regenerar

### Criticas

- `RCC` del `.ioc` no coincide con el arbol de reloj real del codigo
- `PH6` sigue como salida en el `.ioc`
- `LTDC PCPolarity` no coincide
- `LTDC pixel clock` del `.ioc` no coincide
- `FMC/SDRAM` tiene timings parcialmente desfasados

### Importantes pero revisables despues

- `LTDC GPIO speed = VERY_HIGH` puede perderse tras generar
- `SDRAM refresh count = 918` debe confirmarse
- `SDRAM burst length = 8` debe confirmarse
- `PH7` deberia quedar etiquetado como `SELECT_SW`

## Checklist recomendado en CubeMX antes de generar

### RCC

- poner `PLL1` con los divisores reales del codigo
- dejar `SYSCLK = 240 MHz`
- dejar `AHB = 120 MHz`
- dejar `APB1/2/3/4 = 60 MHz`
- confirmar que `FMC` queda alimentado por `D1HCLK`

### LTDC

- mantener la geometria actual del panel
- revisar `PCPolarity` y dejarla como en el codigo funcional
- revisar `PLL3` y dejar el equivalente al ajuste actual de `~29.1 MHz`

### FMC / SDRAM

- ajustar `ExitSelfRefreshDelay = 5`
- ajustar `RowCycleDelay = 4`
- mantener `CAS = 3`
- mantener `ReadBurst = enable`
- mantener `ReadPipeDelay = 1`
- revisar el equivalente del `refresh count = 918`

### GPIO

- cambiar `PH6` a `GPIO_Input`
- dejar `PH7` como input y, si es posible, etiquetarlo `SELECT_SW`
- confirmar `PH8` y `PH9` como inputs

## Checklist recomendado despues de regenerar

Comparar otra vez estos archivos:

- `Core/Src/main.c`
- `Core/Src/ltdc.c`
- `Core/Src/fmc.c`
- `Core/Src/gpio.c`

Y revisar especialmente:

- `PLL1`
- `PLL3`
- `PCPolarity`
- `GPIO speed` del bus LTDC
- `refresh count` SDRAM
- `burst length = 8`
- `PH6` como input

## Riesgo de regenerar sin alinear antes

Si regeneras el `.ioc` sin reflejar estos cambios en CubeMX, los sintomas mas probables son:

- perdida de fluidez o vuelta a un reloj de pantalla distinto
- reaparicion de parpadeo
- menor margen de `FMC/SDRAM`
- botones dejando de responder correctamente por `PH6`

## Que partes del codigo puede pisar CubeMX

CubeMX suele preservar lo que esta dentro de bloques `USER CODE BEGIN ... / END ...`.

Lo que este fuera de esos bloques, dentro de archivos generados por CubeMX, puede ser reescrito al regenerar.

### main.c

#### Se conserva

- includes y helpers dentro de `USER CODE`
- variables y funciones auxiliares de boot/debug dentro de `USER CODE`
- inicializacion temprana manual que vive en `USER CODE BEGIN 1`

#### Se puede sobrescribir

- todo el contenido de `SystemClock_Config()`
- valores de `PLL1`
- divisores `AHB/APB`
- `FLASH_LATENCY`

Conclusion:

- los clocks actuales de `main.c` no estan protegidos por `USER CODE`
- si no replicas el arbol en CubeMX, se perderan al regenerar

### ltdc.c

#### Se conserva

- macros y auxiliares del bloque `USER CODE BEGIN 0`
- `HAL_LTDC_ErrorCallback()` en `USER CODE BEGIN 1`

#### Se puede sobrescribir

- `hltdc.Init.*`
- `PCPolarity`
- sincronismos y acumulados
- configuracion de `PLL3` en `HAL_LTDC_MspInit()`
- velocidad GPIO de las lineas LTDC

Conclusion:

- aunque las macros de usuario se conserven, las asignaciones reales a `hltdc.Init` y `PeriphClkInitStruct` estan fuera de `USER CODE`
- por tanto el ajuste funcional de LTDC si puede perderse

### fmc.c

#### Se conserva

- macros y defines del bloque `USER CODE BEGIN 0`
- llamada a `SDRAM_Initialization_Sequence(&hsdram1)` en `USER CODE BEGIN FMC_Init 2`

#### Se puede sobrescribir

- `hsdram1.Init.*`
- `SdramTiming.*`
- seleccion del reloj `FMC` en `HAL_FMC_MspInit()`
- el cuerpo de `SDRAM_Initialization_Sequence()`

Conclusion:

- aunque los defines de timings sobreviven, la configuracion aplicada al periférico y la secuencia SDRAM siguen fuera de `USER CODE`
- esto significa que `refresh count`, `burst length = 8` y el reloj de `FMC` pueden perderse tras regenerar

### gpio.c

#### Se conserva

- practicamente nada relevante para tu ajuste actual, porque no metiste la configuracion del boton en un bloque `USER CODE`

#### Se puede sobrescribir

- configuracion de `PH6/PH7/PH8/PH9`
- `DOWN_SW` como input
- cualquier cambio en `GPIO_InitStruct` fuera de `USER CODE`

Conclusion:

- el cambio que hace `PH6` entrada es especialmente fragil
- si no lo replicas en el `.ioc`, CubeMX te lo devolvera a salida

### freertos.c

#### Se conserva

- includes de usuario
- variables de usuario
- prototipos de usuario
- cuerpo de `StartDefaultTask()` dentro de `USER CODE`
- logica de telemetria y botones del bloque `USER CODE BEGIN Application`

Conclusion:

- la telemetria CAN, el parser, y el polling de botones en `freertos.c` estan bastante protegidos
- esta no es la zona mas critica de cara a regenerar el `.ioc`

### main.h

#### Se conserva

- defines en `USER CODE BEGIN Private defines`

Conclusion:

- `SELECT_SW_Pin` esta razonablemente seguro mientras siga dentro de ese bloque

### Archivos nuevos de usuario

Estos no forman parte del codigo generado por CubeMX y no deberian borrarse por regenerar el `.ioc`:

- `Core/Inc/display_telemetry_can_config.h`
- `Core/Src/display_telemetry_can_config.c`
- `Core/Inc/ui_buttons.h`
- `Core/Src/ui_buttons.c`

### Resumen rapido de riesgo real

#### Alto riesgo de ser pisado

- `Core/Src/main.c`
- `Core/Src/ltdc.c`
- `Core/Src/fmc.c`
- `Core/Src/gpio.c`

#### Riesgo bajo o bien protegido

- `Core/Src/freertos.c`
- `Core/Inc/main.h`
- archivos nuevos de configuracion CAN y botones

## Conclusion

El estado actual del proyecto puede regenerarse con seguridad solo despues de alinear en CubeMX:

- `RCC`
- `LTDC`
- `FMC/SDRAM`
- `GPIO de botones`

La mejor estrategia es:

1. adaptar el `.ioc`
2. regenerar
3. verificar `main.c`, `ltdc.c`, `fmc.c` y `gpio.c`
4. probar en placa
