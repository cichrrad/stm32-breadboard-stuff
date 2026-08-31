/* STM32G491RE Memory Map */
MEMORY
{
  FLASH (rx) : ORIGIN = 0x08008000, LENGTH = 480K
  RAM (rwx)  : ORIGIN = 0x20000000, LENGTH = 112K
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS
{
  .text :
  {
    KEEP(*(.isr_vector)) 
    *(.text*)
    *(.rodata*)        /* Read-only data (like string literals) */
    . = ALIGN(4);
    _sidata = .;       /* Store the end of FLASH (start of init data) */
  } > FLASH

  /* .data goes to RAM, but its initial values are stored in FLASH at _sidata */
  .data : AT(_sidata)
  {
    . = ALIGN(4);
    _sdata = .;        /* Start of .data in RAM */
    *(.data*)
    . = ALIGN(4);
    _edata = .;        /* End of .data in RAM */
  } > RAM

  .bss :
  {
    . = ALIGN(4);
    _sbss = .;         /* Start of .bss in RAM */
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    _ebss = .;         /* End of .bss in RAM */
  } > RAM
}