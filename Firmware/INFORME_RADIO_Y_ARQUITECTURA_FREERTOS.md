# Informe de Radio y Arquitectura FreeRTOS

## 1. Estado del documento

Este informe queda como referencia mixta:

- la parte de arquitectura FreeRTOS describe el estado actual del firmware
- la parte de radio describe el driver y el trabajo previo, pero no una ruta RTOS activa en la version actual

## 2. Arquitectura FreeRTOS actual

La arquitectura actual ya no usa `radioTask` ni `configTask`.

Tareas activas en [freertos.c](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/freertos.c:128):

| Tarea | Prioridad | Funcion |
|---|---:|---|
| `telemetryTask` | `osPriorityAboveNormal` | Procesa RX CAN, aplica config RX y actualiza el snapshot |
| `defaultTask` | `osPriorityNormal` | Inicializacion y polling de botones |
| `TouchGFXTask` | `osPriorityNormal` | Render y ciclo TouchGFX |
| `canTxTask` | `osPriorityBelowNormal` | Envio CAN |
| `sdTask` | `osPriorityBelowNormal` | Logging CSV en SD |

Colas activas:

| Cola | Longitud | Uso |
|---|---:|---|
| `canRxQueue` | 32 | Tramas recibidas por CAN |
| `canTxQueue` | 16 | Mensajes a enviar por CAN |
| `sdLogQueue` | 16 | Snapshots para logging en SD |

Ya no existen:

- `configQueue`
- `radioTxQueue`

## 3. Flujo de datos actual

### 3.1 Telemetria

El flujo principal es:

`FDCAN RX IRQ -> canRxQueue -> telemetryTask -> snapshot -> UI / SD`

Mas en detalle:

1. llega una trama CAN
2. la IRQ la mete en `canRxQueue`
3. `telemetryTask` la procesa
4. se actualiza `VehicleTelemetry` / `DisplayTelemetry`
5. TouchGFX lee snapshot y `sdTask` recibe fan-out por `sdLogQueue`

### 3.2 Configuracion

La configuracion ya no se centraliza en una tarea separada.

Ahora hay dos dominios:

- local: estado propio de la pantalla
- remoto: parametros que la UI manda al vehiculo por CAN

Flujos actuales:

- `CAN config RX -> telemetryTask -> aplicar`
- `UI local -> AppConfig_SetLocal...`
- `UI parametros remotos -> AppConfig_SetVehicleAlertThreshold() -> canTxQueue -> canTxTask`

## 4. Contenedores actuales

### 4.1 Telemetria

La telemetria central se define en [display_telemetry.h](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Inc/display_telemetry.h:1) como `VehicleTelemetry`, con alias `DisplayTelemetry`.

La estructura ya incluye:

- campos de compatibilidad usados por la UI actual
- ECU
- AMS
- inverter
- GPS

### 4.2 Configuracion local

La configuracion local vive como `LocalConfigState` en [freertos.c](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/freertos.c:87).

Campos actuales:

- `telemetryEnabled`
- `sdLoggingEnabled`

### 4.3 Parametros remotos

Los parametros editables que se mandan por CAN viven en `VehicleConfigParams` en [app_config.h](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Inc/app_config.h:9).

Estado actual:

- `alertThresholds[5]`

La UI los consulta y modifica desde [Model.cpp](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/TouchGFX/gui/src/model/Model.cpp:121).

## 5. CAN de configuracion actual

IDs definidos:

- RX config CAN: `0x120`
- TX config CAN: `0x121`

Claves activas:

- `APP_CONFIG_KEY_APP_TELEMETRY_ENABLE = 1`
- `APP_CONFIG_KEY_SD_LOG_ENABLE = 2`
- `APP_CONFIG_KEY_VEHICLE_ALERT_0 = 16`
- `APP_CONFIG_KEY_VEHICLE_ALERT_1 = 17`
- `APP_CONFIG_KEY_VEHICLE_ALERT_2 = 18`
- `APP_CONFIG_KEY_VEHICLE_ALERT_3 = 19`
- `APP_CONFIG_KEY_VEHICLE_ALERT_4 = 20`

Uso actual:

- los cambios locales modifican estado interno
- los cambios de parametros vehiculo se encolan y salen por `canTxTask`

## 6. Estado actual de la radio

La radio ya no forma parte del camino RTOS activo descrito arriba.

Eso no significa que el trabajo previo sobre `nRF24` haya desaparecido, sino que:

- el driver sigue siendo una referencia util
- el firmware actual no crea `radioTask`
- no existe `radioTxQueue`
- la telemetria ya no hace fan-out a radio

Por tanto, cualquier reintroduccion de radio deberia tratarse como una integracion nueva sobre la arquitectura simplificada actual.

## 7. Driver `nRF24`

El driver sigue localizado en:

- [nrf24.h](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Inc/nrf24.h:1)
- [nrf24.c](/C:/Users/info/OneDrive/Escritorio/Pantalla/Stm32Display/Firmware/Core/Src/nrf24.c:1)

Funciones ya implementadas en ese driver:

- inicializacion del bus
- lectura y escritura de registros
- `flush` TX/RX
- entrada en modo RX
- envio de payload
- lectura de payload RX
- limpieza de IRQ del chip

Esto sirve como base si mas adelante se vuelve a activar un servicio RF.

## 8. Recomendacion de arquitectura si vuelve la radio

Si se reintroduce radio, la forma mas limpia seria:

- mantener `telemetryTask` como unica dueña del snapshot
- no mezclar botones con radio
- usar una cola propia solo si la radio vuelve a tener vida independiente
- evitar volver a meter una `configTask` generica si solo hay un pequeño numero de parametros

## 9. Resumen corto

Estado real actual:

- FreeRTOS esta simplificado
- CAN RX funciona por IRQ + cola
- `telemetryTask` concentra telemetria y config RX
- `canTxTask` es la salida CAN unica
- `sdTask` consume snapshots para CSV
- la UI muestra telemetria y edita parametros mediante APIs
- radio queda fuera del camino RTOS activo en esta version
