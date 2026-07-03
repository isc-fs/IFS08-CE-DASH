ENTRY(Init)

PHDRS
{
  Loader PT_LOAD;
  DevInfo PT_LOAD;
}

MEMORY
{
  RAM (xrw) : ORIGIN = 0x24000004, LENGTH = 64K
}

SECTIONS
{
  "P1-P2 ro" :
  {
    . = ALIGN(4);
    *(.text)
    *(.text*)
    *(.rodata)
    *(.rodata*)
    *(.data)
    *(.data*)
    . = ALIGN(4);
  } >RAM :Loader

  .ARM.extab :
  {
    *(.ARM.extab* .gnu.linkonce.armextab.*)
  } >RAM :Loader

  .ARM.exidx :
  {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >RAM :Loader

  "P1-P2 ui" (NOLOAD) :
  {
    . = ALIGN(4);
    *(.bss)
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
  } >RAM :Loader

  "P3 ro" :
  {
    KEEP(*(.DevInfo))
    KEEP(*(.DevInfo*))
  } >RAM :DevInfo
}
