#pragma once
#include "klib/int.hh"
#include "klib/option.hh"
#include "klib/array.hh"
#include "klib/slice.hh"
#include "klib/result.hh"
#include "klib/pci/pci-ide.hh"
#include "klib/pci/pci.hh"

namespace wlib {
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

        enum class IOError : i8 {
            TryAgain = -1,
            DeviceError = -2,
            BufferTooSmall = -3,
            CacheFull = -4,
        };

        // Made with help from Chickadee OS source (https://github.com/CS161/chickadee/)
        class AHCIState {
          private:
            // See page 23 of Serial ATA AHCI 1.3.1 specification for details (31 on PDF)
            struct port_registers {
                u32 cmdlist_addr;        // PxCLB -- Port x Command List Base Address
                u32 reserved;            // IMPORTANT: this assumes we are using 32-bit. if porting to 64-bit, change
                u32 rfis_base_addr;      // PxRFIS -- Port x RFIS Base Address -- the base address of rfis_state
                u32 reserved1;           // IMPORTANT: see above
                u32 interrupt_status;    // PxIS 
                u32 interrupt_enable;    // PxIE
                u32 command_and_status;  // PxCMD -- Port x Command and Status
                u32 reserved2;           // 0x2C - 0x2F are reserved
                u32 tfd;                 // PxTFD -- Port x Task File Data
                u32 sig;                 // PxSIG -- Port x Signature
                u32 sstatus;             // PxSSTS -- Port x SATA Status, 0 = no device detected
                u32 scontrol;            // PxSCTL -- Port x SATA Control
                u32 serror;              // PxSERR -- Port x SATA Error
                u32 ncq_active;          // PxSACT -- Port x SATA Active
                u32 command_mask;        // PxCI -- Port x Command Issue
                u32 sata_notification;   // PxSNTF -- Port x SATA Notification
                u32 fis_switch_control;  // PxFBS -- Port x FIS-based switching control
                u32 device_sleep;        // PxDEVSLP -- Port x Device Sleep
                u32 vendor_specific[14]; // PxVS -- Port x Vendor Specific (ignore)
            };

            struct registers {
                u32 capabilities;        // CAP: HBA capabilities [R]
                u32 global_hba_control;  // GHC: global HBA control [R/W]
                u32 interrupt_status;    // IS: interrupt status
                u32 port_mask;           // PI: addressable ports
                u32 ahci_version;        // VS: AHCI version
                u32 ccc_control;         // CCC_CTL: Command Completion Coalescing Control
                u32 ccc_port_mask;       // CCC_PORTS
                u32 em_loc;              // EM_LOC: Enclosure Management Location
                u32 em_control;          // EM_CTL: Enclosure Management Control
                u32 cap2;                // CAP2: HBA Capabilities extended
                u32 bohc;                // BOHC: BIOS/OS Handoff Control and Status
                Array<u32, 53> reserved; // Vendor specific registers
                Array<port_registers, 32> port_regs;
            };

            enum class PortCommandMasks : u32 {
                InterfaceMask   = 0xF0000000,
                InterfaceActive = 0x10000000,
                InterfaceIdle   = 0x0,
                CommandRunning  = 0x8000,
                RFISRunning     = 0x4000,
                RFISEnable      = 0x10,
                RFISClear       = 0x8,
                PowerUp         = 0x6,
                Start           = 0x1,
                
            };

            enum class RStatusMasks : u32 {
                Busy    = 0x80,
                DataReq = 0x8,
                Error   = 0x1,
            };

            enum class InterruptMasks : u32 {
                DeviceToHost   = 0x1U,
                NCQComplete    = 0x8U,
                ErrorMask      = 0x7D800010U,
                FatalErrorMask = 0x78000000U, // HBFS|HBDS|IFS|TFES
            };

            enum class GHCMasks : u32 {
                InterruptEnable = 0x2U,
                AHCIEnable      = 0x80000000U,
            };

            // DMA structures for device comm.
            // The disk drive uses these to communicate with the OS.
            
            // PRD -- this is distinct from the ATA PRD/PRDT (see pci/prdt.hh)
            struct prd {
                u32 address;
                u32 reserved_64;     // IMPORTANT: If porting to 64-bits, change address to 64-bits
                u32 reserved;
                u32 data_byte_count; // Bit 31: Interrupt on completion flag
                                     // The byte count is the number of bytes in the buffer - 1
                                     // Technically, the bits [30:22] are reserved, but we do not expect this to ever matter
            };

            struct alignas(128) command_table {
                Array<u32, 16> cfis;    // Command definitions
                Array<u32, 4> acmd;
                Array<u32, 12> reserved;
                Array<prd, 16> prdt;
            };
            
            struct command_header {
                u16 flags;
                u16 num_buffers;
                u32 buffer_byte_pos;
                u32 command_table_address;
                u32 reserved_64; // IMPORTANT: When porting, change above u32 to u64
                Array<u64, 2> reserved;
            };

            struct alignas(256) rfis_state {
                Array<u32, 64> rfis;
            };

            struct alignas(1024) dma_state {
                Array<command_header, 32> ch;
                volatile rfis_state rfis;
                Array<command_table, 32> ct;
            };

            auto static constexpr CFIS_COMMAND = 0x8027;

            auto static constexpr SECTOR_SIZE = 512_usize; // IMPORTANT: May not be true for all drives?

            enum class CHFlag {
                Clear = 0x400,
                Write = 0x40,
            };

            // Actual variable layout:
            dma_state _dma;
            u32 _bus;
            u32 _slot;
            u32 _func;
            u32 _sata_port;
            volatile registers& _drive_registers;
            volatile port_registers& _port_registers;
            
            // These should remain constant after loading
            u32 _irq;
            usize _num_sectors;
            u32 _num_ncq_slots;
            u32 _slots_full_mask;

            // This is modifiable
            u16 _num_slots_available;
            u16 _slots_outstanding_mask;
            Array<volatile u32*, 32> _slot_status; // IMPORTANT: This should become atomic once multicore is set up


            void clear_slot(u16 slot);
            void push_buffer(u32 slot, void* data, usize sz);
            void issue_meta(u32 slot, pci::IDEController::Command command, 
                            u32 features, u32 count = u32(-1));

            void issue_ncq(u32 slot, pci::IDEController::Command command,
                           usize sector, bool fua = false, u32 priority = 0);

            void acknowledge(u32 slot, u32 status);

            void await_basic(u32 slot);    

            auto static inline sstatus_active(u32 sstatus) -> bool {
                return (sstatus & 0x03) == 3
                    || ((1U << ((sstatus & 0xF00) >> 8)) & 0x144) != 0;
            }

            auto read_or_write(pci::IDEController::Command command,
                               Slice<u8>& buf, usize offset) -> Result<Null, IOError>;

            auto read_or_write_cache(pci::IDEController::Command const command,
                                     usize const offset) -> Result<Null, IOError>;
            
          public:
            AHCIState(u8 bus, u8 slot, u8 func_number, u32 sata_port, volatile registers& dr);
            AHCIState(AHCIState const&) = delete;

            inline auto irq() -> u32 { return _irq; }

            void handle_interrupt();
            void handle_error_interrupt();

            auto inline num_sectors() -> usize { return _num_sectors; }

            inline void enable_interrupts() {
                _drive_registers.global_hba_control 
                    = _drive_registers.global_hba_control | u32(GHCMasks::InterruptEnable);
            }

            inline void disable_interrupts() {
                _drive_registers.global_hba_control 
                    = _drive_registers.global_hba_control & (~u32(GHCMasks::InterruptEnable));
            }
            
            [[nodiscard]] auto static find(pci::PCIState::bus_slot_addr = {}, 
                                           u32 sata_port = 0) -> Option<AHCIState&>;

            [[nodiscard]] auto read_cache(usize offset) -> Result<Null, IOError> {
                return read_or_write_cache(pci::IDEController::Command::ReadFPDMAQueued,
                                           offset);
            }

            [[nodiscard]] auto write_cache(usize offset) -> Result<Null, IOError> {
                return read_or_write_cache(pci::IDEController::Command::ReadFPDMAQueued,
                                           offset);
            }

            [[nodiscard]] inline auto read(Slice<u8>& buf, usize offset) -> Result<Null, IOError> {
                return read_or_write(pci::IDEController::Command::ReadFPDMAQueued, 
                                     buf, offset);
            }

            [[nodiscard]] inline auto write(Slice<u8> const& buf, usize offset) -> Result<Null, IOError> {
                // const_cast is OK here since we won't be writing to this buffer
                // when we use the write command
                return read_or_write(pci::IDEController::Command::WriteFPDMAQueued, 
                                     const_cast<Slice<u8>&>(buf), offset);
            }
        };
    }; // namespace ahci
}; // namespace wlib

extern wlib::Option<wlib::ahci::AHCIState&> sata_disk0;
