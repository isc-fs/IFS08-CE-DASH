# Informe del fallo CAN RX entre emisor y pantalla

Fecha: `2026-06-29`
Estado: **RESUELTO**

---

## Resumen

La pantalla (STM32H743IGTx) no recibia ningun frame CAN del emisor (STM32H733ZGTx).
`canRx=0` de forma persistente. El LED de error del emisor parpadeaba continuamente.

El fallo tenia tres causas encadenadas, todas originadas por el commit **"subida de reloj"**
que cambio la fuente de reloj del emisor de HSI (64 MHz) a HSE (25 MHz) sin actualizar
los parametros derivados de PLL2 que usa FDCAN.

---

## Sintoma observado

- `canRx=0` en la pantalla, sin excepcion.
- LED `ERR_STATUS` del emisor parpadeando desde el arranque.
- Registro UART de la pantalla: `FD1 LEC=7 REC=0 TEC=0` (bus silencioso) o
  `FD1 LEC=1 REC=127 TEC=0` (errores de stuffing/form en recepcion).
- Registro UART del emisor: `LEC=3 TEC=128` (ACK error, Error Passive) o
  `LEC=7 TEC=248 BO=1` (Bus-Off).

---

## Diagnostico

### Herramienta de diagnostico anadida

Para diagnosticar el fallo se anadio instrumentacion UART en ambas placas:

**Emisor** (`EnvioCanMain/Core/Src/main.c`):
- Funcion `DBG_Log()` con `HAL_UART_Transmit` bloqueante sobre USART10 (PG12, 115200 baud).
- `MX_USART10_UART_Init()` antes de `SystemClock_Config()` para capturar fallos de reloj.
- Segunda llamada a `MX_USART10_UART_Init()` tras los relojes para recalcular BRR con APB2 correcto.
- Logs de etapa de arranque: `[BOOT] clocks ok`, `[BOOT] fdcan1 init ok`, `[BOOT] can start ok`.
- `Error_Handler()` modificado para imprimir `[ERR] halt at stage=N` antes de bloquearse.
- Log periodico en el loop cada 10 iteraciones: `[CAN] seq=N PSR=XXXXXXXX LEC=N TEC=N`.

**Pantalla** (`Core/Src/memorymap.c`):
- Log periodico de PSR/ECR de FDCAN1 y FDCAN2 en `DisplayDiag_LogRuntime()`:
  `[CAN] FD1 LEC=N REC=N TEC=N | FD2 LEC=N REC=N TEC=N`.

### Causa 1: PLL2 con parametros para HSI aplicados con fuente HSE

**Archivo:** `EnvioCanMain/Core/Src/fdcan.c`, funcion `HAL_FDCAN_MspInit`

El emisor usaba HSE=25 MHz como fuente de PLL, pero los parametros de PLL2 para FDCAN
seguian calculados para HSI=64 MHz:

```
PLL2M = 32  →  VCI = 25/32 = 0.78 MHz  (fuera del rango VCIRANGE_1: 2-4 MHz)
PLL2N = 129
PLL2RGE = RCC_PLL2VCIRANGE_1  (requiere 2-4 MHz)
```

`HAL_RCCEx_PeriphCLKConfig()` fallaba silenciosamente y el FDCAN no arrancaba, o arrancaba
con una frecuencia totalmente incorrecta (~50 MHz en lugar de 129 MHz), produciendo
una velocidad efectiva de ~195 kbps en lugar de 500 kbps → errores de stuffing (LEC=1).

**Evidencia:** PSR del emisor mostraba `LEC=1 REC=16` (stuff errors = mismatch de bitrate).

**Fix aplicado:**

```c
// ANTES (roto con HSE=25 MHz):
PeriphClkInitStruct.PLL2.PLL2M   = 32;
PeriphClkInitStruct.PLL2.PLL2N   = 129;
PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_1;

// DESPUES (correcto para HSE=25 MHz):
PeriphClkInitStruct.PLL2.PLL2M   = 25;   // VCI = 25/25 = 1 MHz  (VCIRANGE_0: 1-2 MHz)
PeriphClkInitStruct.PLL2.PLL2N   = 258;  // VCO = 1*258 = 258 MHz
PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_0;
// PLL2_Q = 258/2 = 129 MHz → 500 kbps (igual que antes con HSI)
```

---

### Causa 2: Mismatch de tolerancia de oscilador entre HSE y HSI

**Archivos:** `Core/Src/fdcan.c` (pantalla) y `EnvioCanMain/Core/Src/fdcan.c` (emisor)

Tras el fix de PLL2, el emisor generaba 500 kbps exactos desde HSE (cristal, ±50 ppm).
La pantalla seguia usando HSI (oscilador interno, ±1%). Con SJW=1 y 129 TQ por bit,
la tolerancia maxima de oscilador era solo **0.08%**, inferior al ±1% del HSI.

Resultado: la pantalla veia los frames del emisor con errores de stuffing y form
(LEC=1/2 alternando, REC=127 saturado). La pantalla enviaba Active Error Flags
que destruian los frames del emisor, escalando el TEC hasta Bus-Off.

**Evidencia:** PSR de la pantalla mostraba `LEC=1 REC=127 TEC=0` con ambas placas conectadas.

Antes de "subida de reloj" ambas placas usaban HSI, por lo que derivaban juntas y
la diferencia relativa era despreciable (<0.1%). El problema solo aparecio al pasar
el emisor a HSE.

**Fix aplicado en ambas placas:**

Cambio del bit timing para aumentar la tolerancia de oscilador a **1.07%** manteniendo
500 kbps:

| Parametro          | Antes | Despues |
|--------------------|-------|---------|
| NominalPrescaler   | 2     | **6**   |
| NominalTimeSeg1    | 103   | **30**  |
| NominalTimeSeg2    | 25    | **12**  |
| NominalSyncJumpWidth | 1   | **12**  |
| TQ por bit         | 129   | **43**  |
| Bitrate            | 500 kbps | 500 kbps |
| Tolerancia OSC     | 0.08% | **1.07%** |

Formula: tolerancia = SJW / (2 × 13 × TBit) = 12 / (2×13×43) = **1.07%**

Con 43 TQ por bit: Prescaler×TBit = 6×43 = 258 = 129 MHz / 500 kbps ✓
Punto de muestreo: (1+30)/43 = **72.1%**

---

### Causa 3: Bus-Off sin recovery en el emisor

**Archivo:** `EnvioCanMain/Core/Src/main.c`

Al arrancar, el emisor enviaba frames antes de que la pantalla levantara FDCAN
(la pantalla necesita que FreeRTOS arranque y el `telemetryTask` inicialice FDCAN).
Sin ACK, TEC subia a 255 → Bus-Off. El periferico FDCAN en STM32H7 no se recupera
solo del Bus-Off: necesita que el software llame a `HAL_FDCAN_Stop` + `HAL_FDCAN_Start`.

Sin recovery, el emisor quedaba bloqueado en Bus-Off y la unica forma de que funcionara
era arrancar las dos placas exactamente a la vez (para que la pantalla tuviera FDCAN
listo cuando llegaran los primeros frames).

**Fix aplicado:**

```c
// En el loop principal, cuando se detecta Bus-Off:
if (psr.BusOff != 0U)
{
    DBG_Log("[CAN] bus-off: restarting\r\n");
    (void)HAL_FDCAN_Stop(&CAN_TELEMETRY_HANDLE);
    HAL_Delay(10U);
    (void)HAL_FDCAN_Start(&CAN_TELEMETRY_HANDLE);
}
```

---

## Resultado tras los tres fixes

Registro de la pantalla con ambas placas conectadas:

```
[DISP] 1s canRx=170  [CAN] FD1 LEC=0 REC=0 TEC=0
[DISP] 1s canRx=166  [CAN] FD1 LEC=0 REC=0 TEC=0
[DISP] 1s canRx=159  [CAN] FD1 LEC=0 REC=0 TEC=0
```

- `LEC=0 REC=0 TEC=0`: bus completamente limpio, sin ningun error.
- `canRx=~165/s`: pantalla recibe frames del emisor a la cadencia esperada (18 frames × ~10/s).

---

## Ficheros modificados

| Fichero | Cambio |
|---------|--------|
| `EnvioCanMain/Core/Src/fdcan.c` | Fix PLL2 (M=25, N=258, VCIRANGE_0) + nuevo bit timing |
| `EnvioCanMain/Core/Src/main.c` | UART debug log + Bus-Off recovery en loop |
| `Core/Src/fdcan.c` | Nuevo bit timing FDCAN1 y FDCAN2 |
| `Core/Src/freertos.c` | Restaurado FDCAN2 en TaskInit, callback acepta FDCAN1+FDCAN2 |
| `Core/Src/memorymap.c` | Log diagnostico PSR/ECR periodico |
