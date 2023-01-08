SECTIONS
{
  . = 0x1000;
  .text BLOCK(4K) : ALIGN(4K) { *(.text) }
  .data BLOCK(4K) : ALIGN(4K) { 
      *(.data) 
      *(.gnu.linkonce.r*)
  }
  .rodata : ALIGN(4K) {
        start_ctors = .;
        *(SORT(.ctors*))
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)
  }
  .init : { *(.init) }
  .fini : { *(.fini) }
}
