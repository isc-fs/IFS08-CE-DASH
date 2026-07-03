# Mensaje de commit — Release v1.0

## Título
```
fix(can): corrección triple de fallo CAN RX + limpieza para release
```

---

## Cuerpo

```
El emisor (STM32H733ZGTx) no transmitía frames CAN a la pantalla
(STM32H743IGTx). canRx=0 persistente, LED ERR parpadeando.

Tres causas encadenadas originadas por el commit "subida de reloj"
(HSI→HSE en el emisor):

1. PLL2 con parámetros de HSI aplicados sobre fuente HSE (25 MHz)
   - VCI = 25/32 = 0.78 MHz fuera del rango VCIRANGE_1
   - Fix: M=25, N=258, Q=2, VCIRANGE_0 → PLL2_Q = 129 MHz ✓

2. Mismatch de tolerancia de oscilador HSE vs HSI
   - SJW=1 con 129 TQ → tolerancia 0.08% < 1% del HSI de pantalla
   - Fix: Prescaler=6, TBit=43 TQ, SJW=12 → tolerancia 1.07% en
     ambas placas. Bitrate 500 kbps mantenido.

3. Bus-Off sin recovery automático en el emisor
   - El FDCAN H7 no se recupera solo de Bus-Off
   - Fix: detección de BusOff en loop principal + HAL_FDCAN_Stop/Start

Otros cambios de esta sesión:
- Período de transmisión CAN: 100 ms → 200 ms (90 frames/s)
- Filtro IIR de RPM en pantalla: /4 → /2 (respuesta ~400 ms)
- Limpieza de dead code en pantalla:
    · g_sdTaskWakeCount eliminada (variable nunca leída)
    · DisplayDiag_LogLtdcStatus() eliminada (nunca llamada)
    · Referencias a módulo radio eliminadas del filtro de logs

Verificado: LEC=0 REC=0 TEC=0, canRx≈165 frames/s estable.

Archivos modificados:
  EnvioCanMain/Core/Src/fdcan.c     — PLL2 + bit timing
  EnvioCanMain/Core/Src/main.c      — Bus-Off recovery + UART debug
  Core/Src/fdcan.c                  — bit timing FDCAN1 y FDCAN2
  Core/Src/freertos.c               — FDCAN2 init + limpieza
  Core/Src/memorymap.c              — diagnóstico CAN + limpieza
  Core/Inc/memorymap.h              — prototipo eliminado
  TouchGFX/gui/src/info_screen/InfoView.cpp — filtro RPM
```
