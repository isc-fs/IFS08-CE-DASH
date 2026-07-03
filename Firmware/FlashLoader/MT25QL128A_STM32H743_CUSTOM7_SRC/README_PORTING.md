# MT25QL128A STM32H743 CUSTOM7 source base

Esta carpeta es una base de trabajo para construir un `STLDR` real desde codigo fuente, en lugar de seguir parcheando binarios.

## Origen

Se ha tomado como plantilla:

- `N25Q256A_STM32L476G-EVAL`

copiada desde los ejemplos fuente incluidos en `STM32CubeProgrammer`.

## Cambios ya aplicados

1. Identidad del loader

- El descriptor del dispositivo ya se ha cambiado a `MT25QL128A_STM32H743_CUSTOM7`.
- El nombre visible del loader ya no apunta a `N25Q256A` ni a `STM32L476`.

2. Geometria de la memoria

- El tamano expuesto ya se ha cambiado de `32 MB` a `16 MB`.
- El numero de sectores ya se ha ajustado a `256 x 64 KB`.

3. Pinout objetivo

- La fuente del loader ya refleja el pinout `CUSTOM7`:
  - `PB2`  -> `CLK`
  - `PB10` -> `NCS`
  - `PF8`  -> `IO0`
  - `PF9`  -> `IO1`
  - `PF7`  -> `IO2`
  - `PF6`  -> `IO3`

4. Region de ejecucion

- Los ficheros de proyecto se han movido a una RAM de trabajo tipo `H7`:
  - `0x24000004`

## Lo que falta para tener un STLDR utilizable

Todavia no es un loader compilable para la placa final. Falta portar la capa de bajo nivel desde `STM32L4` a `STM32H7`.

En concreto:

1. Sustituir la libreria antigua `stm32l4xx_*` por la capa correspondiente de `STM32H7`.
2. Rehacer la configuracion del periferico QSPI/QUADSPI con los registros y relojes del `STM32H743`.
3. Revisar la configuracion de funciones alternativas de los pines, ya que en `H7` el pinout `CUSTOM7` mezcla lineas con `AF9` y `AF10`.
4. Validar que los comandos y el tratamiento del chip `MT25QL128A` son correctos para borrado, escritura, lectura y modo mapeado.
5. Compilar el proyecto con una toolchain compatible con el formato `STLDR` que reconoce `STM32CubeProgrammer`.

## Estado actual

Esta carpeta ya sirve como punto de partida serio para el port:

- ya no parte de una flash Winbond distinta,
- ya no parte de una geometria incorrecta,
- y ya no parte de un pinout que no coincide con la placa.

Lo que queda por hacer es el port real del periferico y la compilacion del loader.
