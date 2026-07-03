# Informe de estado SDRAM + LTDC

Fecha: 2026-03-09  
Proyecto: `TFG`  
Objetivo: dejar constancia del estado actual de la SDRAM, el framebuffer y los relojes tras la depuración del display.

---

## 1. Conclusión ejecutiva

El sistema **ya está mostrando imagen en pantalla** mediante barrido de colores.  
Eso demuestra que:

1. **LTDC está inicializando correctamente**.
2. **La SDRAM externa está siendo usada como framebuffer**.
3. **La ruta CPU -> SDRAM -> LTDC -> panel está funcionando**.

Por tanto, a efectos prácticos de vídeo, **la SDRAM funciona** en la configuración actual.

---

## 2. Qué se ha comprobado realmente

Actualmente el firmware de prueba:

- inicializa GPIO,
- inicializa DMA2D,
- inicializa FMC/SDRAM,
- inicializa LTDC,
- pone el panel en activo mediante `LTDC_STDBY`,
- y escribe colores completos en memoria de framebuffer.

Como la pantalla cambia de color correctamente, queda validado que:

- la CPU puede escribir en la dirección de framebuffer,
- el LTDC puede leer esa memoria,
- y el panel recibe datos válidos.

---

## 3. Evidencia de que el framebuffer está en SDRAM

El framebuffer activo está ubicado en `0xC0000000`, que corresponde a la SDRAM externa conectada al FMC.

Referencias en código:

- En [Core/Src/main.c](Core/Src/main.c#L65-L68) `LCD_FB_ADDRESS` está definido como `0xC0000000`.
- En [Core/Src/main.c](Core/Src/main.c#L517-L533) la función `LTDC_FillColor()` escribe directamente en esa dirección.
- En [Core/Src/ltdc.c](Core/Src/ltdc.c#L80-L87) la capa LTDC usa `FBStartAdress = 0xC0000000`.

Resumen del flujo:

$$
	ext{CPU escribe en SDRAM }(0xC0000000) \rightarrow \text{LTDC lee framebuffer} \rightarrow \text{pantalla}
$$

---

## 4. Configuración de reloj actual

### 4.1 Reloj del sistema

Configuración actual en [Core/Src/main.c](Core/Src/main.c#L851-L900):

- Fuente principal: **HSI**
- PLL1: **desactivada**
- `SYSCLK = 64 MHz`
- `HCLK = 32 MHz`
- APB1/APB2/APB3/APB4 = `16 MHz`

### 4.2 Reloj FMC / SDRAM

Configuración actual en [Core/Src/fmc.c](Core/Src/fmc.c#L86-L96) y [Core/Src/fmc.c](Core/Src/fmc.c#L188-L196):

- `FMC clock = D1HCLK = 32 MHz`
- `SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2`

Por tanto:

$$
SDRAM\_CLK = \frac{32\text{ MHz}}{2} = 16\text{ MHz}
$$

### 4.3 Reloj LTDC

Configuración actual en [Core/Src/ltdc.c](Core/Src/ltdc.c#L114-L128):

- PLL3 configurada para dar aproximadamente **32 MHz** de pixel clock al LTDC.

---

## 5. Configuración funcional actual de vídeo

La configuración de vídeo que ha quedado operativa es:

- Formato LTDC: **RGB565**
- Resolución: **800 x 480**
- Framebuffer: **SDRAM externa** en `0xC0000000`
- Test activo: barrido de colores a pantalla completa

Parámetros relevantes en LTDC:

- `AccumulatedHBP = 10`
- `AccumulatedVBP = 10`
- `AccumulatedActiveW = 810`
- `AccumulatedActiveH = 490`
- `TotalWidth = 1056`
- `TotalHeigh = 525`

Referencias:

- [Core/Src/ltdc.c](Core/Src/ltdc.c#L48-L87)
- [Core/Src/main.c](Core/Src/main.c#L673-L722)

---

## 6. Interpretación técnica del resultado

La SDRAM está funcionando **al menos en el caso de uso real del display**.

Eso significa que no estamos ante un fallo básico de:

- soldadura total de SDRAM,
- mapeo FMC completamente roto,
- framebuffer mal direccionado,
- o LTDC leyendo de una memoria inexistente.

Sin embargo, este resultado **no equivale** a una certificación completa de la SDRAM en todas sus direcciones y patrones.  
Lo que sí queda demostrado es que **sirve para el framebuffer y el refresco de pantalla**.

---

## 7. Observación importante sobre la frecuencia SDRAM

La SDRAM está funcionando actualmente a **16 MHz**, que es una frecuencia baja respecto a la hoja de datos del componente.  
Aun así, en esta placa concreta y con esta carga de trabajo, **la imagen sale correctamente**.

Conclusión práctica:

- desde el punto de vista funcional actual, **sirve**;
- desde el punto de vista de margen y robustez, **convendría revisar más adelante** si se desea una configuración más ortodoxa.

---

## 8. Estado actual del proyecto

Estado confirmado:

- SDRAM usada como framebuffer: **sí**
- LTDC funcionando: **sí**
- Panel mostrando imagen: **sí**
- Barrido de colores completo: **sí**
- TouchGFX/RTOS restaurados: **no todavía**

---

## 9. Siguiente paso recomendado

El siguiente paso lógico es **salir del modo de test de colores** y restaurar el arranque real de la aplicación de forma gradual:

1. mantener clocks y SDRAM en el estado que ya funciona,
2. reactivar LTDC + framebuffer con la misma base,
3. arrancar TouchGFX,
4. después reintroducir FreeRTOS,
5. validar finalmente QSPI/assets si aplica.

---

## 10. Resumen final

Sí: **el framebuffer actual está en SDRAM externa y está funcionando**.  
Sí: **el display ya se está alimentando desde esa SDRAM**.  
Sí: **la configuración actual demuestra funcionamiento real de vídeo**.

Pendiente: recuperar el arranque normal de TouchGFX/RTOS sobre esta base funcional.
