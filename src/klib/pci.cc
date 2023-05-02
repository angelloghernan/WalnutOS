#include "pci.hh"
#include "int.hh"
#include "option.hh"

using namespace pci;

auto PCIState::config_read_word(u8 const bus, u8 const slot,
                                u8 const func_number, u8 const offset) -> u16 {
    auto const ext_bus = u32(bus);
    auto const ext_slot = u32(slot);
    auto const ext_func = u32(func_number);


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
                  (offset & 0xFC); 

    ports::outl(CONFIG_ADDRESS, address);

    // Magic: read the first word of the data register
    u16 ret = (ports::inl(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF;

    return ret;
}

auto PCIState::check_vendor(u8 const bus, 
                            u8 const slot) -> Nullable<u16, NO_VENDOR> {
    return config_read_word(bus, slot, 0, OFFSET_VENDOR_ID);
}

auto PCIState::check_device_id(u8 const bus,
                               u8 const slot) -> Nullable<u16, NO_DEVICE> {
    return config_read_word(bus, slot, 0, OFFSET_DEVICE_ID);
}

auto PCIState::check_header_type(u8 const bus,
                                 u8 const slot) -> HeaderType {
    auto byte = config_read_word(bus, slot, 0, 
                                 OFFSET_HEADER_TYPE) & 0xFF;
    if (byte & 0x10000000) {
        return HeaderType::MultiFunction;
    }

    return static_cast<HeaderType>(byte);
}
