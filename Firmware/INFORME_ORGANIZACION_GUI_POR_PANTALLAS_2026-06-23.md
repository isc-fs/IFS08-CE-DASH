# Informe de Organizacion GUI por Pantallas

## Pantalla 1. Home



- `ecu_fsm_state`
- `ams_state`
- `ams_ok_precarga`
- `ams_soc`
- `inverter_inv_state`
- `inverter_inv_error`
- `inverter_inv_dc_bus_voltage`
- `ecu_aceleracion`
- `ecu_s_freno`



## Pantalla 2. Drive



- `ecu_aceleracion`
- `ecu_s_freno`
- `ecu_torque_total`
- `inverter_inv_rpm`
- `inverter_inv_speed_actual`
- `inverter_inv_current_actual`
- `gps_speed`


## Pantalla 3. Battery

- `ams_soc`
- `ams_v_celda_min`
- `ams_corriente_accu`
- `ams_corriente_dcdc`
- `inverter_inv_dc_bus_voltage`
- `ams_vmin_modulo[0]`
- `ams_vmin_modulo[1]`
- `ams_vmin_modulo[2]`
- `ams_vmin_modulo[3]`
- `ams_vmin_modulo[4]`
- `ams_vmax_modulo[0]`
- `ams_vmax_modulo[1]`
- `ams_vmax_modulo[2]`
- `ams_vmax_modulo[3]`
- `ams_vmax_modulo[4]`


## Pantalla 4. Thermal

- `inverter_inv_motor_temp`
- `inverter_inv_igbt_temp`
- `inverter_inv_air_temp`
- `ams_temp_dcdc`
- `ams_temp_max_modulo[0]`
- `ams_temp_max_modulo[1]`
- `ams_temp_max_modulo[2]`
- `ams_temp_max_modulo[3]`
- `ams_temp_max_modulo[4]`


## pantalla param

-sdLoggingEnabled
-alertThresholds[0] //temperatura motor
-alertThresholds[1] //temperatura inversor
-alertThresholds[2] //temperatura acumulador
-alertThresholds[3] //alerta soc
alertThresholds[4]  //tension minima acumulador

## Maquina de estados botones

Botones disponibles:

- `UP`
- `DOWN`
- `MENU`
- `SELECT`

### Pantalla Info / Telemetria

Estado unico: `INFO_VIEW`

- `UP`: pagina anterior de telemetria.
- `DOWN`: pagina siguiente de telemetria.
- `MENU`: cambia directamente a `PARAM`.
- `SELECT`: sin uso ahora mismo.

Paginas internas circulares:

- `HOME`
- `DRIVE`
- `BATTERY`
- `THERMAL`

Transiciones:

- `HOME` + `DOWN` -> `DRIVE`
- `DRIVE` + `DOWN` -> `BATTERY`
- `BATTERY` + `DOWN` -> `THERMAL`
- `THERMAL` + `DOWN` -> `HOME`
- `HOME` + `UP` -> `THERMAL`
- `THERMAL` + `UP` -> `BATTERY`
- `BATTERY` + `UP` -> `DRIVE`
- `DRIVE` + `UP` -> `HOME`
- Cualquier pagina + `MENU` -> `PARAM`

### Pantalla Param

Estados actuales:

- `MENU_SELECTION`: selecciona seccion de Param.
- `CONTENT_SELECTION`: selecciona parametro dentro de la seccion.
- `EDIT_ALERT`: edita un umbral de aviso.
- `EDIT_CONFIG`: edita una configuracion local.

Secciones internas:

- `AVISOS`
- `CONFIGURACION`

Transiciones en `MENU_SELECTION`:

- `UP`: seccion anterior.
- `DOWN`: seccion siguiente.
- `SELECT`: entra a seleccionar parametros.
- `MENU`: cambia directamente a `INFO`.

Transiciones en `CONTENT_SELECTION`:

- `UP`: parametro anterior.
- `DOWN`: parametro siguiente.
- `SELECT`: entra a editar el parametro seleccionado.
- `MENU`: vuelve a `MENU_SELECTION`.

Transiciones en `EDIT_ALERT`:

- `UP`: sube el valor.
- `DOWN`: baja el valor.
- `SELECT`: guarda el valor y vuelve a `CONTENT_SELECTION`.
- `MENU`: cancela el cambio y vuelve a `CONTENT_SELECTION`.

Transiciones en `EDIT_CONFIG`:

- `UP`: alterna `ON/OFF`.
- `DOWN`: alterna `ON/OFF`.
- `SELECT`: guarda el valor y vuelve a `CONTENT_SELECTION`.
- `MENU`: cancela el cambio y vuelve a `CONTENT_SELECTION`.

### Comportamiento circular de pantallas

Ahora mismo:

- `INFO` + `MENU` -> `PARAM`
- `PARAM` + `MENU` -> `INFO` solo si Param esta en `MENU_SELECTION`

Comportamiento objetivo si `MENU` debe ser siempre cambio de pantalla:

- `INFO` + `MENU` -> `PARAM`
- `PARAM` + `MENU` -> `INFO`
- En ese caso `MENU` deja de usarse como cancelar/volver dentro de Param.
