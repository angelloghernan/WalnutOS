#include "../klib/pagetables.hh"
#include "../klib/int.hh"
#include "../klib/console.hh"

namespace pagetables {
    auto PageDirectory::map(uptr const virtual_addr, uptr const physical_addr, u8 const perm) -> i8 {
        auto const pd_idx = va_to_idx(virtual_addr);
        auto& pagedir = _entries[pd_idx];
        return pagedir.map(virtual_addr, physical_addr, perm);
    }

    auto PageDirectory::try_map(uptr const virtual_addr, uptr const physical_addr, u8 const perm) -> i8 {
        auto const pd_idx = va_to_idx(virtual_addr);

        auto& pagedir = _entries[pd_idx];
        if (pagedir.pt_address() == 0) {
            return -1;
        }

        return pagedir.try_map(virtual_addr, physical_addr, perm);
    }


    auto PageDirectory::add_pagetable(usize const idx, PageTable const& ptable, u8 const perm) -> i8 {
        auto const ptable_addr = reinterpret_cast<uptr>(&ptable);
        return _entries[idx].add_pt(ptable_addr, perm);
    }

    /// Enable the paging [PG] bit in cr0.
    void enable_paging() {
        u32 cr0;
        asm volatile("mov %%cr0, %0" : "=r"(cr0));
        cr0 |= 0x80000000;
        asm volatile("mov %0, %%cr0" : : "r"(cr0));
    }

    /// Set this page directory as the new page directory.
    void PageDirectory::set_page_directory() const {
        asm volatile("mov %0, %%cr3" : : "r"(this));
    }

    
    auto PageDirectory::va_to_pa(uptr const address) const -> uptr {
        usize const pd_idx = va_to_idx(address);
        auto const pt = _entries[pd_idx].get_pt();
        match(pt) {
            case Some:
                return pt.unwrap().va_to_pa(address);
            case None:
                return uptr(-1);
        }
    }

    auto PageDirectoryEntry::add_pt(uptr const ptable_addr, u8 const perm) -> i8 {
        if (pt_address() != 0) { [[unlikely]]  
            /// TODO: Use dynamic allocation to try and add a pagetable to our directory.
            return -1;
        } 
        _internal = ptable_addr | PTE_P | PTE_W;
        return 0;
    }

};
