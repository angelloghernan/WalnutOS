# $@ = target file
# $< = first dependency
# $^ = all dependencies

CPP_SOURCES = $(wildcard kernel/*.cc klib/*.cc)
HEADERS = $(wildcard kernel/*.hh klib/*.hh)
OBJ = ${CPP_SOURCES:.cc=.o} klib/idt.o


CC = i686-elf-g++ 
CFLAGS = -g -std=c++20 -ffreestanding -nostdlib -lgcc -Wall -O3 -flto -ffat-lto-objects
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

run: os-image.bin
		qemu-system-i386 -hda $< -d int 

run-console: os-image.bin
		qemu-system-i386 -hda $< -display curses

gdb: os-image.bin kernel.elf
		qemu-system-i386 -hda $< -S -s -d int -no-reboot -no-shutdown & \
		${GDB}

%.o: %.cc ${HEADERS}
		${CC} ${CFLAGS} -c $< -o $@

%.o: %.asm
		nasm $< -f elf -o $@

%.bin: %.asm
		nasm $< -f bin -o $@

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf *.o boot/*.bin boot/*.o klib/*.o

line_count:
	find . -name '*.asm' | xargs wc -l && \
	find . -name '*.cc'  | xargs wc -l && \
	find . -name '*.hh'  | xargs wc -l
