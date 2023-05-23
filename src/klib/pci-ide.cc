#include "pci-ide.hh"
#include "ports.hh"

using namespace pci;
using enum IDEController::ChannelType;
using enum IDEController::Register;

auto IDEController::read(ChannelType const channel_type, 
                         Register const reg) -> u8 {

    auto const u8_channel = static_cast<u8>(channel_type);
    
    auto const u8_reg = static_cast<u8>(reg);

    auto const& channel = channel_registers[u8_channel];

    auto const reg_type = register_type(reg);

    u8 result;

    if (reg_type == RegisterType::HighLevel) {
        // If this register is a high level register, then enable high order byte
        auto en_hob = u8(ControlBits::HighOrderByte) | channel.no_interrupts;

        IDEController::write(channel_type, Register::Control, en_hob);
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
            result = ports::inb(channel.no_interrupts + u8_reg - 0x0E);
          break;
    }

    if (reg_type == RegisterType::HighLevel) {
        // Disable the high order byte if this is a high level register
        IDEController::write(channel_type, Register::Control,
                             channel.no_interrupts);
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
        auto constexpr reg_control = static_cast<u8>(Register::Control);
        auto en_hob = u8(ControlBits::HighOrderByte) | channel.no_interrupts;
        ports::outb(channel.control + reg_control - 0x0A, en_hob);
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
            ports::outb(channel.no_interrupts + u8_reg - 0x0E, data);
          break;
    }

    if (reg_type == RegisterType::HighLevel) {
        auto constexpr reg_control = static_cast<u8>(Register::Control);
        ports::outb(channel.control + reg_control - 0x0A, 
                    channel.no_interrupts);
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

