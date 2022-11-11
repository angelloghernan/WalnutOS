#include "../klib/apic.hh"
namespace apic {
    auto can_use_apic() -> bool {
        u32 eax, edx;
        x86::cpuid(1, &eax, &edx);
        return edx & CPU_FEAT_EDX_APIC;
    }

}
