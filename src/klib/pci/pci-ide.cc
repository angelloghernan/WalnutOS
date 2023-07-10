#include "pci-ide.hh"
#include "../ports.hh"
#include "../idt.hh"
#include "../assert.hh"
#include "../pagetables.hh"

using namespace pci;
using enum IDEController::ChannelType;
using enum IDEController::Register;

IDEController::IDEController(u32 bar_4) {
    // TODO: Change to check for BARs depending on compat. mode?
    channel_registers[0].io_base = 0x1F0;
    channel_registers[0].control = 0x3F6;
    channel_registers[0].bus_master_ide = bar_4;

    channel_registers[1].io_base = 0x170;
    channel_registers[1].control = 0x376;
    channel_registers[1].bus_master_ide = bar_4 + 8;

    auto constexpr disable_interrupt
        = static_cast<u8>(ControlBits::InterruptDisable);

    write(ChannelType::Primary, Register::Control, disable_interrupt);
    write(ChannelType::Secondary, Register::Control, disable_interrupt);

    detect_drives();
}

void IDEController::detect_drives() {
    u8 count = 0;
    for (u8 i = 0; i < 2; ++i) {
        auto found_drive = false;
        for (u8 j = 0; j < 2; ++j) {
            devices[count].reserved = false;

            u8 const select_master = static_cast<u8>(0xA0) | (j << 4);

            // Channel is either primary (0) or secondary (1)
            auto const channel = static_cast<ChannelType>(i);

            // Control type is either Master (0) or Slave (1)
            auto const control_type = static_cast<ControlType>(j);

            write(channel, Register::HDDevSel, select_master);

            interrupts::sleep(1);

            write(channel, Register::Command, 
                  static_cast<u8>(Command::Identify));

            interrupts::sleep(1);
            auto const drive = i == 0 ? str("Master drive") : str("Slave drive");
            auto const channel_type = j == 0 ? str("primary") : str("secondary");

            if (read(channel, Register::Status) == 0) {
                terminal.print_line(drive, " ", channel_type, " channel is inactive");
                continue;
            }

            bool had_error = false;

            while (true) {
                auto const status = read(channel, Register::Status);
                if (status & static_cast<u8>(Status::Error)) {
                    terminal.print_line("Drive ", i, " ", j, " had an error");
                    had_error = true;
                    break;
                }

                auto const busy = status & static_cast<u8>(Status::Busy);
                auto const ready = status & static_cast<u8>(Status::DataRequestReady);

                if (!busy && ready) {
                    break;
                }
            }

            auto if_type = InterfaceType::ATA;

            terminal.print_line("DRIVE REPORT: ", drive, " ", channel_type);

            if (!had_error) {
                auto c_lower = read(channel, Register::LBA1);
                auto c_higher = read(channel, Register::LBA2);
                terminal.print("Interface type: ");

                if ((c_lower == 0x69 && c_higher == 0x96) ||
                    c_lower == 0x14) {
                    if_type = InterfaceType::ATAPI;
                    terminal.print_line("ATAPI");
                } else {
                    terminal.print_line("Unknown. LBA1/2 is ", 
                                        (void*)(c_lower), " and ", (void*)(c_higher),
                                        ". Assuming ATA.");
                    // continue;
                }

                write(channel, Register::Command, static_cast<u8>(Command::IdentifyPacket));
                interrupts::sleep(1);
            }

            read_buffer(channel, Register::Data, 256);

            auto const buf_ptr = buffer.data();

            devices[count].reserved = true;
            devices[count].interface_type = if_type;
            devices[count].channel_type = channel;
            devices[count].control_type = control_type;
            devices[count].drive_signature = *((u16*)(buf_ptr + static_cast<u8>(IdentityField::DeviceType)));
            devices[count].capabilities = *((u16*)(buf_ptr + static_cast<u8>(IdentityField::Capabilities)));
            devices[count].command_sets = *((u32*)(buf_ptr + static_cast<u8>(IdentityField::CommandSets)));

            terminal.print("Addressing scheme: ");

            if (devices[count].command_sets & (1 << 26)) {
                // Using extended (48-bit) addressing
                terminal.print_line("48-bit");
                devices[count].size = *((u32*)(buf_ptr + static_cast<u8>(IdentityField::MaxLBAExt)));
            } else {
                // Using standard (32-bit) addressing
                terminal.print_line("32-bit");
                devices[count].size = *((u32*)(buf_ptr + static_cast<u8>(IdentityField::MaxLBA)));
            }
            
            terminal.print("Name: ");

            for (auto k = 0; k < 40; k += 2) {
                // Name (model) is stored in 16-bit little-endian chunks
                devices[count].model[k] = buffer[static_cast<u8>(IdentityField::Model) + k + 1];
                devices[count].model[k + 1] = buffer[static_cast<u8>(IdentityField::Model) + k];

                if (devices[count].model[k] == '\0') {
                    break;
                }
                terminal.put_char(devices[count].model[k]);

                if (devices[count].model[k + 1] == '\0') {
                    break;
                }
                terminal.put_char(devices[count].model[k + 1]);
            }

            terminal.put_char('\n');
            devices[count].model.last() = '\0';
            terminal.print_line("Size: ", devices[count].size, " total sectors, or ", 
                                devices[count].size * 512 / (1024 * 1024), " MB");

            terminal.print_line("");

            ++count;
            found_drive = true;
        }

//        if (found_drive) {
//            PRDT::ChannelType channel = i == 0 ?
//                                        PRDT::ChannelType::Primary :
//                                        PRDT::ChannelType::Secondary;
//            auto const result = 
//                prdts[i].initialize(PAGESIZE / sizeof(PRD_Entry), 
//                                    channel_registers[i].bus_master_ide, channel);
//            assert(result.is_ok(), "Failed to allocate memory for PRD table");
//        }

    }
}

void IDEController::read_drive_dma(ChannelType channel_type) {
    auto const u8_channel = static_cast<u8>(channel_type);
    auto const& channel = channel_registers[u8_channel];

    write(channel_type, Register::Command, static_cast<u8>(Command::ReadDMAExt));
}

void IDEController::enable_hob(ChannelType channel_type) {
    auto const u8_channel = static_cast<u8>(channel_type);
    auto const& channel = channel_registers[u8_channel];
    u8 en_hob = u8(ControlBits::HighOrderByte) | channel.no_interrupts;

    write(channel_type, Register::Control, en_hob);
}

void IDEController::disable_hob(ChannelType channel_type) {
    auto const u8_channel = static_cast<u8>(channel_type);
    auto const& channel = channel_registers[u8_channel];

    IDEController::write(channel_type, Register::Control,
                         channel.no_interrupts);
}

auto IDEController::read(ChannelType const channel_type, 
                         Register const reg) -> u8 {

    auto const u8_channel = static_cast<u8>(channel_type);
    
    auto const u8_reg = static_cast<u8>(reg);

    auto const& channel = channel_registers[u8_channel];

    auto const reg_type = register_type(reg);

    u8 result;

    if (reg_type == RegisterType::HighLevel) {
        enable_hob(channel_type);
    }

    switch (reg_type) {
        case RegisterType::LowLevel:
            result = ports::inb(channel.io_base + u8_reg);
          break;
        case RegisterType::HighLevel:
            result = ports::inb(channel.io_base + u8_reg - 0x06);
          break;
        case RegisterType::DeviceControlOrStatus:
            result = ports::inb(channel.control + u8_reg - 0x0A);
          break;
        case RegisterType::BusMasterIDE:
            result = ports::inb(channel.bus_master_ide + u8_reg - 0x0E);
          break;
    }

    if (reg_type == RegisterType::HighLevel) {
        disable_hob(channel_type);
    }

    return result;
}

void IDEController::write(ChannelType const channel_type, 
                          Register const reg, u8 const data) {
    auto const u8_channel = static_cast<u8>(channel_type);
    
    auto const u8_reg = static_cast<u8>(reg);

    auto const& channel = channel_registers[u8_channel];

    auto const reg_type = register_type(reg);
    
    if (reg_type == RegisterType::HighLevel) {
        enable_hob(channel_type);
    }

    switch (reg_type) {
        case RegisterType::LowLevel:
            ports::outb(channel.io_base + u8_reg, data);
          break;
        case RegisterType::HighLevel:
            ports::outb(channel.io_base + u8_reg - 0x06, data);
          break;
        case RegisterType::DeviceControlOrStatus:
            ports::outb(channel.control + u8_reg - 0x0A, data);
          break;
        case RegisterType::BusMasterIDE:
            ports::outb(channel.bus_master_ide + u8_reg - 0x0E, data);
          break;
    }

    if (reg_type == RegisterType::HighLevel) {
        disable_hob(channel_type);
    }
}

void IDEController::read_buffer(ChannelType const channel_type, 
                                Register const reg, 
                                u32 const count) {

    auto const u8_channel = static_cast<u8>(channel_type);

    auto const u8_reg = static_cast<u8>(reg);

    auto const& channel = channel_registers[u8_channel];

    auto const reg_type = register_type(reg);
    
    auto const buf_ptr = reinterpret_cast<uptr>(&buffer[0]);
    
    if (reg_type == RegisterType::HighLevel) {
        enable_hob(channel_type);
    }

    switch (reg_type) {
        case RegisterType::LowLevel:
            ports::insw(channel.io_base + u8_reg, 
                        buf_ptr, count / 2);
          break;
        case RegisterType::HighLevel:
            ports::insw(channel.io_base + u8_reg - 0x06, 
                        buf_ptr, count / 2);
          break;
        case RegisterType::DeviceControlOrStatus:
            ports::insw(channel.control + u8_reg - 0x0A, 
                        buf_ptr, count / 2);
          break;
        case RegisterType::BusMasterIDE:
            ports::insw(channel.bus_master_ide + u8_reg - 0x0E, 
                        buf_ptr, count / 2);
          break;
    }
    
    if (reg_type == RegisterType::HighLevel) {
        disable_hob(channel_type);
    }
}

auto IDEController::register_type(Register reg) -> RegisterType {
    auto const u8_reg = static_cast<u8>(reg);

    if (u8_reg < 0x08) {
        return RegisterType::LowLevel;
    }

    if (u8_reg >= 0x08 && u8_reg <= 0x0B) {
        return RegisterType::HighLevel;
    }
    
    if (u8_reg < 0x0E) {
        return RegisterType::DeviceControlOrStatus;
    }

    return RegisterType::BusMasterIDE;
}

