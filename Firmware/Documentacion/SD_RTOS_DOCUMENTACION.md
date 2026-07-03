# SD en STM32H743

Este documento resume el estado actual real de la integracion SD dentro de la arquitectura RTOS.

## 1. Objetivo final

La SD queda integrada como servicio desacoplado que consume snapshots de telemetria.

Estado actual:

- la tarjeta se detecta correctamente
- `SDMMC1` inicializa
- `FatFs` monta el volumen
- `sdTask` consume `sdLogQueue`
- se escribe telemetria en CSV

## 2. Arquitectura actual

### 2.1 Flujo general

1. `main()` inicializa perifericos y middleware.
2. `MX_FATFS_Init()` enlaza FatFs con la SD.
3. `MX_FREERTOS_Init()` crea `sdTask`.
4. `telemetryTask` genera snapshots.
5. `sdTask` consume snapshots desde `sdLogQueue` y los persiste.

### 2.2 Papel de `sdTask`

La logica funcional vive en [freertos.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/freertos.c:341).

La tarea hace:

1. esperar paquetes en `sdLogQueue`
2. montar la SD si aun no esta montada
3. abrir `0:/TELLOG.CSV`
4. escribir cabecera CSV si el fichero esta vacio
5. anadir lineas de telemetria
6. hacer `f_sync()` periodicamente

## 3. Hardware usado

### 3.1 Interfaz

La SD usa `SDMMC1`.

Archivo principal:

- [sdmmc.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/sdmmc.c)

Configuracion actual conservadora:

- bus en `1 bit`
- `ClockDiv = 8`

### 3.2 Deteccion de tarjeta

Se usa un pin fisico de deteccion:

- `MICROSD_DET_Pin`
- `PC7`

Definido en:

- [main.h](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Inc/main.h)

Configurado en:

- [gpio.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/gpio.c)

La deteccion real se hace en:

- [bsp_driver_sd.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/FATFS/Target/bsp_driver_sd.c)

## 4. Capa software

### 4.1 FatFs

Archivos clave:

- [fatfs.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/FATFS/App/fatfs.c)
- [ffconf.h](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/FATFS/Target/ffconf.h)
- [sd_diskio.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/FATFS/Target/sd_diskio.c)
- [bsp_driver_sd.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/FATFS/Target/bsp_driver_sd.c)

### 4.2 Fichero actual

Fichero usado por el logger:

- `0:/TELLOG.CSV`

El logger escribe:

- una cabecera CSV al crear o abrir un fichero vacio
- una linea por snapshot procesado

## 5. Logs utiles actuales

Mensajes esperables:

```text
[RTOS] sdTask entered
[SD] service start
[SD] mounted
[SD] telemetry logger ready
```

En errores pueden aparecer:

- `wait ready fail`
- `mount fail`
- `open fail`
- `header write fail`
- `line write fail`
- `sync fail`

## 6. Problemas reales encontrados

### 6.1 `FatFs` devolvia errores pese a ver la tarjeta

Durante la depuracion se vieron fallos de montaje y acceso incoherente.

La solucion que se mantuvo fue:

- usar buffer intermedio `scratch`
- copiar con `memcpy()` hacia el buffer real de `FatFs`

Archivo clave:

- [sd_diskio.c](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/FATFS/Target/sd_diskio.c)

### 6.2 La SD no debe bloquear el arranque principal

Decision mantenida:

- no hacer una ruta funcional pesada en `PreOS`
- dejar el trabajo real dentro de `sdTask`

## 7. Ajustes tecnicos mantenidos

### 7.1 `sd_diskio.c`

Siguen activados:

- `ENABLE_SD_DMA_CACHE_MAINTENANCE`
- `ENABLE_SCRATCH_BUFFER`

### 7.2 Timeout SD

Se mantienen timeouts acotados en:

- [stm32h7xx_hal_conf.h](/c:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Inc/stm32h7xx_hal_conf.h)

### 7.3 Deteccion real de tarjeta

La presencia de SD depende del pin `MICROSD_DET`.

## 8. Relacion con la arquitectura RTOS actual

La SD queda desacoplada de:

- la ISR CAN
- TouchGFX
- botones

Su unico productor actual es `telemetryTask`, que envia `TelemetryFanoutPacket` por `sdLogQueue`.

## 9. Estado final actual

La SD se considera integrada como logger de telemetria:

- montaje bajo demanda
- apertura de CSV
- escritura desacoplada por cola
- sincronizacion periodica

## 10. Recomendaciones

### 10.1 Mantener

- `sdTask` como unico punto de acceso a `FatFs`
- buffer intermedio `scratch + memcpy`
- logger desacoplado por `sdLogQueue`

### 10.2 Siguientes mejoras razonables

1. reducir logs de depuracion cuando el sistema quede estable
2. revisar el formato final del CSV
3. anadir rotacion de fichero si el log crece mucho
4. reevaluar `4-bit` si mas adelante hace falta mas rendimiento
