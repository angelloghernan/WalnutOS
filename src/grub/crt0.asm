[bits 32]
[extern kernel_main]
[extern _init]
[extern _fini]

MBOOT_PAGE_ALIGN EQU 1 << 0
MBOOT_MEMORY_INFO EQU 1 << 1
MBOOT_MAGIC EQU 0x1BADB002
MBOOT_FLAGS EQU MBOOT_PAGE_ALIGN | MBOOT_MEMORY_INFO
MBOOT_CHECKSUM EQU -(MBOOT_MAGIC + MBOOT_FLAGS)

STACK_SIZE EQU 0x4000

section .multiboot
align 4

; The multi-boot header proper
dd MBOOT_MAGIC
dd MBOOT_FLAGS
dd MBOOT_CHECKSUM

section .bss
align 16
stack_bottom:
    resb STACK_SIZE
stack_top:

section .text
global _start
_start:
    mov esp, stack_top
    call _init
    call kernel_main
    call _fini
    cli
    hlt_forever:
    hlt
    jmp hlt_forever

