#pragma once
#include "int.hh"

namespace ahci {
    // The FIS type, or the type of packet transporting data between
    // the device and the host (our OS/the CPU)
    enum class FISType : u8 {
        REG_HOST_TO_DEVICE = 0x27,
        REG_DEVICE_TO_HOST = 0x34,
        DMA_ACTIVATE       = 0x39, // activate DMA transfer
        DMA_SETUP          = 0x41, // bidirectional
        DATA               = 0x46, // bidirectional
        BIST               = 0x58, // Built In Self Test
        PIO_SETUP          = 0x5F, // setting up PIO (slower than DMA)
        SET_DEVICE_BITS    = 0xA1, // 
    };
}; // namespace ahci
