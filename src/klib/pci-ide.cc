#include "pci-ide.hh"
#include "ports.hh"
#include "idt.hh"

using namespace pci;
using enum IDEController::ChannelType;
using enum IDEController::Register;

IDEController::IDEController() {
    channel_registers[0].io_base = 0x1F0;
    channel_registers[0].control = 0x3F6;
    channel_registers[0].bus_master_ide = 0;

    channel_registers[1].io_base = 0x170;
    channel_registers[1].control = 0x376;
    channel_registers[1].bus_master_ide = 8;

    auto constexpr en_interrupt 
        = static_cast<u8>(ControlBits::InterruptDisable);

    write(ChannelType::Primary, Register::Control, en_interrupt);
    write(ChannelType::Secondary, Register::Control, en_interrupt);

    detect_drives();
}

void IDEController::detect_drives() {
    u8 count = 0;
    for (u8 i = 0; i < 2; ++i) {
        for (u8 j = 0; j < 2; ++j) {
            devices[count].reserved = false;

            u8 const select_master = static_cast<u8>(0xA0) | (j << 4);

            auto const channel = static_cast<ChannelType>(i);

            write(channel, Register::HDDevSel, select_master);

            interrupts::sleep(1);

            write(channel, Register::Command, 
                  static_cast<u8>(Command::Identify));

            interrupts::sleep(1);

            if (read(channel, Register::Status) == 0) {
                continue;
            }   
        }
    }
}

void IDEController::enable_hob(ChannelType channel_type) {
    auto const u8_channel = static_cast<u8>(channel_type);
    auto const& channel = channel_registers[u8_channel];
    auto en_hob = u8(ControlBits::HighOrderByte) | channel.no_interrupts;

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
                                uptr const buffer, 
                                u32 const count) {

    auto const u8_channel = static_cast<u8>(channel_type);

    auto const u8_reg = static_cast<u8>(reg);

    auto const& channel = channel_registers[u8_channel];

    auto const reg_type = register_type(reg);
    
    if (reg_type == RegisterType::HighLevel) {
        enable_hob(channel_type);
    }

    switch (reg_type) {
        case RegisterType::LowLevel:
            ports::insw(channel.io_base + u8_reg, 
                        buffer, count / 2);
          break;
        case RegisterType::HighLevel:
            ports::insw(channel.io_base + u8_reg - 0x06, 
                        buffer, count / 2);
          break;
        case RegisterType::DeviceControlOrStatus:
            ports::insw(channel.control + u8_reg - 0x0A, 
                        buffer, count / 2);
          break;
        case RegisterType::BusMasterIDE:
            ports::insw(channel.bus_master_ide + u8_reg - 0x0E, 
                        buffer, count / 2);
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

