#pragma once
#include "../klib/x86.hh"
#include "../klib/int.hh"
#include "../klib/option.hh"
#include "../klib/array.hh"
namespace wlib {
    namespace apic {
        class LocalApic {
          public:
            LocalApic(LocalApic const& a) = delete; 
            LocalApic() = delete;
            static auto get() -> LocalApic&;
            static auto get_pa() -> uptr;
            void enable();
            // Set the physical address of the APIC. 
            // Do not call while a LocalApic instance is active.
            static void set_pa(uptr addr);
            auto get_id() const -> u8;
            // Send an end-of-interrupt notification to APIC controller.
            void end_of_interrupt();
          private:
            // An APIC register. Each one has a 32-bit readable value.
            // The rest of the register (all 128 - 32 bits) is unusable.
            struct alignas(16) apic_register {
                volatile u32 value;
            };

            Array<volatile apic_register, 0x40> _registers;

            auto read_register(usize offset) const -> u32;
            void write_register(usize offset, u32 value);

            static auto can_use_apic() -> bool;

            // Model-specific register number for the APIC base register
            static u32 constexpr IA32_APIC_BASE_MSR        = 0x1B;
            // Bitmask for enabling the APIC in the model-specific register
            static u32 constexpr IA32_APIC_BASE_MSR_ENABLE = 0x800;
            // Bitmask for edx register from cpuid. Checks whether we have APIC feature
            static u32 constexpr CPUID_EDX_HAS_APIC        = 0b1000000000;
            // Bitmask for telling if this is the bootstrap processor (BSP)
            static u32 constexpr IA32_APIC_BASE_MSR_BSP    = 0x100;
            // Default physical address of the local apic's registers
            static uptr constexpr PA_DEFAULT               = 0xFEE00000; 
            // Register offset for LAPIC ID
            static u32 constexpr REG_LAPIC_ID              = 0x2;
            // Register offset for LVT timer
            static u32 constexpr REG_LVT_TIMER             = 0x32;
            // Register offset for End-Of-Interrupt
            static u32 constexpr REG_EOI                   = 0xB;
            // Register offset for Spurious-Interrupt Vector register
            static u32 constexpr REG_SPURIOUS_INTERRUPT    = 0xF;
            // Bitmask for enabling interrupts using the SVR register
            static u32 constexpr APIC_ENABLE               = 0x100;
            
        };

        class IOApic {
          public:
          private:
        };

    }
}; // namespace wlib
