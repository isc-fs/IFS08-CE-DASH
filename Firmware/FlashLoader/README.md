# FlashLoader

Este directorio documenta el estado actual de la carga de assets graficos y guarda el material relacionado con la flash QSPI externa.

## 1. Loader localizado para la QSPI

Archivo incluido:

- `MT25Q128_STM32H7XX-CUSTOM7.FLM`
- `MT25QL128A_STM32H7XX-CUSTOM7-TENTATIVE.stldr`
- `MT25QL128A_STM32H7XX-CUSTOM7-STFMT.stldr`
- `Originales_ST/MT25QL128A_STM32F469I-DK.stldr`
- `MT25QL128A_STM32H743_CUSTOM7_SRC/`

Notas:

- `CUSTOM7` coincide con el pinout QSPI actual del proyecto:
  - `PB2` `CLK`
  - `PB10` `NCS`
  - `PF8` `IO0`
  - `PF9` `IO1`
  - `PF7` `IO2`
  - `PF6` `IO3`
- Este archivo esta en formato `FLM`.
- Para `STM32CubeProgrammer` normalmente hace falta un loader en formato `STLDR`.
- Tener este archivo en el proyecto sirve para documentarlo y conservar la referencia, pero no hace que CubeProgrammer lo use automaticamente.

Estado del `STLDR` tentativo:

- Se ha generado una variante propia para `MT25QL128A` con el pinout `CUSTOM7`.
- No se ha construido desde codigo fuente del fabricante, sino parcheando el `STLDR` H7 con el pinout exacto y adaptando:
  - el nombre del loader,
  - el tamano expuesto de `32 MB` a `16 MB`,
  - y la comprobacion JEDEC para la familia `MT25QL128A / N25Q128A`.
- El fichero de salida es:
  - `FlashLoader/MT25QL128A_STM32H7XX-CUSTOM7-TENTATIVE.stldr`
- El script reproducible usado para generarlo es:
  - `FlashLoader/make_mt25ql128_custom7_stldr.py`
- Se considera una solucion de prueba razonable para `STM32CubeProgrammer`, pero debe validarse con:
  - `erase`,
  - `program`,
  - `verify`,
  - y lectura posterior de la QSPI.

Variante de compatibilidad adicional:

- Se ha generado tambien una segunda variante:
  - `FlashLoader/MT25QL128A_STM32H7XX-CUSTOM7-STFMT.stldr`
- Esta mantiene la misma logica del loader tentativo, pero ajusta la presentacion de las secciones ELF para parecerse mas al formato habitual de los loaders oficiales de ST:
  - `DevInfo`
  - `PrgCode`
  - `PrgData`
- Su objetivo es comprobar si `STM32CubeProgrammer` estaba ignorando el primer fichero por el formato del contenedor ELF y no por la logica interna del acceso QSPI.

Referencia oficial adicional:

- Tambien se ha guardado una copia del loader original de `STM32CubeProgrammer` para la misma memoria:
  - `FlashLoader/Originales_ST/MT25QL128A_STM32F469I-DK.stldr`
- Este fichero no coincide con la familia del microcontrolador ni con el pinout de la placa actual, por lo que no debe usarse directamente como loader final del `STM32H743`.
- Su utilidad es servir como referencia fiable de ST para la memoria `MT25QL128A`, de cara a comparar comportamiento, comandos esperados y futuras adaptaciones.

Base fuente para port real:

- Tambien se ha creado una base de trabajo desde codigo fuente:
  - `FlashLoader/MT25QL128A_STM32H743_CUSTOM7_SRC/`
- Esta carpeta parte del ejemplo fuente de ST para una flash Micron/N25 y ya incorpora:
  - nombre de loader adaptado,
  - tamano de `16 MB`,
  - pinout `CUSTOM7`,
  - y una region de ejecucion orientada a `STM32H7`.
- La nota de estado y porting esta en:
  - `FlashLoader/MT25QL128A_STM32H743_CUSTOM7_SRC/README_PORTING.md`

## 2. Configuracion temporal actual

Estado actual del proyecto:

- Los assets de TouchGFX se han movido temporalmente a la flash interna.
- Esta configuracion se ha dejado para poder ver las imagenes sin depender todavia de un `external loader` funcional para la QSPI.
- La QSPI externa sigue siendo la arquitectura objetivo final, pero ahora mismo no se usa para cargar los bitmaps en la version de prueba.

Cambios realizados:

- En `STM32H743IGTX_FLASH.ld`, la seccion `ExtFlashSection` se enlaza temporalmente en `FLASH` en lugar de `QSPI`.
- En `Core/Src/main.c`, los mensajes de arranque ya no hablan de `QSPI memory-mapped assets`, sino de `asset section`, porque los recursos pueden estar en otra memoria.
- El asset `LogoISC.png` se ha reducido temporalmente a `36x29`, que es el tamano real usado en pantalla.

Ficheros clave de esta configuracion temporal:

- `STM32H743IGTX_FLASH.ld`
- `Core/Src/main.c`
- `TouchGFX/assets/images/LogoISC.png`
- `TouchGFX/generated/images/src/image_LogoISC.cpp`

Backup del logo original:

- `FlashLoader/LogoISC_fullres_backup.png`

## 3. Motivo del cambio temporal

Se detecto que:

- el firmware arrancaba correctamente,
- la ventana QSPI mapeada en memoria respondia,
- pero la zona `0x90000000` estaba llena de `FF`,
- por tanto las imagenes no estaban grabadas fisicamente en la flash externa.

Como no habia un `CUSTOM7.stldr` listo para `STM32CubeProgrammer`, se ha dejado esta solucion temporal para seguir avanzando con la interfaz y las pantallas.

## 4. Como volver a la configuracion final con QSPI externa

Cuando se disponga de un `external loader` valido para la placa, hay que deshacer esta configuracion temporal:

1. Restaurar el logo original:
   - copiar `FlashLoader/LogoISC_fullres_backup.png`
   - sobre `TouchGFX/assets/images/LogoISC.png`

2. Restaurar el linker:
   - en `STM32H743IGTX_FLASH.ld`
   - cambiar la seccion `ExtFlashSection` para que vuelva a terminar en `>QSPI`
   - si se quiere, quitar tambien el comentario temporal de debug

3. Restaurar los mensajes de diagnostico, si se desea:
   - en `Core/Src/main.c`
   - cambiar `asset section` por los mensajes originales orientados a QSPI

4. Regenerar los assets de TouchGFX

5. Recompilar el firmware

6. Programar con `STM32CubeProgrammer` usando `ST-LINK` y un `external loader` valido para la QSPI

## 5. Prueba del STLDR tentativo

Para probar el loader generado:

1. Copiar `MT25QL128A_STM32H7XX-CUSTOM7-TENTATIVE.stldr` a la carpeta `ExternalLoader` de `STM32CubeProgrammer`.
2. Abrir `STM32CubeProgrammer`.
3. Conectar por `ST-LINK`.
4. Seleccionar el loader externo tentativo.
5. Probar primero un borrado de la QSPI.
6. Programar despues los assets o una imagen de prueba.
7. Ejecutar `verify`.
8. Confirmar finalmente que la zona `0x90000000` contiene datos validos y que TouchGFX vuelve a leer los recursos desde la memoria externa.

## 6. Senales de que la vuelta a QSPI externa ha funcionado

Al arrancar, la traza ya no deberia indicar una seccion vacia. En una configuracion correcta deberian verse bytes distintos de `FF` en la zona de assets y las imagenes deberian volver a mostrarse usando la memoria externa.

## 7. Loader GCC compilable para H743 + CUSTOM7

Ademas de los loaders tentativos parcheados sobre binarios existentes, se ha dejado una base nueva compilable desde codigo fuente usando GCC y HAL de `STM32H7`.

Ubicacion de la base fuente:

- `FlashLoader/MT25QL128A_STM32H743_CUSTOM7_GCC`

Artefacto generado:

- `FlashLoader/MT25QL128A_STM32H743_CUSTOM7_GCC.stldr`

Notas de este loader:

- esta orientado especificamente a `MT25QL128A`,
- usa el pinout `CUSTOM7`,
- inicializa la QSPI con HAL de `STM32H7`,
- y ya compila generando un ELF con `DevInfo`.

Para reconstruirlo:

1. entrar en `FlashLoader/MT25QL128A_STM32H743_CUSTOM7_GCC`
2. ejecutar `make`

Estado:

- compilable: si
- aceptacion por `STM32CubeProgrammer`: pendiente de validacion en la herramienta
