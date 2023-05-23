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

    enum class Register : u8 {
        VendorId       = 0x0,
        DeviceId       = 0x2,
        Command        = 0x4,
        Status         = 0x6,
        RevisionId     = 0x8,
        ProgIF         = 0x9,
        Subclass       = 0xA,
        ClassCode      = 0xB,
        CacheLineSize  = 0xC,
        LatencyTimer   = 0xD,
        HeaderType     = 0xE,
        BIST           = 0xF,
        GDBaseAddress0 = 0x10,
        GDBaseAddress1 = 0x14,
        GDBaseAddress2 = 0x18,
        GDBaseAddress3 = 0x1C,
        GDBaseAddress4 = 0x20,
        GDBaseAddress5 = 0x24,
    };

    enum class HeaderType : u8 {
        GeneralDevice = 0x0,
        PciToPci      = 0x1,
        PciToCardBus  = 0x2,
        MultiFunction,
        Unknown,
    };

    struct command_register {
      public:
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

        auto get_bit(bit b) -> bool {
            auto b_u8 = static_cast<u8>(b);
            return bytes & (1 << b_u8);
        }

        u16 bytes;
    };

    struct status_register {
      public:
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

    struct memory_space_bar {
      public:
        auto get_address() -> u32 {
            return bytes & (~0xF);
        }

        auto prefetchable() -> bool {
            return bytes & (1 << 3);
        }

        auto type() -> u8 {
            u32 constexpr mask = 0b11 << 1;
            return (bytes & mask) >> 1;
        }

        u32 bytes;
    };

    struct io_space_bar {
      public:
        auto get_address() -> u32 {
            return bytes & (~0b11);
        }

        u32 bytes;
    };

    // State representing one PCI bus
    class PCIState {
      public:
        auto config_read_word(u8 bus, u8 slot,
                              u8 func_number, Register offset) -> u16;
        auto config_read_byte(u8 const bus, u8 const slot,
                              u8 const func_number, Register const offset) -> u8;
        auto check_vendor(u8 bus, u8 slot) -> Nullable<u16, NO_VENDOR>;
        auto check_device_id(u8 const bus,
                             u8 const slot) -> Nullable<u16, NO_DEVICE>;
        auto check_header_type(u8 const bus,
                               u8 const slot) -> HeaderType;
        auto get_status(u8 const bus, u8 const slot) -> Option<status_register>;
        void set_status(u8 const bus, u8 const slot, status_register status);
      private:
        // TODO(?): store header types and no longer assume Header Type 0 
        // (normal devices; not bridges)
    };
}; // namespace pci
