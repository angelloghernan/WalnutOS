#pragma once
#include "klib/int.hh"
#include "klib/array.hh"
#include "klib/console.hh"
#include "klib/nullable.hh"
#include "klib/result.hh"


namespace wlib {
    auto static constexpr PAGESIZE = 4096;
    auto static constexpr PTE_P   = 0b001;
    auto static constexpr PTE_W   = 0b010;
    auto static constexpr PTE_U   = 0b100;
    auto static constexpr PTE_PWU = PTE_P | PTE_W | PTE_U;
    auto static constexpr PTE_PW  = PTE_P | PTE_W;
    auto static constexpr PTE_PU  = PTE_P | PTE_U;

    namespace pagetables {
        void enable_paging();

        class PageTableEntry {
          public:
            PageTableEntry() : _internal(0) {}
            PageTableEntry(PageTableEntry const& pte) = delete;

            /// Return whether this page table entry is [P]resent.
            [[nodiscard]] auto constexpr present()       const -> bool { return _internal & 0b1; }
            /// Return whether this page table entry is [W]ritable.
            [[nodiscard]] auto constexpr writable()      const -> bool { return _internal & 0b10; }
            /// Return whether this page table entry is [U]ser-accessible.
            [[nodiscard]] auto constexpr user()          const -> bool { return _internal & 0b100; }
            /// Return whether this page table entry uses write-through caching.
            [[nodiscard]] auto constexpr write_through() const -> bool { return _internal & 0b1000; }
            /// Return whether this page table entry has disabled caching.
            [[nodiscard]] auto constexpr cache_disable() const -> bool { return _internal & 0b10000; }
            /// Return whether this page table entry was read during virtual address translation.
            [[nodiscard]] auto constexpr accessed()      const -> bool { return _internal & 0b100000; }
            /// Return whether this page was written.
            [[nodiscard]] auto constexpr dirty()         const -> bool { return _internal & 0b1000000; }
            /// Return whether Page Attribute Table (PAT) is supported.
            [[nodiscard]] auto constexpr using_pat()     const -> bool { return _internal & 0b10000000; }
            /// Return whether or not to invalidate the TLB entry for this page when CR3 is changed.
            [[nodiscard]] auto constexpr global()        const -> bool { return _internal & 0b100000000; }

            /// Return the address of the page pointed by this PT Entry.
            [[nodiscard]] auto constexpr page_address() const -> uptr { 
                return _internal & 0xFFFFFF00;
            }

            /// Make this pagetable entry map to physical address [addr].
            auto map(uptr addr, u8 perm) -> Result<Null, Null> {
                _internal = addr;
                _internal |= perm;
                return Result<Null, Null>::Ok({});
            }
            
            [[nodiscard]] auto try_map(uptr addr, u8 perm) -> Result<Null, Null> {
                return map(addr, perm);
            }


          private:
            u32 _internal;
        }__attribute__((packed));

        class alignas(PAGESIZE) PageTable {
          public:
            PageTable(PageTable const& pt) = delete;
            PageTable() {};
            static usize constexpr const NUM_ENTRIES = 1024;

            constexpr PageTableEntry const& operator[](usize idx) const { 
                return _entries[idx]; 
            }
            constexpr PageTableEntry& operator[](usize idx) {
                return _entries[idx]; 
            }

            [[nodiscard]] auto constexpr pt_idx(uptr address) const -> usize {
                // Indexed by bits 12-21 of the address
                return (address & 0x3FF000) >> 12;
            }

            [[nodiscard]] auto va_to_pa(uptr address) const -> uptr {
                // Page table indexed by bits 12-21 of the address
                auto const entry_idx = pt_idx(address);
                auto const& pte = _entries[entry_idx];
                return pte.page_address();
            }

          private:
            Array<PageTableEntry, NUM_ENTRIES> _entries;
        };


        class PageDirectoryEntry {
          public:
            PageDirectoryEntry() : _internal(0) {}
            PageDirectoryEntry(PageDirectoryEntry const& pde) = delete;

            /// Return whether this directory is [P]resent.
            [[nodiscard]] auto constexpr present()       const -> bool  { return _internal & 0b1; }
            /// Return whether this directory is [W]ritable.
            [[nodiscard]] auto constexpr writable()      const -> bool  { return _internal & 0b10; }
            /// Return whether this directory is [U]ser-accessible.
            [[nodiscard]] auto constexpr user()          const -> bool  { return _internal & 0b100; }
            /// Return whether this directory uses write-through caching.
            [[nodiscard]] auto constexpr write_through() const -> bool  { return _internal & 0b1000; }
            /// Return whether this directory has disabled caching.
            [[nodiscard]] auto constexpr cache_disable() const -> bool  { return _internal & 0b10000; }
            /// Return whether this directory was read during virtual address translation.
            [[nodiscard]] auto constexpr accessed()      const -> bool  { return _internal & 0b100000; }

            /// Return the address of the pagetable pointed by this PD Entry.
            [[nodiscard]] auto constexpr pt_address() const -> uptr { 
                return _internal & 0xFFFFFF00;
            }
            
            // Obtain a reference to the pagetable associated with this directory entry, if possible.
            [[nodiscard]] auto get_pt() const -> Option<PageTable&> {
                auto const pt_addr = reinterpret_cast<PageTable*>(pt_address());
                if (!pt_addr) {
                    return Option<PageTable&>::None();
                } else {
                    auto& pt_ref = *pt_addr;
                    return Option<PageTable&>::Some(pt_ref);
                }
            }

            // Map [virtual_addr] to [physical_addr] with given [perm]issions.
            // Returns -1 on failure. Else, returns 0.
            auto map(uptr const virtual_addr, uptr const physical_addr, u8 const perm) -> Result<Null, Null> {
                auto& pagetable = get_pt().unwrap();
                auto pt_idx = pagetable.pt_idx(virtual_addr);

                auto& pt_entry = pagetable[pt_idx];
                return pt_entry.map(physical_addr, perm);
            }
            
            // Try to map [virtual_addr] to [physical_addr] with given [perm]issions.
            // If this operation fails, returns -1. Otherwise, returns 0.
            [[nodiscard]] auto try_map(uptr const virtual_addr,
                                       uptr const physical_addr, u8 const perm) -> Result<Null, Null> {
                auto maybe_pagetable = get_pt();
                if (maybe_pagetable.none()) {
                    return Result<Null, Null>::Err({});
                }

                auto& pagetable = maybe_pagetable.unwrap();

                auto pt_idx = pagetable.pt_idx(virtual_addr);
                auto& pt_entry = pagetable[pt_idx];

                return pt_entry.try_map(physical_addr, perm);
            }


            [[nodiscard]] auto add_pt(uptr ptable_addr, u8 perm) -> Result<Null, Null>;

          private:
            u32 _internal;
        } __attribute__((packed));

        class alignas(PAGESIZE) PageDirectory {
          public:
            PageDirectory(PageDirectory const& pd) = delete;
            PageDirectory() {}
            static auto constexpr NUM_ENTRIES = 1024_usize;

            auto get_entry(usize const idx)       -> PageDirectoryEntry& { return _entries[idx]; }
            auto get_entry(usize const idx) const -> PageDirectoryEntry const& { return _entries[idx]; }

            auto add_pagetable(usize const idx, PageTable const&, u8 perm) -> Result<Null, Null>;

            auto map(uptr const virtual_addr, uptr const physical_addr, u8 perm) -> Result<Null, Null>;

            [[nodiscard]] auto try_map(uptr const virtual_addr, 
                                       uptr const physical_addr, u8 const perm) -> Result<Null, Null>;

            /// Set this page directory to be the page directory in force using %cr3.
            void set_page_directory() const;

            auto va_to_pa(uptr const address) const -> Nullable<uptr, uptr(-1)>;

            [[nodiscard]] auto constexpr va_to_idx(uptr addr) const -> usize {
                // Indexed by top 10 bits (2^10 = 1024)
                return (addr & 0xFFC00000) >> 22;
            }
            
            PageDirectoryEntry& operator[](usize const idx) { return _entries[idx]; }
            PageDirectoryEntry const& operator[](usize const idx) const { return _entries[idx]; }

          private:
            Array<PageDirectoryEntry, NUM_ENTRIES> _entries;
        };
    };
};
