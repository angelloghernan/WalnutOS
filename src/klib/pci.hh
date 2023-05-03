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

    struct command_register {
        enum class bit : u8 {
            InterruptDisable        = 10,
            FastBackToBackEnable    = 9,
            SERRNoEnable            = 8,
            ParityErrorResponse     = 6,
            VGAPaletteSnoop         = 5,
            MemoryWriteInvalidateEn = 4,
            SpecialCycles           = 3,
            BusMaster               = 2,
            MemorySpace             = 1,
            IOSpace                 = 0,
        };

        void set_bit(bit b) {
            auto b_u8 = static_cast<u8>(b);
            bytes |= 1 << b_u8;
        }

        auto get_bit(bit b) {
            auto b_u8 = static_cast<u8>(b);
            return bytes & (1 << b_u8);
        }

        u16 bytes;
    };

    struct status_register {
        enum class bit : u8 {
            DetectedParityError   = 15,
            SignaledSystemError   = 14,
            ReceivedMasterAbort   = 13,
            ReceivedTargetAbort   = 12,
            SignaledTargetAbort   = 11,
            MasterDataParityError = 8,
            FastBackToBackCapable = 7,
            Mhz66Capable          = 5,
            CapabilitiesList      = 4,
            InterruptStatus       = 3,
        };

        void set_bit(bit const b) {
            auto const b_u8 = static_cast<u8>(b);
            bytes |= 1 << b_u8;
        }

        auto get_bit(bit const b) -> bool {
            auto const b_u8 = static_cast<u8>(b);
            return bytes & (1 << b_u8);
        }

        auto get_devsel_timing() -> u8 {
            u16 constexpr mask = 0b11 << 9;
            return (bytes & mask) >> 9;
        }
        
        u16 bytes;  
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
