# Hoja de Ruta de Arquitectura Display

Fecha: 2026-06-21

## 1. Objetivo

Dejar la arquitectura del display simple, estable y facil de extender, separando bien:

- telemetria recibida por CAN
- configuracion local de la pantalla
- parametros editables que la UI manda por CAN
- logging en SD

## 2. Estado decidido hasta ahora

La base actual queda asi:

- `telemetryTask` es la dueña del estado entrante
- `canTxTask` es la salida CAN unica
- `sdTask` solo consume snapshots de telemetria
- `defaultTask` se queda para botones
- `TouchGFXTask` sigue desacoplada del transporte

Lo eliminado:

- `radioTask`
- `radioTxQueue`
- `configTask`
- `configQueue`

## 3. Modelo de datos objetivo

### 3.1 Telemetria

Contenedor:

- `VehicleTelemetry` con alias `DisplayTelemetry`

Uso:

- almacenar todo lo que llega del vehiculo por CAN
- servir snapshot a la UI
- servir snapshot a SD

### 3.2 Configuracion local

Contenedor:

- `LocalConfigState`

Uso:

- flags internos del display
- opciones que no tienen por que salir del equipo

Ejemplos actuales:

- `telemetryEnabled`
- `sdLoggingEnabled`

### 3.3 Parametros remotos

Contenedor:

- `VehicleConfigParams`

Uso:

- parametros que la UI puede editar
- valores que luego se mandan al vehiculo por CAN

## 4. Hoja de ruta

## Fase 1. Cerrar arquitectura base

Estado:

- cerrada

Objetivo:

- no tocar mas la separacion de tareas salvo necesidad real

Tareas:

- mantener `defaultTask` solo para botones
- mantener `telemetryTask` solo para RX, parseo y fan-out
- mantener `canTxTask` como backend unico de envio
- mantener `sdTask` solo para logging

Criterio de cierre:

- ninguna tarea vuelve a mezclar botones, CAN RX, config y logging

Arquitectura base congelada:

- `defaultTask`: botones
- `telemetryTask`: RX CAN, parseo, snapshot, config RX, fan-out
- `canTxTask`: salida CAN
- `sdTask`: persistencia CSV
- `TouchGFXTask`: render UI

Colas base congeladas:

- `canRxQueue`
- `canTxQueue`
- `sdLogQueue`

Fuera de arquitectura base:

- radio
- colas de radio
- tarea de configuracion separada
- cola de configuracion separada

## Fase 2. Completar `VehicleTelemetry`

Estado:

- cerrada

Objetivo:

- tener definida la estructura completa de variables que va a mandar la ECU

Tareas:

- revisar `DASH_VARIABLES.md` / `CAN3_MAP.md` / `CAN_IDS_VARIABLES.md`
- confirmar que cada variable del contrato dash tenga campo en `VehicleTelemetry`
- desacoplar la UI de la struct cruda mediante su propio modelo de pantalla

Criterio de cierre:

- todas las variables esperadas existen en la struct, aunque no todas esten parseadas todavia

Estado real actual:

- `VehicleTelemetry` ya incluye bloques `ECU`, `AMS`, `inverter` y `GPS`
- el contrato confirmado por `CAN3_MAP.md` y `CAN_IDS_VARIABLES.md` ya queda cubierto por la struct actual
- la UI ya no depende de campos planos viejos y trabaja sobre su propio mapeo

## Fase 3. Completar el parseo CAN de telemetria

Objetivo:

- mapear las tramas reales a `VehicleTelemetry`

Tareas:

- completar `display_telemetry_can_config.c`
- dejar cada ID CAN con su DLC y offsets reales
- eliminar espejos provisionales cuando existan señales reales

Criterio de cierre:

- las variables que llegan por CAN actualizan su campo correcto

## Fase 4. Consolidar la configuracion local

Objetivo:

- dejar claro que cosas son solo del display

Tareas:

- mantener en `LocalConfigState` solo flags locales
- no mezclar ahi parametros del vehiculo
- usar `AppConfig_IsLocal...` y `AppConfig_SetLocal...` como unica API publica

Criterio de cierre:

- cualquier opcion local se cambia sin pasar por CAN TX

## Fase 5. Consolidar parametros remotos

Objetivo:

- dejar claro que cosas edita la UI para mandarlas fuera

Tareas:

- ampliar `VehicleConfigParams` cuando se conozcan mas parametros reales
- asociar cada parametro a su clave CAN
- seguir encolando cambios en `canTxQueue`

Criterio de cierre:

- la UI no construye mensajes CAN directamente
- todo sale por `canTxTask`

## Fase 6. Terminar la pagina de parametros

Objetivo:

- hacer que la UI refleje bien la separacion entre local y remoto

Tareas:

- mantener una zona de avisos o umbrales remotos
- mantener una zona de configuracion local
- reutilizar filas ya existentes antes de crear mas widgets

Pendiente claro:

- si se quiere, activar desde pantalla el toggle de `sdLoggingEnabled`
- no hace falta crear layout nuevo porque ya hay hueco reutilizable

Criterio de cierre:

- la UI muestra y edita lo necesario sin mezclar telemetria con configuracion

## Fase 7. Logging en SD

Objetivo:

- dejar la SD como logger simple y fiable

Decision actual:

- guardar solo telemetria en `CSV`
- hacer que el `CSV` siga el contenido completo de `VehicleTelemetry`
- usar un fichero por sesion de logging
- persistir el ultimo `session_id` en un fichero `config`

Estado real actual:

- esto aun no esta implementado en firmware
- ahora mismo el logger sigue escribiendo en `0:/TELLOG.CSV`
- `CONFIG.CSV` queda definido como siguiente paso de implementacion

Formato minimo previsto de `CONFIG.CSV`:

```csv
key,value
next_session_id,1
```

Comportamiento previsto:

- al iniciar logging se lee `next_session_id` desde `CONFIG.CSV`
- se crea el fichero de sesion con ese id, por ejemplo `LOG_0001.CSV`
- despues se actualiza `CONFIG.CSV` con el siguiente id disponible

Tareas:

- mantener `sdLogQueue` solo con `TelemetryFanoutPacket`
- sustituir `0:/TELLOG.CSV` por nombre de sesion cuando se implemente
- leer de `config` el ultimo `session_id`
- arrancar una nueva sesion al iniciar logging
- incrementar y guardar en `config` el siguiente `session_id`
- dejar para mas adelante el etiquetado o gestion avanzada desde `param`
- revisar mas adelante si hace falta guardar eventos de config

Criterio de cierre:

- cada sesion genera su propio CSV de telemetria y nada mas

## Fase 8. Validacion funcional

Objetivo:

- comprobar que la arquitectura responde bien en uso real

Tareas:

- verificar que botones navegan siempre a la primera
- verificar que la UI recibe telemetria sin bloquearse
- verificar que editar parametros genera TX CAN
- verificar que activar o desactivar SD afecta solo al logging

Criterio de cierre:

- cada camino funcional queda probado de punta a punta

## 5. Orden recomendado de ejecucion

1. cerrar `VehicleTelemetry`
2. completar parseo CAN real
3. consolidar `VehicleConfigParams`
4. terminar pantalla de parametros
5. validar envio CAN desde UI
6. validar logging SD

## 6. Cosas que no haria ahora

- reintroducir radio
- volver a crear `configTask`
- mezclar config con telemetria en la cola de SD
- crear mas tareas si las actuales ya cubren el flujo

## 7. Resultado esperado

Si se sigue esta hoja de ruta, el sistema deberia quedar asi:

- CAN entra por un camino claro
- la UI consume snapshot limpio
- la config local no contamina la telemetria
- los parametros remotos salen por un unico backend CAN
- la SD registra solo lo que tiene que registrar
