#pragma once
#include "../klib/x86.hh"
#include "../klib/int.hh"
namespace apic {
    static u32 const CPU_FEAT_EDX_APIC = 0b100000000;
    auto can_use_apic() -> bool;

    class Apic {
    public:
        
    private:
        static usize const IA32_APIC_BASE_MSR_BSP = 0x100;
    };
}
