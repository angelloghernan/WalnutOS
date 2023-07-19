#include "../klib/strings.hh"
#include "../klib/console.hh"
#include "../klib/array.hh"
#include "../klib/pic.hh"
#include "../klib/apic.hh"
#include "../klib/idt.hh"
#include "../klib/assert.hh"
#include "../kernel/kernel.hh"
#include "../klib/result.hh"
#include "../klib/ps2/ps2.hh"
#include "../klib/ps2/keyboard.hh"
#include "../klib/circular_buffer.hh"
#include "../klib/pci/pci.hh"
#include "../klib/pci/pci-ide.hh"
#include "../klib/ahci/ahci.hh"

using pagetables::PageDirectory;
using pagetables::PageTable;
using ps2::Ps2Keyboard;

// Special, static variables for the starting page directory.
PageDirectory kernel_pagedir;
static PageTable starter_pt;
static PageTable io_pt;

Option<ahci::AHCIState&> sata_disk0;

Idt idt;
Idtr idtr;
extern void* isr_stub_table[];
extern Ps2Keyboard keyboard;

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
    auto& pci_state = pci::PCIState::get();
    auto bar_4 = 0;

    for (auto i = 0; i < 10; ++i) {
        for (auto j = 0; j < 10; ++j) {
            for (auto k = 0; k < 10; ++k) {
                auto vendor_id = pci_state.config_read_word(i, j, k, pci::Register::VendorId);
                if (vendor_id == pci::NO_VENDOR) {
                    break;
                }
                if (vendor_id != pci::NO_VENDOR)  {
                    auto class_code = pci_state.config_read_byte(i, j, k, pci::Register::ClassCode);
                    auto subclass = pci_state.config_read_byte(i, j, k, pci::Register::Subclass);
                    auto prog_if = pci_state.config_read_byte(i, j, k, pci::Register::ProgIF);
                    bar_4 = pci_state.config_read_u32(i, j, k, pci::Register::GDBaseAddress4);
                    auto device_id = pci_state.config_read_word(i, j, k, pci::Register::DeviceId);
                    auto int_line = pci_state.config_read_byte(i, j, k, pci::Register::InterruptLine);

                    terminal.print_line(i, " ", j, " device id ",
                                        (void*)device_id, " vendor ", (void*)(vendor_id), 
                                        " int line ", (void*)int_line);

                    if (class_code == 0x01 && subclass == 0x01) {
                        // pci::IDEController ide_controller(bar_4);
                        // Device is an IDE controller, which is what we were looking for
                        // terminal.print_line(i, " ", j, " is the boot drive controller");
                        // terminal.print_line(i, " ", j, " has base addr ", (void*)(base_addr));
                    } else {
                        // terminal.print_line(i, " " , j, " has class ", class_code, " and subclass ", subclass);
                    }
                }
            }
        }
    }

    // Time to test AHCIState, finally
    sata_disk0 = ahci::AHCIState::find();
    terminal.print_line("Finished");

    assert(sata_disk0.some(), "Unable to find hard disk");

    sata_disk0->enable_interrupts();

    terminal.print_line("One");

    Array<u8, 512> hello_ahci {'H', 'e', 'l', 'l', 'o', ',', ' ', 'A', 'H', 'C', 'I'};

    auto attempt = sata_disk0->write(Slice<u8>(hello_ahci).to_raw_bytes(), 0);

    assert(attempt.is_ok(), "Failure 1");

//    Array<u8, hello_ahci.len()> buf;
//    Slice<u8> buf2(buf);
//    terminal.print_line("Two");
//
//    attempt = sata_disk0->read(buf2, hello_ahci.len());
//    assert(attempt.is_ok(), "Failure 2");
//    terminal.print_line("Three");


//    terminal.print_line();

    keyboard.enqueue_command(ResetAndSelfTest);
    keyboard.enqueue_command(Echo);

    bool left_shift_pressed = false;
    bool right_shift_pressed = false;
    bool extended = false;

    while (true) {
        using enum ps2::KeyboardResponse;
        for (auto maybe_key_code = keyboard.pop_response(); 
             maybe_key_code.some();
             maybe_key_code = keyboard.pop_response()) {
            auto key_code = maybe_key_code.unwrap();
            auto key = [&]{
                if (!left_shift_pressed && !right_shift_pressed) {
                    return Ps2Keyboard::response_to_char(key_code);
                } else {
                    return Ps2Keyboard::response_to_shifted_char(key_code);
                }
            }();

            if (key != '\0') {
                terminal.print(key);
                continue;
            } 

            if (!extended) {
                switch (key_code) {
                    case BackspaceDown:
                        terminal.print_back_char(' ');
                        break;
                    case LeftShiftDown:
                        left_shift_pressed = true;
                        break;
                    case RightShiftDown:
                        right_shift_pressed = true;
                        break;
                    case LeftShiftUp:
                        left_shift_pressed = false;
                        break;
                    case RightShiftUp:
                        right_shift_pressed = false;
                        break;
                    case NextIsExtended:
                        extended = true;
                        break;
                    default:
                        break;
                }
            } else {
                switch (key_code) {
                    case ExCursorUpDown:
                        terminal.row_up();
                        break;
                    case ExCursorDownDown:
                        terminal.row_down();
                        break;
                    case ExCursorLeftDown:
                        terminal.col_back();
                        break;
                    case ExCursorRightDown:
                        terminal.col_forward();
                        break;
                    default:
                        break;
                }
                extended = false;
            }
        }
        __asm__ volatile ("hlt");
    }
}

/// Enable paging by setting up the kernel pagedir and switching to it.
void setup_pagedir() {
    kernel_pagedir.add_pagetable(0, starter_pt, PTE_PW);
    kernel_pagedir.add_pagetable(1019, io_pt, PTE_PW);

    auto result = kernel_pagedir.try_map(0, 0, 0);

    assert(result.is_ok(), "Failure on initial maps!");

    // Identity map everything else
    for (auto address = 0x1000; address < 0x400000; address += PAGESIZE) {
        auto result = kernel_pagedir[0].try_map(address, address, PTE_PW);
        assert(result.is_ok(), "Failure on initial maps!");
    }

    kernel_pagedir.set_page_directory();
    pagetables::enable_paging();
}

void Idt::init() {
    idtr.set_base(reinterpret_cast<uptr>(&_idt[0]));
    idtr.set_limit(sizeof(IdtEntry) * 63);
    for (auto i = 0; i < 64; ++i) {
        auto const ptr = reinterpret_cast<uptr>(isr_stub_table[i]) - kernel::KERNEL_START;
        auto const code_segment = (ptr / kernel::SEGMENT_SIZE) + kernel::KERNEL_CS_SEG_START;
        _idt[i].set(isr_stub_table[i], 0x8E, code_segment);
    }

    // _idt[0x21].set(reinterpret_cast<void*>(keyboard_handler), 0x8E);

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
}
