#pragma once
#include "int.hh"

namespace pci {
    class IDEController {
      public:
        enum class Mode : u8 {
            Native,
            Compatibility,
        };

        enum class Status : u8 {
            Busy               = 0x80,
            ReadyDrive         = 0x40,
            DriveWriteFault    = 0x20,
            DriveSeekComplete  = 0x10,
            DataRequestReady   = 0x08,
            CorrectedData      = 0x04,
            Index              = 0x02,
            Error              = 0x01,
        };

        enum class Error : u8 {
            BadBlock           = 0x80,
            Uncorrectable      = 0x40,
            MediaChanged       = 0x20,
            IDMarkNotFound     = 0x10,
            MediaChangeRequest = 0x08,
            CommandAborted     = 0x04,
            Track0NotFound     = 0x02,
            NoAddressMark      = 0x01,
        };

        enum class Command : u8 {
            ReadPIO          = 0x20,
            ReadPIOExt       = 0x24,
            ReadDMA          = 0xC8,
            ReadDMAExt       = 0x25,
            WritePIO         = 0x30,
            WritePIOExt      = 0x34,
            WriteDMA         = 0xCA,
            WriteDMAExt      = 0x35,
            CacheFlush       = 0xE7,
            CacheFlushExt    = 0xEA,
            Packet           = 0xA0,
            IdentifyPacket   = 0xA1,
            Identify         = 0xEC,
        };

        enum class Register : u8 {
            Data        = 0x00,
            Error       = 0x01,
            Features    = 0x01,
            SecCount0   = 0x02,
            LBA0        = 0x03,
            LBA1        = 0x04,
            LBA2        = 0x05,
            HDDevSel    = 0x06,
            Command     = 0x07,
            Status      = 0x07,
            SecCount1   = 0x08,
            LBA3        = 0x09,
            LBA4        = 0x0A,
            LBA5        = 0x0B,
            Control     = 0x0C,
            AltStatus   = 0x0C,
            DevAddress  = 0x0D,
        };

        enum class ATAPICommand : u8 {
            Read = 0xA8,
            Eject = 0x1B,
        };

        enum class InterfaceType : u8 {
            ATA   = 0x0,
            ATAPI = 0x1,
        };

        enum class ControlType : u8 {
            Master = 0x0,
            Slave  = 0x1,
        };

        enum class IdentityField : u8 {
            DeviceType     = 0,
            Cylinders      = 2,
            Heads          = 6,
            Sectors        = 12,
            Serial         = 20,
            Model          = 54,
            Capabilities   = 98,
            FieldValid     = 106,
            MaxLBA         = 120,
            CommandSets    = 164,
            MaxLBAExt      = 200,
        };
        
      private:
        u8 bus;
        u8 slot;
        Mode mode;
    };
};
