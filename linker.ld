ENTRY(_start)
SECTIONS {
  . = SEGMENT_START("text-segment", 0x400000) + SIZEOF_HEADERS;
  .text : { *(.text) }
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data .data.*) }
  .bss : { *(.bss .bss.*) *(COMMON) }
  /DISCARD/ : { *(.note.*) *(.comment) }
}
