# Informe de configuracion de memoria STM32H743

## 1. Objetivo

Este documento recoge, con foco tecnico y a nivel de firmware, como esta configurada la memoria del proyecto basado en `STM32H743IGT6`. La idea es que sirva como base para redactar despues una memoria o capitulo formal, teniendo ya identificados:

- mapa de memoria interno y externo,
- uso real de `FLASH`, `RAM`, `SDRAM` y `QSPI`,
- configuracion de `MPU`, `I-Cache` y `D-Cache`,
- secuencia de arranque relacionada con memoria,
- reparto real de secciones del binario segun el ultimo `link`,
- observaciones importantes y riesgos de mantenimiento.

El contenido de este informe esta basado en la configuracion efectiva del proyecto, no solo en teoria general del micro.

## 2. Resumen ejecutivo

La aplicacion arranca desde `FLASH` interna en `0x08000000`, ejecuta el codigo principal desde esa flash y coloca las secciones de datos y `bss` en la `AXI SRAM` del dominio D1 (`0x24000000`).

El framebuffer de `TouchGFX` no se coloca en RAM interna, sino en `SDRAM` externa mapeada en `0xC0000000`. En concreto, se reservan dos framebuffers consecutivos para doble buffer y ambos caben dentro de los primeros `2 MB` de la SDRAM.

La `MPU` se usa para:

- marcar la `QSPI` como region cacheable y no ejecutable,
- marcar una ventana grande de `SDRAM` como cacheable,
- superponer encima una subregion de `2 MB` no cacheable y shareable en la base de `SDRAM`,
- conseguir que el framebuffer quede fuera de `D-Cache`, evitando problemas de coherencia entre CPU, LTDC y DMA2D.

La `D-Cache` y la `I-Cache` estan habilitadas al inicio del `main`, antes de `HAL_Init()`. Esto mejora rendimiento, pero obliga a tratar con cuidado todas las memorias compartidas con perifericos DMA.

## 3. Fuentes revisadas

Se ha extraido informacion de:

- `STM32H743IGTX_FLASH.ld`
- `Core/Src/main.c`
- `Core/Src/fmc.c`
- `Core/Src/memorymap.c`
- `TFG.ioc`
- `Debug/TFG.map`
- `Core/Inc/FreeRTOSConfig.h`
- `FATFS/Target/sd_diskio.c`

## 4. Mapa de memoria del micro y del proyecto

### 4.1 Regiones declaradas en el linker

Segun `STM32H743IGTX_FLASH.ld`, el proyecto define estas regiones:

| Region | Direccion base | Tamano | Uso previsto |
|---|---:|---:|---|
| `FLASH` | `0x08000000` | `512 KB` | codigo de arranque y programa |
| `FLASH2` | `0x08100000` | `512 KB` | segundo banco, no usado como region principal de enlace |
| `DTCMRAM` | `0x20000000` | `128 KB` | disponible, no usada por las secciones principales |
| `RAM_D1` | `0x24000000` | `512 KB` | `.data`, `.bss`, heap/stack |
| `RAM_D2` | `0x30000000` | `288 KB` | disponible, no usada por defecto por el linker |
| `RAM_D3` | `0x38000000` | `64 KB` | disponible, no usada por defecto por el linker |
| `SDRAM` | `0xC0000000` | `32 MB` | framebuffer TouchGFX |
| `QSPI` | `0x90000000` | `256 MB` | assets externos memory-mapped |
| `ITCMRAM` | `0x00000000` | `64 KB` | disponible, no usada por defecto por el linker |

### 4.2 Lectura arquitectonica

En `STM32H743` no toda la RAM interna se comporta igual:

- `ITCMRAM` esta pensada para ejecucion rapida de instrucciones.
- `DTCMRAM` esta pensada para acceso rapido del nucleo a datos, sin pasar por AXI.
- `RAM_D1` es la AXI SRAM del dominio D1, mas adecuada para volumen general de datos.
- `RAM_D2` y `RAM_D3` pertenecen a otros dominios y pueden ser utiles para aislar buffers o perifericos concretos.

En este proyecto, la decision principal es clara: usar `RAM_D1` como RAM general del firmware y sacar el framebuffer grande a `SDRAM` externa.

## 5. Reparto real de secciones del binario

### 5.1 Ubicacion de las secciones principales

Del `Debug/TFG.map` se observa:

| Seccion | Direccion | Tamano |
|---|---:|---:|
| `.isr_vector` | `0x08000000` | `0x298` |
| `.text` | `0x080002A0` | `0x34B5C` |
| `.rodata` | `0x08034DFC` | `0x35B0` |
| `.data` | `0x24000000` | `0x100` |
| `.bss` | `0x24000300` | `0x143C4` |
| `._user_heap_stack` | `0x240146C4` | `0x604` |
| `.TouchGFX_Framebuffer` | `0xC0000000` | `0x177000` |
| `ExtFlashSection` | `0x90000000` | `0x2A30` |

### 5.2 Interpretacion

- El codigo y constantes viven en `FLASH` interna.
- Los datos inicializados y no inicializados viven en `RAM_D1`.
- El heap y stack "minimos de linker" tambien se reservan dentro de `RAM_D1`.
- El framebuffer de `TouchGFX` vive completamente en `SDRAM`.
- Los assets externos colocados en `ExtFlashSection` viven en la memoria `QSPI` mapeada.

### 5.3 Heap y stack reservados por el linker

El linker define:

- `_Min_Heap_Size = 0x200` (`512 bytes`)
- `_Min_Stack_Size = 0x400` (`1024 bytes`)

Esto no representa toda la memoria dinamica real del sistema, porque ademas `FreeRTOS` usa su propio heap.

### 5.4 Heap real de FreeRTOS

En `Core/Inc/FreeRTOSConfig.h`:

- `configTOTAL_HEAP_SIZE = 65536`

En el `map` aparece:

- `.bss.ucHeap = 0x10000`

Por tanto, `FreeRTOS` reserva `64 KB` dentro de `RAM_D1` para `heap_4.c`.

## 6. Arranque y secuencia de inicializacion de memoria

En `main.c`, el orden relevante es:

1. `MPU_Config();`
2. `SCB_EnableICache();`
3. `SCB_EnableDCache();`
4. `HAL_Init();`
5. `SystemClock_Config();`
6. `PeriphCommonClock_Config();`
7. inicializacion de perifericos
8. `MX_FMC_Init();`
9. `MX_LTDC_Init();`
10. `MX_QUADSPI_Init();`
11. activacion del modo memory-mapped de QSPI

Este orden importa mucho:

- la MPU define los atributos de memoria antes de que el nucleo empiece a trabajar con cachas activas,
- despues se activan `I-Cache` y `D-Cache`,
- luego se inicializan SDRAM, LTDC, QSPI y resto del sistema.

## 7. Configuracion de la FLASH interna

### 7.1 Uso funcional

La `FLASH` interna es la memoria no volatil principal desde la que arranca el firmware:

- tabla de vectores en `0x08000000`,
- codigo en `.text`,
- constantes en `.rodata`,
- imagen de carga de `.data`.

### 7.2 Bancos de flash

El linker define dos bancos:

- `FLASH` en `0x08000000`, `512 KB`
- `FLASH2` en `0x08100000`, `512 KB`

En esta build, las secciones principales van a `FLASH`. `FLASH2` esta declarada, pero no se usa como destino por defecto de secciones principales.

### 7.3 Latencia de flash

En `SystemClock_Config()` se configura:

- `SYSCLK` desde PLL,
- `HAL_RCC_ClockConfig(..., FLASH_LATENCY_5)`

La latencia de flash se ajusta a `5 wait states` para soportar la frecuencia del sistema configurada. Esto es critico porque la CPU ejecuta desde flash y una latencia insuficiente provocaria fallos de funcionamiento.

## 8. Configuracion de relojes con impacto en memoria

### 8.1 Reloj del nucleo

La configuracion principal es:

- fuente base `HSI`,
- `PLLM = 8`
- `PLLN = 60`
- `PLLP = 2`
- `PLLQ = 4`
- `PLLR = 2`

Con `HSI = 64 MHz`, esto da:

- entrada PLL = `64 / 8 = 8 MHz`
- VCO = `8 x 60 = 480 MHz`
- `SYSCLK = 480 / 2 = 240 MHz`

Luego:

- `SYSCLKDivider = 1`
- `AHBCLKDivider = 2`

Por tanto:

- CPU / `SYSCLK` = `240 MHz`
- `HCLK` / AXI / AHB principal = `120 MHz`

### 8.2 Impacto sobre FMC/SDRAM

En `fmc.c`, el reloj del FMC se fija a:

- `FmcClockSelection = RCC_FMCCLKSOURCE_D1HCLK`

Es decir, el FMC cuelga del `D1HCLK`, que aqui vale `120 MHz`.

Ademas:

- `SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2`

Eso implica:

- `SDRAM_CLK = 120 MHz / 2 = 60 MHz`

Este valor es la base para calcular todos los tiempos de la SDRAM externa.

## 9. RAM interna: como se usa realmente

### 9.1 RAM_D1 como RAM general

La seccion `.data` y `.bss` se ubican en `RAM_D1`, con direccion base `0x24000000`.

Eso significa que:

- variables globales inicializadas,
- variables globales sin inicializar,
- heap de FreeRTOS,
- heap/stack minimo del linker

viven en la AXI SRAM.

Esta es una eleccion razonable porque:

- ofrece bastante capacidad,
- esta bien integrada para uso general,
- convive mejor con el resto del firmware que intentar meter todo en `DTCM`.

### 9.2 DTCMRAM e ITCMRAM

Aunque el micro dispone de:

- `DTCMRAM` en `0x20000000`
- `ITCMRAM` en `0x00000000`

este proyecto no coloca por defecto secciones ahi.

Eso significa que ahora mismo:

- no hay funciones criticas movidas a `ITCM`,
- no hay buffers de alto rendimiento colocados explicitamente en `DTCM`.

Si en el futuro hiciera falta optimizacion extra, estas regiones son candidatas naturales para:

- ISR muy criticas,
- codigo de muy baja latencia,
- buffers CPU-only que no deban ser visibles a DMA.

### 9.3 RAM_D2 y RAM_D3

Tambien estan declaradas:

- `RAM_D2` en `0x30000000`
- `RAM_D3` en `0x38000000`

pero no aparecen como destino de secciones generales en el linker actual.

Esto deja abierta la posibilidad de usarlas mas adelante para:

- buffers DMA dedicados,
- memoria compartida con perifericos concretos,
- aislamiento de trafico respecto a la AXI SRAM principal.

## 10. SDRAM externa

### 10.1 Papel dentro del sistema

La SDRAM externa es la memoria grande del sistema y, en esta aplicacion, su uso principal es alojar el framebuffer del display.

El linker la declara asi:

- base `0xC0000000`
- tamano `32 MB`

### 10.2 Inicializacion del periferico FMC

En `MX_FMC_Init()` se configura:

- banco `FMC_SDRAM_BANK1`
- `ColumnBitsNumber = 8`
- `RowBitsNumber = 12`
- `MemoryDataWidth = 16`
- `InternalBankNumber = 4`
- `CASLatency = 3`
- `SDClockPeriod = 2`
- `ReadBurst = enable`
- `ReadPipeDelay = 1`

Esto describe una SDRAM de:

- bus de datos de `16 bits`,
- `4` bancos internos,
- direccionamiento por `8` bits de columna y `12` bits de fila.

### 10.3 Timings efectivos

El perfil activo de `fmc.c` es el seguro:

- `SDRAM_PROFILE_SAFE`

Los valores efectivos son:

| Parametro | Valor |
|---|---:|
| `LoadToActiveDelay` | `2` |
| `ExitSelfRefreshDelay` | `5` |
| `SelfRefreshTime` | `3` |
| `RowCycleDelay` | `4` |
| `WriteRecoveryTime` | `2` |
| `RPDelay` | `2` |
| `RCDDelay` | `2` |
| `CASLatency` | `3` |
| `ReadPipeDelay` | `1` |
| `RefreshCount` | `918` |

El propio codigo documenta que estos timings estan ajustados para:

- `FMC_CLK = 120 MHz`
- `SDRAM_CLK = 60 MHz`

y que se han elegido de forma conservadora para uso con:

- `LTDC`
- `TouchGFX`
- framebuffer en SDRAM

### 10.4 Secuencia de inicializacion SDRAM

Despues del `HAL_SDRAM_Init()`, el codigo ejecuta manualmente la secuencia SDRAM:

1. `CLK_ENABLE`
2. espera `10 ms`
3. `PALL`
4. `AUTOREFRESH` con `8` ciclos
5. carga del mode register
6. programacion del refresh rate

El `mode register` se carga con:

- burst length `8`
- burst type secuencial
- `CAS = 3`
- operating mode estandar
- write burst single

Esta eleccion tiene una intencion practica clara: mejorar las lecturas largas del LTDC sobre el framebuffer.

### 10.5 Nota importante sobre `.ioc` frente a codigo real

Hay una discrepancia relevante:

- en `TFG.ioc`, `RowCycleDelay1 = 3`
- en `Core/Src/fmc.c`, el valor efectivo compilado es `RowCycleDelay = 4`

Por tanto, la verdad operativa del firmware no es exactamente la del `.ioc`, sino la del codigo C generado y posteriormente ajustado. Esto conviene explicarlo en cualquier redaccion porque afecta a la trazabilidad de la configuracion.

## 11. Framebuffer y memoria grafica

### 11.1 Direcciones y tamano

En `main.c` y `memorymap.c`:

- ancho = `800`
- alto = `480`
- formato = `RGB565`
- bytes por pixel = `2`

Tamano de un framebuffer:

- `800 x 480 x 2 = 768000 bytes`

Direccion del primer framebuffer:

- `0xC0000000`

Direccion del segundo framebuffer:

- `0xC0000000 + 768000 = 0xC00BB800`

### 11.2 Reserva real en el linker

La seccion `.TouchGFX_Framebuffer` ocupa:

- `0x177000` bytes

Eso equivale exactamente a:

- `2 x 768000 = 1536000 bytes`

Por tanto, hay doble buffer real en SDRAM.

### 11.3 Ventaja funcional

Sacar el framebuffer de la RAM interna evita consumir una parte enorme de `RAM_D1` y permite que:

- el firmware general use la SRAM interna,
- la memoria de video quede en una region grande y contigua,
- `LTDC` lea directamente desde SDRAM.

## 12. QSPI externa y assets

### 12.1 Region de memoria

La `QSPI` se declara en el linker en:

- `0x90000000`
- tamano `256 MB`

### 12.2 Uso en el proyecto

El linker define una seccion:

- `ExtFlashSection`

En la build revisada ocupa:

- `0x2A30` bytes

Ademas, en `main.c` se activa el modo:

- `QSPI_EnableMemoryMappedMode()`

Esto implica que una vez configurada la QSPI, la CPU puede leer assets externos como si fueran memoria mapeada en el espacio del procesador.

## 13. MPU: configuracion detallada

### 13.1 Objetivo de la MPU

La `MPU` se usa para asignar atributos a regiones de memoria:

- acceso o no acceso,
- cacheable o no cacheable,
- shareable o no shareable,
- ejecutable o no ejecutable.

En micros Cortex-M7 con caches, esto es esencial para evitar corrupciones sutiles con memorias usadas por CPU y perifericos.

### 13.2 Region 0: barrera por defecto

Se configura:

- base `0x00000000`
- tamano `4 GB`
- `NO_ACCESS`
- `XN`
- no cacheable
- shareable

Ademas usa `SubRegionDisable = 0x87`.

La idea es establecer una region base defensiva sobre casi todo el espacio de direcciones y luego ir abriendo excepciones con regiones superiores.

### 13.3 Region 1: QSPI

Se configura:

- base `0x90000000`
- tamano `256 MB`
- acceso completo
- cacheable
- no shareable
- no ejecutable

Esto tiene sentido para assets graficos en flash externa:

- la CPU puede leerlos con cache,
- no se pretende ejecutar codigo desde ahi,
- se mejora rendimiento de lectura.

### 13.4 Region 2: SDRAM grande cacheable

Se configura:

- base `0xC0000000`
- tamano `32 MB`
- acceso completo
- cacheable
- no ejecutable

Esta region cubre toda la SDRAM y, por si sola, la haria cacheable. Eso es bueno para buffers de trabajo generales en SDRAM, pero no para un framebuffer consumido continuamente por `LTDC`.

### 13.5 Region 3: ventana SDRAM no cacheable

Despues se superpone otra region:

- base `0xC0000000`
- tamano `2 MB`
- acceso completo
- no cacheable
- shareable
- no ejecutable

En ARM MPU, la region con numero mas alto tiene prioridad cuando hay solapamiento. Por tanto:

- los primeros `2 MB` de SDRAM quedan no cacheables,
- el resto de la SDRAM sigue siendo cacheable segun la region 2.

### 13.6 Consecuencia importante

Esto no parece casualidad. La seccion `.TouchGFX_Framebuffer` ocupa `0x177000`, es decir, menos de `2 MB`, y empieza justo en `0xC0000000`.

Por tanto:

- todo el doble framebuffer cae dentro de la subregion no cacheable,
- se evita tener que limpiar o invalidar `D-Cache` constantemente para el video,
- `LTDC` y `DMA2D` ven el contenido real de memoria sin incoherencias por cache.

Esta es probablemente la decision mas importante de toda la configuracion de memoria del proyecto.

## 14. CPU cache: I-Cache y D-Cache

### 14.1 Estado

En `main.c`:

- `SCB_EnableICache();`
- `SCB_EnableDCache();`

Ambas caches estan activadas.

En `TFG.ioc` tambien aparecen:

- `CPU_ICache = Enabled`
- `CPU_DCache = Enabled`

### 14.2 I-Cache

La `I-Cache` mejora la ejecucion desde flash, reduciendo accesos lentos repetidos a la memoria no volatil. Es especialmente util porque el codigo principal vive en `FLASH` interna.

### 14.3 D-Cache

La `D-Cache` mejora accesos a datos, pero introduce un problema clasico:

- la CPU puede tener datos "sucios" o "viejos" en cache,
- mientras un periferico DMA lee o escribe directamente en RAM fisica.

Por eso, con `D-Cache` activada, hay dos estrategias tipicas:

1. usar regiones no cacheables para buffers compartidos criticos,
2. hacer mantenimiento explicito de cache antes y despues de DMA.

En este proyecto se usan ambas, segun el caso.

### 14.4 Caso del framebuffer

Para el framebuffer se escoge la opcion 1:

- region no cacheable por MPU,
- sin necesidad de mantenimiento continuo de cache para video.

### 14.5 Caso de SDMMC DMA

En `FATFS/Target/sd_diskio.c` existe soporte para mantenimiento de cache:

- `SCB_InvalidateDCache_by_Addr`
- `SCB_CleanDCache_by_Addr`

pero la macro:

- `ENABLE_SD_DMA_CACHE_MAINTENANCE`

aparece comentada y no activada por defecto.

Esto significa que, con `D-Cache` activa, el camino de `SDMMC` por DMA merece una revision cuidadosa. El codigo ya esta preparado para tratar coherencia, pero esa proteccion no parece estar habilitada de forma explicita en el estado revisado.

## 15. Relacion entre linker, MPU y perifericos

La configuracion de memoria correcta no sale de un unico archivo, sino de la combinacion de tres capas:

### 15.1 Capa 1: linker

Define donde vive cada cosa:

- codigo en flash,
- datos en RAM_D1,
- framebuffer en SDRAM,
- assets en QSPI.

### 15.2 Capa 2: init de hardware

Hace que esas memorias existan de verdad en runtime:

- PLL y relojes,
- FMC y secuencia SDRAM,
- QSPI y modo memory-mapped.

### 15.3 Capa 3: MPU/cache

Define como las ve la CPU:

- cacheable o no,
- ejecutable o no,
- compartida o no compartida.

Si una de estas tres capas no esta alineada, aparecen fallos tipicos como:

- lecturas erraticas,
- ghosting o tearing,
- FIFO underrun en LTDC,
- corrupcion silenciosa por DMA y cache.

## 16. Observaciones tecnicas importantes

### 16.1 La SDRAM esta pensada como memoria de video, no como RAM general sin mas

Aunque la SDRAM tiene `32 MB`, la configuracion revisada esta claramente orientada a uso grafico:

- burst largo,
- region inicial no cacheable,
- doble framebuffer,
- comentario explicito sobre reducir starvation del LTDC.

### 16.2 El diseño de la MPU esta bien alineado con el framebuffer

El hecho de reservar exactamente una ventana de `2 MB` no cacheable y meter ahi un doble framebuffer de `0x177000` bytes es una decision muy coherente.

### 16.3 El `.ioc` no es la unica fuente de verdad

Hay parametros que han quedado ajustados directamente en C. Si se regenera sin cuidado desde CubeMX, parte de esta logica fina podria perderse o quedar desincronizada.

### 16.4 RAM interna aun tiene margen de optimizacion

`DTCM`, `ITCM`, `RAM_D2` y `RAM_D3` existen, pero no se explotan de forma especifica en esta configuracion. En una fase futura se podria estudiar:

- mover buffers criticos a `DTCM`,
- reservar RAM no cacheable dedicada para DMA,
- llevar codigo ultra-critico a `ITCM`.

## 17. Posibles puntos de redaccion para una memoria formal

Si esto se convierte despues en un texto academico o de proyecto, una estructura razonable seria:

1. arquitectura de memoria del `STM32H743`,
2. mapa de memoria adoptado por el firmware,
3. justificacion del uso de SDRAM para framebuffer,
4. configuracion de la MPU y caches,
5. implicaciones de coherencia de cache con LTDC, DMA2D y SDMMC,
6. resultados practicos y ventajas de la configuracion elegida.

## 18. Conclusiones

La configuracion actual de memoria del proyecto no es una configuracion basica, sino una arquitectura bastante intencionada para una aplicacion grafica con `TouchGFX` sobre `STM32H743`.

Los puntos clave son:

- ejecucion desde `FLASH` interna con latencia ajustada,
- uso de `RAM_D1` como RAM principal del firmware,
- uso de `SDRAM` externa para doble framebuffer,
- uso de `QSPI` memory-mapped para assets,
- activacion de `I-Cache` y `D-Cache`,
- uso de `MPU` para dejar cacheable la memoria externa cuando conviene y no cacheable la ventana del framebuffer.

La decision mas critica, y probablemente la mas valiosa de cara a la estabilidad del display, es la superposicion de regiones `MPU` para que el framebuffer en `SDRAM` quede fuera de `D-Cache`, mientras el resto de la SDRAM puede seguir aprovechando cache.

## 19. Anexo de datos concretos del proyecto

### 19.1 Direcciones clave

- `FLASH` principal: `0x08000000`
- `RAM_D1`: `0x24000000`
- `SDRAM`: `0xC0000000`
- `QSPI`: `0x90000000`
- framebuffer 1: `0xC0000000`
- framebuffer 2: `0xC00BB800`

### 19.2 Tamaños clave

- framebuffer simple: `768000 bytes`
- doble framebuffer: `1536000 bytes`
- ventana MPU no cacheable en SDRAM: `2 MB`
- heap FreeRTOS: `64 KB`

### 19.3 Valores clave de reloj

- `SYSCLK`: `240 MHz`
- `HCLK / D1HCLK`: `120 MHz`
- `FMC clock`: `120 MHz`
- `SDRAM clock`: `60 MHz`

