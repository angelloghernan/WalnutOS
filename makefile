# $@ = target file
# $< = first dependency
# $^ = all dependencies

CPP_SOURCES = $(wildcard src/klib/*.cc src/klib/*/*.cc src/kernel/*.cc)
HEADERS = $(wildcard src/kernel/*.hh src/klib/*.hh src/klib/*/*.hh)
OBJ = ${CPP_SOURCES:.cc=.o} src/klib/idt.o
DEBUG_OBJ = ${CPP_SOURCES:%.cc=debug/%.o} src/klib/idt.o

CC = i686-elf-g++ 
CXXFLAGS += -g -std=c++20 -fmodules-ts -ffreestanding -nostdlib -lgcc -lsupc++ -Wall -flto -ffat-lto-objects \
         				   -fno-threadsafe-statics -fno-stack-protector -fno-exceptions
DEBUG_FLAGS = -DDEBUG -O1
RELEASE_FLAGS = -O3
GDB = gdb

CRTI_OBJ = src/boot/crti.o
CRTN_OBJ = src/boot/crtn.o
CRT0_OBJ = src/boot/crt0.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CXXFLAGS) -print-file-name=crtend.o)
OBJ_LINK_LIST:= $(CRT0_OBJ) $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)
DEBUG_OBJ_LINK_LIST:= $(CRT0_OBJ) $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(DEBUG_OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)

all: run

os-image.bin: src/boot/boot.bin kernel.bin
		cat $^ > $@

kernel.bin: ${OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin  -o $@ --script=ldconfig.ld $^ --oformat binary

debug/kernel.bin: ${DEBUG_OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin  -o $@ --script=ldconfig.ld $^ --oformat binary

kernel.elf: ${OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin -o $@ --script=ldconfig.ld $^ 

dump: kernel.elf
		objdump -d kernel.elf > dump.txt

debug/os-image.bin: src/boot/boot.bin debug/kernel.bin
		cat $^ > $@

debug-file-structure:
	   find src -type d -exec mkdir -p -- debug/{} \;

debug: debug-file-structure debug/os-image.bin 
		qemu-system-i386 -hda debug/os-image.bin

run: os-image.bin
		qemu-system-i386 -hda $<

run-debug-int: os-image.bin
		qemu-system-i386 -hda $< -d int -no-reboot -no-shutdown

run-console: os-image.bin
		qemu-system-i386 -hda $< -display curses

gdb: os-image.bin kernel.elf
		qemu-system-i386 -hda $< -S -s -no-reboot -no-shutdown & \
		${GDB}

debug/%.o: %.cc
		${CC} ${CXXFLAGS} ${DEBUG_FLAGS} -MMD -c $< -o $@

%.o: %.cc
		${CC} ${CXXFLAGS} ${RELEASE_FLAGS} -MMD -c $< -o $@

-include $(wildcard **/*.d)

%.o: %.asm
		nasm $< -f elf -o $@

%.bin: %.asm
		nasm $< -f bin -o $@

clean:
	find . -type f -name '*.o' -delete
	find . -type f -name '*.bin' -delete
	find . -type f -name '*.elf' -delete
	find . -type f -name '*.dis' -delete
	#rm -rf *.o boot/*.bin boot/*.o klib/*.o klib/*/*.o kernel/*.o obj/*
	#rm -rf *.o boot/*.bin boot/*.o klib/*.o klib/*/*.o kernel/*.o obj/*

summary:
	find . -name '*.asm' | xargs wc -l && \
	find . -name '*.cc'  | xargs wc -l && \
	find . -name '*.hh'  | xargs wc -l
