# Informe fallo CAN RX pantalla - 2026-06-28

## Sintoma

- La pantalla arranca correctamente y la tarea de telemetria entra.
- El log de pantalla muestra:
  - `[CAN] FDCAN1 display telemetry ready` o, en la ultima prueba interrumpida, `FDCAN1/2 display telemetry ready`.
  - `[DISP] 1s canRx=0 canDrop=0 ...`
- No aparecen valores de telemetria en la GUI.
- En la placa emisora `EnvioCanMain` parpadea el LED `ERR_STATUS`.

## Lo que significa

- `canRx=0` significa que no entra ninguna interrupcion RX CAN en pantalla.
- No es un problema de pintado de la GUI ni del parser de telemetria: si llegase cualquier frame aceptado, `canRx` subiria.
- El LED `ERR_STATUS` del emisor indica error de bus CAN, normalmente falta de ACK, bus desconectado, transceiver no activo, puerto fisico incorrecto o cableado CANH/CANL/GND.

## Estado actual del codigo

- `EnvioCanMain` esta configurado para enviar por `FDCAN1`.
- `EnvioCanMain` usa `CAN_TELEMETRY_PERIOD_MS = 100U`.
- La prueba de recuperacion bus-off fue retirada.
- En la pantalla, tras la interrupcion, quedo pendiente revisar/limpiar si sigue escuchando tambien `FDCAN2`.
  - Debe quedar finalmente solo `FDCAN1` si el montaje real es puerto 1.

## Pruebas realizadas

- Se probo subir prioridad de `telemetryTask`.
  - Resultado: la tarea entra, pero `canRx` sigue a cero.
- Se probo abrir filtro de pantalla a cualquier ID.
  - Resultado: `canRx` sigue a cero.
- Se comprobo que emisor y pantalla tienen mismo bitrate nominal:
  - `NominalPrescaler = 2`
  - `NominalTimeSeg1 = 103`
  - `NominalTimeSeg2 = 25`
  - PLL2/FDCAN coincidente en ambas configuraciones.
- Se volvio el emisor a 100 ms porque el fallo aparecio tras probar 200 ms.

## Hipotesis mas probable

El emisor esta intentando transmitir por `FDCAN1`, pero no recibe ACK del bus. La pantalla no ve ningun frame. Por tanto el fallo probable esta fuera del flujo GUI/parser:

- cableado CANH/CANL invertido o abierto,
- falta de GND comun,
- transceiver en standby/sin alimentacion,
- puerto fisico de la placa no corresponde a `FDCAN1` del micro,
- no se ha flasheado realmente el binario esperado en una de las placas.

## Siguiente paso minimo

1. Dejar pantalla estrictamente en `FDCAN1`.
2. Dejar `EnvioCanMain` estrictamente en `FDCAN1` y `100 ms`.
3. Flashear ambos binarios confirmando ruta:
   - Pantalla: `Debug/TFG.elf`
   - Emisor: `EnvioCanMain/Debug/Main.elf`
4. Si sigue `canRx=0` y `ERR_STATUS` parpadea, medir fisicamente:
   - continuidad CANH/CANL entre placas,
   - GND comun,
   - alimentacion/transceiver enable,
   - actividad en TXD del transceiver emisor.

