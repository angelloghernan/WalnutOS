# $@ = target file
# $< = first dependency
# $^ = all dependencies
# disk:
#    qemu-img create -f raw img/disk.img 10M

OBJ_FOLDER = obj
SRC_FOLDER = src
DEBUG_FOLDER = debug
DEFAULT_ENTRY_FILE = src/kernel/kernel.cc
CPP_SOURCES := $(wildcard ${SRC_FOLDER}/klib/*.cc ${SRC_FOLDER}/klib/*/*.cc ${SRC_FOLDER}/kernel/*.cc ${SRC_FOLDER}/kernel/*/*.cc)
OS_IMAGE = os-image.bin
KERNEL_IMAGE = kernel.bin

KERNEL_IMAGE_DIR = isodir/boot/${KERNEL_IMAGE}

ifdef TEST-FILE
	CPP_SOURCES := $(filter-out $(DEFAULT_ENTRY_FILE), $(CPP_SOURCES))
	CPP_SOURCES += $(TEST-FILE)
	OS_IMAGE := test-os-image.bin
	KERNEL_IMAGE := test-kernel-image.bin
endif

QEMU_FLAGS = -device piix4-ide,bus=pci.0,id=piix4-ide \
	-drive file=${OBJ_FOLDER}/${OS_IMAGE},if=none,format=raw,id=bootdisk\
	-device ide-hd,drive=bootdisk,bus=piix4-ide.0 \
	-drive file=img/disk.img,if=none,format=raw,id=maindisk\
    -device ahci,id=ahci \
	-device ide-hd,drive=maindisk,bus=ahci.0

BOOT_FOLDER = grub

HEADERS = $(wildcard ${SRC_FOLDER}/kernel/*.hh ${SRC_FOLDER}/kernel/*/*.hh ${SRC_FOLDER}/klib/*.hh ${SRC_FOLDER}/klib/*/*.hh)
OBJ = ${CPP_SOURCES:%.cc=${OBJ_FOLDER}/%.o} src/klib/idt.o
DEBUG_OBJ = ${CPP_SOURCES:%.cc=${DEBUG_FOLDER}/%.o} src/klib/idt.o
HEADER_SOURCES = ${CPP_SOURCES:%.cc=%.d}

CC = i686-elf-g++ 
CXXFLAGS += -g -std=c++20 -fmodules-ts -ffreestanding -nostdlib -lgcc -lsupc++ -flto -ffat-lto-objects \
					   -fno-threadsafe-statics -fno-stack-protector -fno-exceptions -fno-use-cxa-atexit -Wall -I./src
DEBUG_FLAGS = -DDEBUG -O1
RELEASE_FLAGS = -O3
GDB = gdb

CRTI_OBJ = ${SRC_FOLDER}/${BOOT_FOLDER}/crti.o
CRTN_OBJ = ${SRC_FOLDER}/${BOOT_FOLDER}/crtn.o
CRT0_OBJ = ${SRC_FOLDER}/${BOOT_FOLDER}/crt0.o
CRTBEGIN_OBJ:=$(shell $(CC) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CXXFLAGS) -print-file-name=crtend.o)
OBJ_LINK_LIST:= $(CRT0_OBJ) $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)
DEBUG_OBJ_LINK_LIST:= $(CRT0_OBJ) $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(DEBUG_OBJ) $(CRTEND_OBJ) $(CRTN_OBJ)

all: run

# ${OBJ_FOLDER}/${OS_IMAGE}: ${SRC_FOLDER}/boot/boot.bin ${OBJ_FOLDER}/${KERNEL_IMAGE}
# cat $^ > $@

${OBJ_FOLDER}/${OS_IMAGE}: ${SRC_FOLDER}/${BOOT_FOLDER}/crt0.o ${KERNEL_IMAGE_DIR}
	grub-mkrescue -o ${OBJ_FOLDER}/${OS_IMAGE} isodir

${KERNEL_IMAGE_DIR}: ${OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin  -o $@ --script=ldconfig.ld $^

${DEBUG_FOLDER}/${KERNEL_IMAGE_DIR}: ${DEBUG_OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin  -o $@ --script=ldconfig.ld $^

${OBJ_FOLDER}/kernel.elf: ${OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin -o $@ --script=ldconfig.ld $^

${DEBUG_FOLDER}/kernel.elf: ${DEBUG_OBJ_LINK_LIST}
		i686-elf-ld -flto -use-linker-plugin -o $@ --script=ldconfig.ld $^ 

dump: ${OBJ_FOLDER}/kernel.elf
		objdump -d $^ > dump.txt

${DEBUG_FOLDER}/${OS_IMAGE}: ${SRC_FOLDER}/boot/boot.bin ${DEBUG_FOLDER}/${KERNEL_IMAGE}
		cat $^ > $@

object-file-structure:
	mkdir ${OBJ_FOLDER}; find ${SRC_FOLDER} -type d -exec mkdir -p -- ${OBJ_FOLDER}/{} \;

debug-file-structure:
	  mkdir ${DEBUG_FOLDER}; find ${SRC_FOLDER} -type d -exec mkdir -p -- ${DEBUG_FOLDER}/{} \;

debug: debug-file-structure ${DEBUG_FOLDER}/${OS_IMAGE} 
		qemu-system-i386 -hda ${DEBUG_FOLDER}/${OS_IMAGE} ${QEMU_FLAGS}

run: object-file-structure ${OBJ_FOLDER}/${OS_IMAGE}
		qemu-system-i386 ${QEMU_FLAGS}

run-debug-int: object-file-structure ${OBJ_FOLDER}/${OS_IMAGE}
		qemu-system-i386 ${QEMU_FLAGS} -d int -no-reboot -no-shutdown

run-console: object-file-structure ${OBJ_FOLDER}/${OS_IMAGE}
		qemu-system-i386 ${QEMU_FLAGS} -display curses

gdb: ${OBJ_FOLDER}/${OS_IMAGE} ${OBJ_FOLDER}/kernel.elf
		qemu-system-i386 ${QEMU_FLAGS} -S -s -no-reboot -no-shutdown & \
		${GDB}

test-%: src/test/%.cc
		make TEST-FILE=$<

test-debug-%: src/test/%.cc
		make debug TEST-FILE=$<

-include $(HEADER_SOURCES)

${DEBUG_FOLDER}/%.o: %.cc
		${CC} ${CXXFLAGS} ${DEBUG_FLAGS} -MMD -c $< -o $@

${OBJ_FOLDER}/%.o: %.cc
		${CC} ${CXXFLAGS} ${RELEASE_FLAGS} -MMD -c $< -o $@

%.o: %.cc
		${CC} ${CXXFLAGS} ${RELEASE_FLAGS} -MMD -c $< -o $@

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
