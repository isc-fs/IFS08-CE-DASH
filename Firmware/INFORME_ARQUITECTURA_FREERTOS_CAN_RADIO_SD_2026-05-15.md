# INFORME ARQUITECTURA FREERTOS CAN SD 2026-06-21

## 1. Objetivo

Este documento describe la arquitectura real actual del firmware tras la simplificacion de tareas RTOS.

La idea actual ya no es tener una tarea de configuracion separada ni una ruta RTOS de radio, sino:

- una tarea dueña de la telemetria
- una tarea dedicada al envio CAN
- una tarea dedicada al log en SD
- una tarea ligera para botones
- TouchGFX desacoplado del transporte

## 2. Resumen de arquitectura

### 2.1 Flujo principal de telemetria

El flujo activo ahora mismo es:

`FDCAN1 RX ISR -> canRxQueue -> telemetryTask -> snapshot global`

Desde ese snapshot:

- TouchGFX lee por `DisplayTelemetry_GetSnapshot()`
- `sdTask` recibe una copia por `sdLogQueue`

Ya no existe fan-out RTOS hacia radio.

### 2.2 Flujo principal de configuracion

La configuracion ya no pasa por `configQueue` ni por `configTask`.

Ahora hay dos caminos diferenciados:

- configuracion local: la UI cambia estado interno de la propia pantalla
- configuracion remota: la UI edita parametros del vehiculo y estos se encolan para salir por `canTxTask`

Flujos actuales:

`CAN config RX -> canRxQueue -> telemetryTask -> aplicar`

`UI local -> AppConfig_SetLocal... -> aplicar local`

`UI parametros vehiculo -> AppConfig_SetVehicleAlertThreshold() -> canTxQueue -> canTxTask`

## 3. Tareas FreeRTOS actuales

Definidas en [freertos.c](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/freertos.c:128).

### 3.1 `defaultTask`

Responsabilidad:

- inicializar botones
- hacer polling de botones

No hace:

- polling CAN
- configuracion
- radio

Prioridad actual:

- `osPriorityNormal`

### 3.2 `telemetryTask`

Responsabilidad:

- esperar tramas en `canRxQueue`
- distinguir telemetria CAN de tramas de configuracion
- actualizar el snapshot global
- aplicar configuracion recibida por CAN
- hacer fan-out a SD

Prioridad actual:

- `osPriorityAboveNormal`

### 3.3 `canTxTask`

Responsabilidad:

- consumir `canTxQueue`
- esperar hueco en el TX FIFO de `FDCAN1`
- enviar mensajes CAN de salida

Uso actual:

- envio de parametros editados desde la UI
- ACK o mensajes de configuracion CAN
- futuras ordenes salientes

Prioridad actual:

- `osPriorityBelowNormal`

### 3.4 `sdTask`

Responsabilidad:

- montar la tarjeta
- abrir `0:/TELLOG.CSV`
- escribir snapshots en CSV
- sincronizar periodicamente

Prioridad actual:

- `osPriorityBelowNormal`

### 3.5 `TouchGFXTask`

Se mantiene desacoplada.

La UI no toca CAN ni SD directamente. Solo lee snapshot y usa APIs de configuracion.

## 4. Colas RTOS actuales

### 4.1 `canRxQueue`

Tipo:

- `CanRxFrame`

Uso:

- mover tramas recibidas por interrupcion desde FDCAN hasta `telemetryTask`

Tamano actual:

- `32`

### 4.2 `canTxQueue`

Tipo:

- `CanTxMessage`

Uso:

- desacoplar la logica de aplicacion del envio CAN real

Tamano actual:

- `16`

### 4.3 `sdLogQueue`

Tipo:

- `TelemetryFanoutPacket`

Uso:

- entregar snapshots listos a `sdTask`

Tamano actual:

- `16`

Ya no existen:

- `configQueue`
- `radioTxQueue`

## 5. Contenedores de estado

### 5.1 Telemetria recibida

El snapshot central ahora se modela como `VehicleTelemetry`, con alias de compatibilidad `DisplayTelemetry`, en [display_telemetry.h](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Inc/display_telemetry.h:1).

Se mantienen:

- campos de compatibilidad usados por la UI actual
- nuevos bloques para ECU
- AMS
- inverter
- GPS

No todos los campos nuevos se rellenan todavia desde CAN, pero la estructura ya esta preparada.

### 5.2 Configuracion local

La configuracion propia de la pantalla vive internamente como `LocalConfigState` en [freertos.c](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/freertos.c:87).

Campos actuales:

- `telemetryEnabled`
- `sdLoggingEnabled`

Esta configuracion no sale por CAN salvo que se decida expresamente en otra capa.

### 5.3 Parametros de configuracion del vehiculo

Los parametros editables que la UI puede mandar al exterior viven en `VehicleConfigParams` en [app_config.h](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Inc/app_config.h:9).

Estado actual:

- `alertThresholds[5]`

Claves CAN asociadas:

- `APP_CONFIG_KEY_VEHICLE_ALERT_0`
- `APP_CONFIG_KEY_VEHICLE_ALERT_1`
- `APP_CONFIG_KEY_VEHICLE_ALERT_2`
- `APP_CONFIG_KEY_VEHICLE_ALERT_3`
- `APP_CONFIG_KEY_VEHICLE_ALERT_4`

## 6. Integracion actual de la UI

La UI puede mostrar dos tipos de informacion:

- telemetria recibida por CAN
- parametros/configuracion

Pero ya no mezcla ambas cosas internamente.

### 6.1 Variables locales

`Model.cpp` usa:

- `AppConfig_IsLocalTelemetryEnabled()`
- `AppConfig_SetLocalTelemetryEnabled()`

para estado local de la aplicacion.

### 6.2 Parametros remotos

`Model.cpp` usa:

- `AppConfig_GetVehicleAlertThreshold()`
- `AppConfig_SetVehicleAlertThreshold()`

Cuando se edita un parametro:

1. se actualiza el contenedor `VehicleConfigParams`
2. se construye un mensaje CAN de configuracion
3. se encola en `canTxQueue`
4. `canTxTask` lo transmite

Con esto la UI no envia CAN directamente.

## 7. CAN de configuracion

IDs actuales:

- `CAN_CONFIG_RX_ID = 0x120`
- `CAN_CONFIG_TX_ID = 0x121`

Formato soportado actualmente:

- clave en `byte 0`
- valor de 8 o 16 bits segun helper usado

Helpers publicos:

- `AppConfig_SendCanConfigU8()`
- `AppConfig_SendCanConfigU16()`

## 8. Lo eliminado respecto a la arquitectura anterior

Ya no forman parte del camino activo:

- `radioTask`
- `radioTxQueue`
- `configTask`
- `configQueue`

Tampoco existe ya el concepto de:

- reenviar telemetria por radio desde `telemetryTask`
- backend RTOS de configuracion separado para la GUI

## 9. Estado actual y limites

Estado actual:

- la arquitectura RTOS esta mas simple
- TouchGFX sigue desacoplado
- botones siguen en tarea propia por polling
- la configuracion local y la remota ya estan separadas
- el envio de parametros remotos sale por `canTxTask`

Limites actuales:

- `VehicleTelemetry` ya es mas grande que el mapeo CAN implementado
- `VehicleConfigParams` todavia solo contiene 5 umbrales genericos
- no hay camino RTOS activo de radio en esta version

## 10. Resumen ejecutivo

La arquitectura actual se queda en cinco piezas claras:

- `defaultTask` para botones
- `telemetryTask` como dueña del estado entrante
- `canTxTask` como unica salida CAN
- `sdTask` como logger desacoplado
- `TouchGFXTask` como consumidora de snapshot y editora de parametros via API

La simplificacion importante es que configuracion y radio ya no tienen tarea propia dentro del firmware actual.
