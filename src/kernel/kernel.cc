#include "wnfs/wnfs.hh"
#include "wnfs/cache.hh"
#include "klib/strings.hh"
#include "klib/console.hh"
#include "klib/array.hh"
#include "klib/pic.hh"
#include "klib/apic.hh"
#include "klib/idt.hh"
#include "klib/assert.hh"
#include "klib/result.hh"
#include "klib/circular_buffer.hh"
#include "klib/pci/pci.hh"
#include "klib/pci/pci-ide.hh"
#include "klib/ahci/ahci.hh"
#include "klib/ps2/ps2.hh"
#include "klib/ps2/keyboard.hh"
#include "kernel/alloc.hh"
#include "kernel/kernel.hh"
#include "kernel/ext2/ext2.hh"
#include "kernel/ext2/blocks.hh"
#include "userspace/shell/shell.hh"

using namespace wlib;

using kernel::ext2::Superblock;
using pagetables::PageDirectory;
using pagetables::PageTable;
using ps2::Ps2Keyboard;

// Special, static variables for the starting page directory.
PageDirectory kernel_pagedir;
static PageTable starter_pt;
static PageTable io_pt;

Option<ahci::AHCIState&> sata_disk0 = Option<ahci::AHCIState&>::None();

Idt idt;
Idtr idtr;
extern void* isr_stub_table[];
Ps2Keyboard keyboard;
wnfs::BufCache bufcache;

extern "C" void kernel_main() {
    using enum ps2::KeyboardCommand;
    terminal.clear();
    terminal.print_line("Press F1 to exit.");
    setup_pagedir();

    // Remap master to 0x20, slave to 0x28
    Pic::remap(0x20, 0x28);
    Pic::clear_masks();

    idt.init();

    Idt::enable_interrupts();

    // QEMU: 
    // Slot 0 is Natoma (chipset)
    // Slot 1 is ISA controller
    // Slot 2 is QEMU Virtual Video Controller
    // Slot 3 is an Ethernet device
    // Slot 4 should be the PCI IDE controller

    sata_disk0 = ahci::AHCIState::find();

    assert(sata_disk0.some(), "Unable to find hard disk");

    sata_disk0.unwrap().enable_interrupts();


    /*

    Superblock superblock;

    auto result = superblock.cache_read();

    assert(result.is_ok(), "Error reading ext2 superblock");

    if (superblock.has_signature()) {
        terminal.print_line("Successfully detected ext2 signature");
    }

    result = kernel::ext2::format_disk(&superblock);

    assert(result.is_ok(), "Error formating disk with superblock");
    */

    terminal.print_line("formatting");
    assert(wnfs::format_disk(&sata_disk0.unwrap()).is_ok(), "Error formatting sata disk 0");
    terminal.print_line("Done formatting");

    keyboard.enqueue_command(ResetAndSelfTest);
    keyboard.enqueue_command(Echo);

    shell_main();
}

/// Enable paging by setting up the kernel pagedir and switching to it.
void setup_pagedir() {
    kernel_pagedir.add_pagetable(0, starter_pt, PTE_PW);
    kernel_pagedir.add_pagetable(1019, io_pt, PTE_PW);

    auto result = kernel_pagedir.try_map(0, 0, 0);

    assert(result.is_ok(), "Failure on initial maps!");

    // Identity map everything else
    for (uptr address = 0x1000; address < 0x400000; address += PAGESIZE) {
        auto result = kernel_pagedir[0].try_map(address, address, PTE_PW);
        assert(result.is_ok(), "Failure on initial maps!");
    }

    kernel_pagedir.set_page_directory();
    pagetables::enable_paging();
}

void Idt::init() {
    idtr.set_base(reinterpret_cast<uptr>(&_idt[0]));
    idtr.set_limit(sizeof(IdtEntry) * 63);
    for (usize i = 0; i < 64; ++i) {
        auto const ptr = reinterpret_cast<uptr>(isr_stub_table[i]) - kernel::KERNEL_START;
        auto const code_segment = u16((ptr / kernel::SEGMENT_SIZE) + kernel::KERNEL_CS_SEG_START);
        _idt[i].set(isr_stub_table[i], 0x8E, code_segment);
    }

    // _idt[0x21].set(reinterpret_cast<void*>(keyboard_handler), 0x8E);

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
}
