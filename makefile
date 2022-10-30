# $@ = target file
# $< = first dependency
# $^ = all dependencies

CPP_SOURCES = $(wildcard *.cc)
HEADERS = $(wildcard *.hh)
OBJ = ${CPP_SOURCES:.cc=.o}

CC = i686-elf-g++
CFLAGS = -g
GDB = gdb

all: run

os-image.bin: boot/boot.bin kernel.bin
		cat $^ > $@

kernel.bin: boot/kernel_entry.o ${OBJ}
		i686-elf-ld -o $@ -Ttext 0x1000 $^ --oformat binary

kernel.elf: boot/kernel_entry.o ${OBJ}
		i686-elf-ld -o $@ -Ttext 0x1000 $^ 

run: os-image.bin
		qemu-system-i386 -hda $<

gdb: os-image.bin kernel.elf
		qemu-system-i386 -hda $< -S -s & \
		${GDB}

%.o: %.cc ${HEADERS}
		${CC} ${CFLAGS} -ffreestanding -c $< -o $@

%.o: %.asm
		nasm $< -f elf -o $@

%.bin: %.asm
		nasm $< -f bin -o $@

clean:
	rm -rf *.bin *.dis *.o os-image.bin *.elf
	rm -rf *.o boot/*.bin boot/*.o
