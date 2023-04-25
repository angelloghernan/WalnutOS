target remote localhost:1234
break *0x1000
layout asm
layout regs
symbol-file debug/kernel.elf
