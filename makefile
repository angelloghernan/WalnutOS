# $@ = target file
# $< = first dependency
# $^ = all dependencies

all: run

obj/kernel.bin: obj/kernel_entry.o obj/kernel.o
		i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

obj/kernel_entry.o: boot/kernel_entry.asm
		nasm $< -f elf -o $@

obj/kernel.o: kernel.cc
		i686-elf-g++ -ffreestanding -c $< -o $@

obj/kernel.dis: obj/kernel.bin
		ndisasm -b 32 $< > $@

obj/boot.bin: boot/boot.asm boot/vga.asm boot/boot_utils.asm
		nasm $< -f bin -o $@

obj/os-image.bin: obj/boot.bin obj/kernel.bin
		cat $^ > $@

run: obj/os-image.bin
		qemu-system-i386 -hda $<

clean:
		rm obj/*
