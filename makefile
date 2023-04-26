# $@ = target file
# $< = first dependency
# $^ = all dependencies

OBJ_FOLDER = obj
SRC_FOLDER = src
DEBUG_FOLDER = debug
DEFAULT_ENTRY_FILE = src/kernel/kernel.cc
CPP_SOURCES := $(wildcard ${SRC_FOLDER}/klib/*.cc ${SRC_FOLDER}/klib/*/*.cc ${SRC_FOLDER}/kernel/*.cc)

ifdef TEST
	CPP_SOURCES := $(filter-out $(DEFAULT_ENTRY_FILE), $(CPP_SOURCES))
	CPP_SOURCES += $(TEST)
endif

HEADERS = $(wildcard ${SRC_FOLDER}/kernel/*.hh ${SRC_FOLDER}/klib/*.hh ${SRC_FOLDER}/klib/*/*.hh)
OBJ = ${CPP_SOURCES:%.cc=${OBJ_FOLDER}/%.o} src/klib/idt.o
DEBUG_OBJ = ${CPP_SOURCES:%.cc=${DEBUG_FOLDER}/%.o} src/klib/idt.o

CC = i686-elf-g++ 
CXXFLAGS += -g -std=c++20 -fmodules-ts -ffreestanding -nostdlib -lgcc -lsupc++ -Wall -flto -ffat-lto-objects \
         				   -fno-threadsafe-statics -fno-stack-protector -fno-exceptions
DEBUG_FLAGS = -DDEBUG -O1
RELEASE_FLAGS = -O3
GDB = gdb

CRTI_OBJ = ${SRC_FOLDER}/boot/crti.o
CRTN_OBJ = ${SRC_FOLDER}/boot/crtn.o
CRT0_OBJ = ${SRC_FOLDER}/boot/crt0.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CXXFLAGS) -print-file-name=crtend.o)
OBJ_LINK_LIST:= $(CRT0_OBJ) $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)
DEBUG_OBJ_LINK_LIST:= $(CRT0_OBJ) $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(DEBUG_OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)

all: run

${OBJ_FOLDER}/os-image.bin: ${SRC_FOLDER}/boot/boot.bin ${OBJ_FOLDER}/kernel.bin
		cat $^ > $@

${OBJ_FOLDER}/kernel.bin: ${OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin  -o $@ --script=ldconfig.ld $^ --oformat binary

${DEBUG_FOLDER}/kernel.bin: ${DEBUG_OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin  -o $@ --script=ldconfig.ld $^ --oformat binary

${OBJ_FOLDER}/kernel.elf: ${OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin -o $@ --script=ldconfig.ld $^ 

${DEBUG_FOLDER}/kernel.elf: ${DEBUG_OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin -o $@ --script=ldconfig.ld $^ 

dump: ${OBJ_FOLDER}/kernel.elf
		objdump -d $^ > dump.txt

${DEBUG_FOLDER}/os-image.bin: ${SRC_FOLDER}/boot/boot.bin ${DEBUG_FOLDER}/kernel.bin
		cat $^ > $@

object-file-structure:
		mkdir ${OBJ_FOLDER}; find ${SRC_FOLDER} -type d -exec mkdir -p -- ${OBJ_FOLDER}/{} \;

debug-file-structure:
	  mkdir ${DEBUG_FOLDER}; find ${SRC_FOLDER} -type d -exec mkdir -p -- ${DEBUG_FOLDER}/{} \;

debug: debug-file-structure ${DEBUG_FOLDER}/os-image.bin 
		qemu-system-i386 -hda ${DEBUG_FOLDER}/os-image.bin

run: object-file-structure ${OBJ_FOLDER}/os-image.bin
		qemu-system-i386 -hda ${OBJ_FOLDER}/os-image.bin

run-debug-int: object-file-structure ${OBJ_FOLDER}/os-image.bin
		qemu-system-i386 -hda $< -d int -no-reboot -no-shutdown

run-console: object-file-structure ${OBJ_FOLDER}/os-image.bin
		qemu-system-i386 -hda $< -display curses

gdb: ${DEBUG_FOLDER}/os-image.bin ${DEBUG_FOLDER}/kernel.elf
		qemu-system-i386 -hda $< -S -s -no-reboot -no-shutdown & \
		${GDB}

test-%: src/test/%.cc
		make TEST=$<

${DEBUG_FOLDER}/%.o: %.cc
		${CC} ${CXXFLAGS} ${DEBUG_FLAGS} -MMD -c $< -o $@

${OBJ_FOLDER}/%.o: %.cc
		${CC} ${CXXFLAGS} ${RELEASE_FLAGS} -MMD -c $< -o $@

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

summary:
	find . -name '*.asm' | xargs wc -l && \
	find . -name '*.cc'  | xargs wc -l && \
	find . -name '*.hh'  | xargs wc -l
