#pragma once
#include "ports.hh"
#include "int.hh"
#include "nullable.hh"
#include "option.hh"

namespace pci {
    auto constexpr static CONFIG_ADDRESS = 0xCF8;
    auto constexpr static CONFIG_DATA = 0xCFC;
    auto constexpr static NO_VENDOR = 0xFFFF_u16;
    auto constexpr static NO_DEVICE = 0xFFFF_u16;
    auto constexpr static OFFSET_VENDOR_ID = 0x0_u16;
    auto constexpr static OFFSET_DEVICE_ID = 0x2_u16;
    auto constexpr static OFFSET_HEADER_TYPE = 0xE_u16;

    enum class HeaderType : u8 {
        GeneralDevice = 0x0,
        PciToPci = 0x1,
        PciToCardBus = 0x2,
        MultiFunction,
    };
    
    class PCIState {
      public:
        auto config_read_word(u8 bus, u8 slot, 
                             u8 func_number, u8 offset) -> u16;
        auto check_vendor(u8 bus, u8 slot) -> Nullable<u16, NO_VENDOR>;
        auto check_device_id(u8 const bus,
                             u8 const slot) -> Nullable<u16, NO_DEVICE>;
        auto check_header_type(u8 const bus,
                               u8 const slot) -> HeaderType;
      private:
    };
}; // namespace pci
