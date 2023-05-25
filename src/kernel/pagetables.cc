#include "../klib/pagetables.hh"
#include "../klib/int.hh"
#include "../klib/console.hh"
#include "../klib/nullable.hh"
#include "../kernel/kernel.hh"
#include "../klib/result.hh"

namespace pagetables {
    auto PageDirectory::map(uptr const virtual_addr, 
                            uptr const physical_addr, u8 const perm) -> Result<Null, Null> {
        auto const pd_idx = va_to_idx(virtual_addr);
        auto& pagedir = _entries[pd_idx];
        return pagedir.map(virtual_addr, physical_addr, perm);
    }

    auto PageDirectory::try_map(uptr const virtual_addr, 
                                uptr const physical_addr, u8 const perm) -> Result<Null, Null> {
        auto const pd_idx = va_to_idx(virtual_addr);

        auto& pagedir = _entries[pd_idx];
        if (pagedir.pt_address() == 0) {
            auto new_pt = simple_allocator.kalloc(PAGESIZE);

            if (new_pt.none()) {
                return Result<Null, Null>::Err({});
            }

            if (pagedir.add_pt(new_pt.unwrap_as<uptr>(), perm).is_err()) {
                simple_allocator.kfree(new_pt.unwrap());
                return Result<Null, Null>::Err({});
            }
            
            // Need to zero out pages, or else weird bugs
            // start to occur 
            auto ptr = new_pt.unwrap_as<int*>();
            
            for (u16 i = 0; i < PAGESIZE / sizeof(int); ++i) {
                // We can assume PAGSIZE is divisible by 4 reasonably
                *ptr = 0;
                ++ptr;
            }
        }

        return pagedir.try_map(virtual_addr, physical_addr, perm);
    }


    auto PageDirectory::add_pagetable(usize const idx, 
                                      PageTable const& ptable, u8 const perm) -> Result<Null, Null> {
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

    
    auto PageDirectory::va_to_pa(uptr const address) const -> Nullable<uptr, uptr(-1)> {
        usize const pd_idx = va_to_idx(address);
        auto const pt = _entries[pd_idx].get_pt();
        switch(pt.matches()) {
            case Some:
                return pt.unwrap().va_to_pa(address);
            case None:
                return Nullable<uptr, uptr(-1)>();
        }
    }

    auto PageDirectoryEntry::add_pt(uptr const ptable_addr, u8 const perm) -> Result<Null, Null> {
        if (pt_address() != 0) [[unlikely]] {
            return Result<Null, Null>::Err({});
        } 
        _internal = ptable_addr | PTE_P | PTE_W;
        return Result<Null, Null>::Ok({});
    }

};
