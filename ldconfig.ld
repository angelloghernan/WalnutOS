SECTIONS
{
  . = 1M;
  .text BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)
        *(.text) 
  }

  .init : { *(.init) }
  .fini : { *(.fini) }

  .rodata BLOCK(4K) : ALIGN(4K) {
        start_ctors = .;
        *(SORT(.ctors*))
        end_ctors = .;

        start_dtors = .;
        *(SORT(.dtors*))
        end_dtors = .;

        *(.rodata*)
        *(.gnu.linkonce.r*)
  }
  .data BLOCK(4K) : ALIGN(4K) {
        *(.data) 
        *(.gnu.linkonce.r*)
  }

  .bss BLOCK(4K) : ALIGN(4K) {
        *(COMMON)
        *(.bss)
  }
}
