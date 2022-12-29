SECTIONS
{
  . = 0x1000;
  .text BLOCK(4K) : ALIGN(4K) { *(.text) }
  .data BLOCK(4K) : ALIGN(4K) { *(.data) }
  .init : { *(.init) }
  .fini : { *(.fini) }
  .rodata : {
        start_ctors = .;
        *(SORT(.ctors*))  /* Note the "SORT" */
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)
  }
}
