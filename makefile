# $@ = target file
# $< = first dependency
# $^ = all dependencies

CPP_SOURCES = $(wildcard klib/*.cc klib/*/*.cc kernel/*.cc)
HEADERS = $(wildcard kernel/*.hh klib/*.hh klib/*/*.hh)
OBJ = ${CPP_SOURCES:.cc=.o} klib/idt.o


CC = i686-elf-g++ 
CFLAGS = -g -std=c++20 -fmodules-ts -ffreestanding -nostdlib -lgcc -lsupc++ -Wall -O2 -flto -ffat-lto-objects \
         -fno-threadsafe-statics -fno-stack-protector -fno-exceptions
GDB = gdb

CRTI_OBJ = boot/crti.o
CRTN_OBJ = boot/crtn.o
CRT0_OBJ = boot/crt0.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
OBJ_LINK_LIST:= $(CRT0_OBJ) $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)

all: run

os-image.bin: boot/boot.bin kernel.bin
		cat $^ > $@

kernel.bin: ${OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin  -o $@ --script=ldconfig.ld $^ --oformat binary

kernel.elf: ${OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin -o $@ --script=ldconfig.ld $^ 

dump: kernel.elf
		objdump -d kernel.elf > dump.txt

run: os-image.bin
		qemu-system-i386 -hda $<

run-debug-int: os-image.bin
		qemu-system-i386 -hda $< -d int -no-reboot -no-shutdown

run-console: os-image.bin
		qemu-system-i386 -hda $< -display curses

gdb: os-image.bin kernel.elf
		qemu-system-i386 -hda $< -S -s -no-reboot -no-shutdown & \
		${GDB}

%.o: %.cc
		${CC} ${CFLAGS} -MMD -c $< -o $@

-include $(wildcard kernel/*.d klib/*.d klib/*/*.d)

%.o: %.asm
		nasm $< -f elf -o $@

%.bin: %.asm
		nasm $< -f bin -o $@

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf *.o boot/*.bin boot/*.o klib/*.o klib/*/*.o kernel/*.o obj/*

summary:
	find . -name '*.asm' | xargs wc -l && \
	find . -name '*.cc'  | xargs wc -l && \
	find . -name '*.hh'  | xargs wc -l
