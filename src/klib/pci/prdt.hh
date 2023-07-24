#pragma once
#include "klib/int.hh"
#include "klib/result.hh"
#include "klib/slice.hh"
#include "klib/pair.hh"

namespace wlib {
    namespace pci {
        class IDEController;

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
            PRDT(PRDT const&) = delete;
            constexpr PRDT() {}
        
            enum class DMAOpMask : u8 {
                Read  = 0b11111111,
                Write = 0b11110111,
            };

            enum class ChannelType : u8 {
                Primary = 0x0,
                Secondary = 0x8,
            };

            enum class StatusBits : u8 {
                DriveGeneratedIRQ = 0b100,
                DMAFailed         = 0b010,
                InDMAMode         = 0b001,
            };

            auto initialize(u16 entry_count, u16 bus_master_register, ChannelType channel) -> Result<Null, Null>;
        
            // Set up DMA transfer by loading data into the PRDT, setting the bus master register's 
            // "start" bit, and setting a R/W operation.
            // This will stop and restart any DMA operations running on this drive channel,
            // so be careful to make sure a DMA is not in progress. 
            // 
            // This will not fully start DMA. The IDE controller must be issued a Read/Write DMA
            // command (see pci-ide.cc/hh). 
            auto set_up_dma(Slice<Pair<uptr, usize>> const& entries, 
                           ChannelType channel,
                           DMAOpMask operation) -> Result<Null, u16>;
            auto status();

          private:
            // Offsets into the bus master register (relative)
            enum class BMROffset : u8 {
                Command       = 0x0,
                Status        = 0x2,
                PRDTAddress   = 0x4,
            };

            enum class StartMask : u8 {
                Stop  = 0b11111110,
                Start = 0b11111111,
            };

            friend class Result<PRDT, Null>;
            friend class IDEController;
            friend class Array<PRDT, 4>;

            void command_start(StartMask bit, ChannelType channel);
            void set_read_write(DMAOpMask operation);
            

            
            PRD_Entry* prdt_location;
            u16 entry_count;
            u16 bus_master_register;
        };
    }; // namespace pci
};
