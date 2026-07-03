# Informe de Mapeo CAN y Parametros de Display

Fecha: 2026-06-21

## Resumen

El display ya no usa el contrato CAN antiguo.

El contrato activo que se parsea en el firmware es el del dash:

- `0x510..0x521`

La telemetria vive en `VehicleTelemetry`, expuesta con alias `DisplayTelemetry`, y el mapeo actual esta centralizado en:

- `Core/Inc/display_telemetry.h`
- `Core/Inc/display_telemetry_can_config.h`
- `Core/Src/display_telemetry_can_config.c`

## Tramas actuales

| ID CAN | Uso |
|---|---|
| `0x510` | estado general, flags y secuencia |
| `0x511` | pedales y freno |
| `0x512` | tension bus, v celda minima y estado inversor |
| `0x513` | temperaturas inversor |
| `0x514` | rpm inversor |
| `0x515` | velocidad inversor |
| `0x516` | corriente inversor |
| `0x517` | estados ECU y AMS |
| `0x518` | AMS lento |
| `0x519` | GPS A |
| `0x51A` | GPS B |
| `0x51B` | GPS C + tick |
| `0x51C` | `vmin_modulo[0..2]` |
| `0x51D` | `vmin_modulo[3..4]` |
| `0x51E` | `vmax_modulo[0..2]` |
| `0x51F` | `vmax_modulo[3..4]` |
| `0x520` | `temp_max_modulo[0..2]` |
| `0x521` | `temp_max_modulo[3..4]` + `temp_dcdc` |

## Asociacion a la struct

### Campos ECU

- `0x510` -> `ecu_torque_total`, `ecu_flag_ev_2_3`, `ecu_flag_t11_8_9`, `ecu_boton_arranque`, `sequence`
- `0x511` -> `ecu_s1_aceleracion`, `ecu_s2_aceleracion`, `ecu_s_freno`
- `0x517` -> `ecu_fsm_state`

### Campos AMS

- `0x510` -> `ams_ok_precarga`
- `0x512` -> `ams_v_celda_min`
- `0x517` -> `ams_state`
- `0x518` -> `ams_soc`, `ams_corriente_accu`, `ams_corriente_dcdc`, `ams_temp_dcdc`
- `0x51C..0x51D` -> `ams_vmin_modulo[]`
- `0x51E..0x51F` -> `ams_vmax_modulo[]`
- `0x520..0x521` -> `ams_temp_max_modulo[]`

### Campos inversor

- `0x510` -> `inverter_inv_state`
- `0x512` -> `inverter_inv_dc_bus_voltage`, `inverter_inv_error`, `inverter_inv_vdc_ready`
- `0x513` -> `inverter_inv_motor_temp`, `inverter_inv_igbt_temp`, `inverter_inv_air_temp`
- `0x514` -> `inverter_inv_rpm`
- `0x515` -> `inverter_inv_speed_actual`
- `0x516` -> `inverter_inv_current_actual`

### Campos GPS

- `0x519` -> `gps_speed`, `gps_course_deg`, `gps_altitude`
- `0x51A` -> `gps_fix_type`, `gps_sat_count`, `gps_hdop`, `gps_latitude`
- `0x51B` -> `gps_longitude`, `tick_ms`

## Campos de compatibilidad de UI

La UI actual sigue leyendo algunos campos planos de `DisplayTelemetry`.

Esos campos ya no representan un contrato CAN antiguo; ahora son espejos de compatibilidad del contrato nuevo:

- `accel` <- `ecu_s1_aceleracion`
- `brake` <- `ecu_s_freno`
- `soc` <- `ams_soc`
- `dcBus` <- `inverter_inv_dc_bus_voltage`
- `vMin` <- `ams_v_celda_min`
- `invState` <- `inverter_inv_state`
- `tempInv` <- `inverter_inv_igbt_temp`
- `tempAccu` <- `ams_temp_dcdc`
- `tempMotor` <- `inverter_inv_motor_temp`

## Lo eliminado

Ya no forma parte del firmware actual:

- `0x123`
- `0x101`
- `0x102`
- el modo legacy de `4 bytes`
- los espejos provisionales basados en ese contrato antiguo

## Nota sobre la pantalla

Mientras la `InfoScreen` siga leyendo los campos planos, esos espejos de compatibilidad se mantienen.

Cuando la UI pase a leer directamente los bloques `ecu`, `ams`, `inverter` y `gps`, se podran borrar tambien esos campos planos.
