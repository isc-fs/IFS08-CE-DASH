# Informe de cambios manuales (no reflejados automáticamente en el .ioc)

Fecha: 01/03/2026
Proyecto: `Stm32Display/Firmware`

## 1) Resumen ejecutivo

Se han hecho varios cambios **directamente en C** para diagnóstico/puesta en marcha de SDRAM y microSD.
Estos cambios pueden **perderse al regenerar código desde CubeMX** si no se trasladan al `.ioc` o a bloques USER.

Estado funcional validado en pruebas:
- SD: test básico OK (inicialización + estado + info + lectura de 1 bloque).
- Señalización actual por LED en `main.c`:
  - `LED1` encendido fijo = firmware vivo.
  - `LED2` y `LED3` encendidos junto con `LED1` = test SD OK.

---

## 2) Cambios manuales actuales (código)

## 2.1 `Core/Src/main.c`

Cambios relevantes:
1. Se desactivó temporalmente la llamada a MPU al inicio:
   - `// MPU_Config();`

2. Flujo de arranque cambiado a modo test SD:
   - Se inicializa GPIO muy pronto.
   - Se inicializa SDMMC (`MX_SDMMC1_SD_Init()`).
   - Se ejecuta `SD_QuickTest()`.
   - Se usan LEDs para resultado final.

3. Se añadió función de test SD (`SD_QuickTest`) con:
   - validación de bandera `sdmmc1_init_ok`,
   - espera a estado `HAL_SD_CARD_TRANSFER` con timeout,
   - lectura de `HAL_SD_GetCardInfo`,
   - validación de `BlockSize == 512`,
   - lectura de un bloque (`HAL_SD_ReadBlocks`) y espera de transferencia.

4. `SystemClock_Config` quedó forzado a interno para esta fase:
   - `RCC_OscillatorType` sin HSE,
   - `RCC_OscInitStruct.HSEState = RCC_HSE_OFF`,
   - SYSCLK desde HSI.

5. `Error_Handler` personalizado para LEDs:
   - fuerza `LED1=ON`, `LED2=OFF`, `LED3=OFF`.

## 2.2 `Core/Src/sdmmc.c`

Cambios relevantes:
1. Bandera de estado añadida:
   - `uint8_t sdmmc1_init_ok = 0U;`

2. `MX_SDMMC1_SD_Init` modificado:
   - `BusWide` cambiado de 4-bit a 1-bit.
   - `ClockDiv` cambiado de `2` a `120` (arranque conservador).
   - En fallo de `HAL_SD_Init`, devuelve `return` (ya no llama `Error_Handler`).

3. `HAL_SD_MspInit` modificado:
   - Si falla `HAL_RCCEx_PeriphCLKConfig`, hace `return` (ya no `Error_Handler`).
   - Pull-up en líneas SD (`PC8..PC12`, `PD2`) cambiado a `GPIO_PULLUP`.

## 2.3 `Core/Inc/sdmmc.h`

Cambios relevantes:
- Export de bandera para consulta desde `main.c`:
  - `extern uint8_t sdmmc1_init_ok;`

## 2.4 `Core/Src/fmc.c`

Cambios relevantes (SDRAM):
1. Perfiles por macro:
   - `SDRAM_PROFILE_SAFE` y `SDRAM_PROFILE_FAST`.

2. Parámetros SDRAM manuales:
   - CAS, refresh, read burst, read pipe, timings.

3. Secuencia JEDEC de inicialización añadida manualmente:
   - `CLK_ENABLE -> PALL -> AUTOREFRESH -> LOAD_MODE -> ProgramRefreshRate`.

4. `SAFE` actual usa refresh bajo (`230`) para reloj efectivo bajo.

---

## 3) Qué NO está en el .ioc y deberías reflejar en CubeMX

> Si no lo copias al `.ioc`, CubeMX puede sobrescribirlo al regenerar.

## 3.1 Reloj principal (RCC)

En código actual trabajas con HSI (HSE OFF). En el `.ioc` deberías revisar:
- fuente SYSCLK en HSI mientras estés en fase de validación,
- desactivar dependencia de HSE si no lo usas todavía.

## 3.2 SDMMC1

En `.ioc` actualmente se observan parámetros de 4-bit y `ClockDiv=2`.
Para replicar el estado estable de test:
- SDMMC1 bus width: **1-bit**,
- SDMMC1 ClockDiv: **120** (temporal de bring-up),
- Pull-up en CMD y DAT (interno o externo según hardware; idealmente externos en tarjeta SD).

## 3.3 Manejo de fallos en init SD

CubeMX genera `Error_Handler()` ante fallos de init. En tu código de prueba ahora no bloquea.
Si quieres conservar este comportamiento tras regenerar:
- mover manejo personalizado a bloques USER,
- o rehacer el wrapper de init en archivo de usuario.

## 3.4 SDRAM/FMC

La configuración SDRAM estable actual está manual en `fmc.c`.
Para persistir en `.ioc`:
- CAS latency,
- SDClockPeriod,
- ReadBurst/ReadPipe,
- timings FMC,
- secuencia de init SDRAM y refresh equivalente.

---

## 4) Recomendación de migración a "producción"

1. Mantener temporalmente test SD en `main.c` hasta cerrar hardware.
2. Pasar parámetros estables al `.ioc` (SDMMC + RCC + FMC).
3. Regenerar y verificar que no se pierde funcionalidad.
4. Mover tests a `#ifdef DIAG_MODE` para activarlos solo cuando haga falta.
5. Restaurar flujo normal de aplicación en `main.c`.

---

## 5) Riesgo si regeneras ahora mismo

Al regenerar sin actualizar `.ioc`, es muy probable que se sobrescriban:
- `BusWide/ClockDiv/pull-up` de SDMMC,
- manejo no bloqueante de fallos en `sdmmc.c`,
- ajustes manuales SDRAM en `fmc.c`,
- parte de lógica de test en `main.c` fuera de USER blocks.

---

## 6) Checklist rápido (lo que debes cambiar en CubeMX)

- [ ] RCC en modo interno (HSI) durante pruebas.
- [ ] SDMMC1 en 1-bit, reloj lento de arranque.
- [ ] Revisar pull-up de CMD/DAT (según esquema de placa).
- [ ] FMC/SDRAM: copiar CAS/timings/refresh del perfil estable.
- [ ] Regenerar y comprobar test de LEDs.

---

Si quieres, en una siguiente iteración te puedo generar también un `INFORME_IoC_VALORES.md` con una tabla exacta "valor actual C" vs "valor en .ioc" campo por campo.