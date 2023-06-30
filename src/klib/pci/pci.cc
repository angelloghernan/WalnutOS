#include "pci.hh"
#include "../int.hh"
#include "../option.hh"

using namespace pci;

auto PCIState::config_read_word(u8 const bus, u8 const slot,
                                u8 const func_number, Register const offset) -> u16 {
    auto const ext_bus = u32(bus);
    auto const ext_slot = u32(slot);
    auto const ext_func = u32(func_number);
    auto const offset_u8 = static_cast<u8>(offset);

    // Layout:
    // Bit 31: Enable bit
    // Bits 30-24: Reserved
    // Bits 23-16: Bus number
    // Bits 15-11: Slot/device number
    // Bits 10-8: Function number
    // Bits 7-0: Register offset
    u32 address = (u32(0x80000000)) |
                  (ext_bus << 16)   |
                  (ext_slot << 11)  |
                  (ext_func << 8)   |
                  (offset_u8 & 0xFC); 

    ports::outl(CONFIG_ADDRESS, address);

    // Magic: read the first word (16 bits) of the data register
    return (ports::inl(CONFIG_DATA) >> ((offset_u8 & 2) * 8)) & 0xFFFF;
}

auto PCIState::config_read_u32(u8 const bus, u8 const slot, 
                               u8 const func_number, Register const offset) -> u32 {
    auto const ext_bus = u32(bus);
    auto const ext_slot = u32(slot);
    auto const ext_func = u32(func_number);
    auto const offset_u8 = static_cast<u8>(offset);

    // Layout:
    // Bit 31: Enable bit
    // Bits 30-24: Reserved
    // Bits 23-16: Bus number
    // Bits 15-11: Slot/device number
    // Bits 10-8: Function number
    // Bits 7-0: Register offset
    u32 address = (u32(0x80000000)) |
                  (ext_bus << 16)   |
                  (ext_slot << 11)  |
                  (ext_func << 8)   |
                  (offset_u8); 

    ports::outl(CONFIG_ADDRESS, address);

    return ports::inl(CONFIG_DATA);
}

auto PCIState::config_read_byte(u8 const bus, u8 const slot,
                                u8 const func_number, Register const offset) -> u8 {
    auto const word = config_read_word(bus, slot, func_number, offset);
    auto const offset_u8 = static_cast<u8>(offset);

    if (offset_u8 & 0b1) {
        return word >> 8;
    } else {
        return word & 0xFF;
    }
}

auto PCIState::check_vendor(u8 const bus, 
                            u8 const slot) -> Nullable<u16, NO_VENDOR> {
    return config_read_word(bus, slot, 0, Register::VendorId);
}

auto PCIState::check_device_id(u8 const bus,
                               u8 const slot) -> Nullable<u16, NO_DEVICE> {
    return config_read_word(bus, slot, 0, Register::DeviceId);
}

auto PCIState::check_header_type(u8 const bus,
                                 u8 const slot) -> HeaderType {
    auto const byte = config_read_word(bus, slot, 0, 
                                       Register::HeaderType) & 0xFF;
    if (byte & 0x10000000) {
        return HeaderType::MultiFunction;
    }

    return static_cast<HeaderType>(byte);
}

auto PCIState::get_status(u8 const bus, u8 const slot) -> Option<status_register> {
    auto const bytes = config_read_word(bus, slot, 0, Register::Status);
    if (bytes == 0xFFFF) {
        return {};
    }
    return status_register {bytes};
}
