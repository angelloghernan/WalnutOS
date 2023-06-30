#pragma once
#include "../int.hh"
#include "../result.hh"
#include "../slice.hh"
#include "../pair.hh"

namespace pci {
    class PRD_Entry {
      public:
        void set_address(u32 const address) {
            buffer_address = address;
        }

        void set_size(u32 const size) {
            buffer_size = size;
        }

        void set_last_entry_flag() {
            last_entry |= (0b1 << 15);
        }

        void clear_last_entry_flag() {
            last_entry &= (~(0b1 << 15));
        }

      private:
        u32 buffer_address; // address of the buffer
        u16 buffer_size;    // size of the buffer in *bytes*
        u16 last_entry = 0; // reserved, except for MSB indicating this is the last entry
    };

    // Class representing the PRDT (Physical Region Descriptor Table), for use with ATA
    // 
    // This allows us to use DMA to read from disk asynchronously, as opposed to PIO 
    // which blocks the CPU.
    //
    // The PRDT must be 32-bit (4-byte) aligned and cannot cross a 64k-boundary.
    class alignas(8) PRDT {
      public:
        auto static create(usize entry_count) -> Result<PRDT, Null>;
        auto add_entries(Slice<Pair<uptr, usize>> const& entries) -> Result<Null, u16>;
      private:
        friend class Result<PRDT, Null>;
        auto add_entry(Pair<uptr, usize> const& entry) -> Result<Null, Null>;
        PRDT() {}
        PRD_Entry* prdt_location;
        u32 count;
    };
}; // namespace pci
