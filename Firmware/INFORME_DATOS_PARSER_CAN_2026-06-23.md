# Informe de Datos que Llegan al Parser CAN

Fecha: 2026-06-23

## Objetivo

Dejar en un sitio simple que tramas CAN acepta hoy el parser del display y que campos actualiza en `VehicleTelemetry`.

Fuente real usada:

- `Core/Src/display_telemetry_can_config.c`
- `Core/Inc/display_telemetry.h`
- `Core/Src/freertos.c`

## Resumen rapido

El parser de telemetria acepta actualmente tramas CAN estandar en el rango:

- `0x510` a `0x521`

Ademas, el mismo flujo RX acepta una trama de configuracion:

- `0x120`

La telemetria se parsea sobre:

- `VehicleTelemetry`
- alias `DisplayTelemetry`

## Tramas de telemetria parseadas

| ID CAN | DLC | Campos actualizados |
|---|---:|---|
| `0x510` | 8 | `inverter_inv_state`, `ecu_torque_total`, `ecu_flag_ev_2_3`, `ecu_flag_t11_8_9`, `ams_ok_precarga`, `ecu_boton_arranque`, `sequence` |
| `0x511` | 6 | `ecu_s1_aceleracion`, `ecu_s2_aceleracion`, `ecu_s_freno` |
| `0x512` | 6 | `inverter_inv_dc_bus_voltage`, `ams_v_celda_min`, `inverter_inv_error`, `inverter_inv_vdc_ready` |
| `0x513` | 6 | `inverter_inv_motor_temp`, `inverter_inv_igbt_temp`, `inverter_inv_air_temp` |
| `0x514` | 4 | `inverter_inv_rpm` |
| `0x515` | 4 | `inverter_inv_speed_actual` |
| `0x516` | 4 | `inverter_inv_current_actual` |
| `0x517` | 2 | `ecu_fsm_state`, `ams_state` |
| `0x518` | 7 | `ams_soc`, `ams_corriente_accu`, `ams_corriente_dcdc`, `ams_temp_dcdc` |
| `0x519` | 8 | `gps_speed`, `gps_course_deg`, `gps_altitude` |
| `0x51A` | 8 | `gps_fix_type`, `gps_sat_count`, `gps_hdop`, `gps_latitude` |
| `0x51B` | 8 | `gps_longitude`, `tick_ms` |
| `0x51C` | 6 | `ams_vmin_modulo[0]`, `ams_vmin_modulo[1]`, `ams_vmin_modulo[2]` |
| `0x51D` | 4 | `ams_vmin_modulo[3]`, `ams_vmin_modulo[4]` |
| `0x51E` | 6 | `ams_vmax_modulo[0]`, `ams_vmax_modulo[1]`, `ams_vmax_modulo[2]` |
| `0x51F` | 4 | `ams_vmax_modulo[3]`, `ams_vmax_modulo[4]` |
| `0x520` | 6 | `ams_temp_max_modulo[0]`, `ams_temp_max_modulo[1]`, `ams_temp_max_modulo[2]` |
| `0x521` | 6 | `ams_temp_max_modulo[3]`, `ams_temp_max_modulo[4]`, `ams_temp_dcdc` |

## Trama de configuracion recibida

Esta no rellena `VehicleTelemetry`, pero entra por el mismo camino RX:

| ID CAN | Uso |
|---|---|
| `0x120` | configuracion recibida desde otra ECU |

Formato minimo que acepta hoy:

- byte 0: `key`
- byte 1: `value`

## Campos actuales de `VehicleTelemetry`

### Metadatos

- `tick_ms`
- `sequence`

### ECU

- `ecu_fsm_state`
- `ecu_boton_arranque`
- `ecu_s1_aceleracion`
- `ecu_s2_aceleracion`
- `ecu_s_freno`
- `ecu_torque_total`
- `ecu_flag_ev_2_3`
- `ecu_flag_t11_8_9`

### AMS

- `ams_ok_precarga`
- `ams_state`
- `ams_v_celda_min`
- `ams_soc`
- `ams_vmin_modulo[5]`
- `ams_vmax_modulo[5]`
- `ams_corriente_accu`
- `ams_corriente_dcdc`
- `ams_temp_dcdc`
- `ams_temp_max_modulo[5]`

### Inversor

- `inverter_inv_state`
- `inverter_inv_vdc_ready`
- `inverter_inv_error`
- `inverter_inv_dc_bus_voltage`
- `inverter_inv_motor_temp`
- `inverter_inv_igbt_temp`
- `inverter_inv_air_temp`
- `inverter_inv_rpm`
- `inverter_inv_speed_actual`
- `inverter_inv_current_actual`

### GPS

- `gps_speed`
- `gps_course_deg`
- `gps_altitude`
- `gps_fix_type`
- `gps_sat_count`
- `gps_hdop`
- `gps_latitude`
- `gps_longitude`

## Agrupacion por dominio

### ECU

- `0x510`
- `0x511`
- `0x517`

### AMS

- `0x510`
- `0x512`
- `0x517`
- `0x518`
- `0x51C`
- `0x51D`
- `0x51E`
- `0x51F`
- `0x520`
- `0x521`

### Inversor

- `0x510`
- `0x512`
- `0x513`
- `0x514`
- `0x515`
- `0x516`

### GPS

- `0x519`
- `0x51A`
- `0x51B`

## Notas utiles para la GUI

- No todo llega en la misma trama.
- Un snapshot completo depende de varias IDs.
- `tick_ms` llega por `0x51B`, no es un tick local generado por la UI.
- `sequence` llega por `0x510`.
- `ams_temp_dcdc` hoy se actualiza tanto en `0x518` como en `0x521`.

## Estado real del parser

Hoy el parser:

- acepta solo IDs conocidas
- valida DLC por trama
- escribe directamente en `VehicleTelemetry`
- deja la UI leyendo snapshots ya parseados

No hace hoy:

- escalado fisico adicional
- fusion temporal entre señales
- filtrado visual para la UI
