#pragma once
#include "int.hh"
#include "array.hh"

static auto constexpr const PAGESIZE = 4096;
namespace pagetables {
    class PageTableEntry {
      public:
        PageTableEntry() : _flags(0), _avl_addr_lower(0), _addr_higher(0) {}

        /// Return whether this page table entry is [P]resent.
        [[nodiscard]] auto constexpr present()       const -> bool { return _flags & 0b1; }
        /// Return whether this page table entry is [W]ritable.
        [[nodiscard]] auto constexpr writable()      const -> bool { return _flags & 0b10; }
        /// Return whether this page table entry is [U]ser-accessible.
        [[nodiscard]] auto constexpr user()          const -> bool { return _flags & 0b100; }
        /// Return whether this page table entry uses write-through caching.
        [[nodiscard]] auto constexpr write_through() const -> bool { return _flags & 0b1000; }
        /// Return whether this page table entry has disabled caching.
        [[nodiscard]] auto constexpr cache_disable() const -> bool { return _flags & 0b10000; }
        /// Return whether this page table entry was read during virtual address translation.
        [[nodiscard]] auto constexpr accessed()      const -> bool { return _flags & 0b100000; }
        /// Return whether this page was written.
        [[nodiscard]] auto constexpr dirty()         const -> bool { return _flags & 0b1000000; }
        /// Return whether Page Attribute Table (PAT) is supported.
        [[nodiscard]] auto constexpr using_pat()     const -> bool { return _flags & 0b10000000; }
        /// Return whether or not to invalidate the TLB entry for this page when CR3 is changed.
        [[nodiscard]] auto constexpr global()        const -> bool { return _flags & 0b100000000; }

        /// Return the address of the page pointed by this PT Entry.
        [[nodiscard]] auto constexpr page_address() const -> uptr { 
            return (usize(_addr_higher) << 16) | (_avl_addr_lower & 0xF0);
        }

      private:
        u8 _flags;
        u8 _avl_addr_lower;
        u16 _addr_higher;
    }__attribute__((packed));

    class alignas(PAGESIZE) PageTable {
      public:
        static usize constexpr const NUM_ENTRIES = 1024;

        PageTableEntry const& operator[](usize idx) const { return _entries[idx]; }
        PageTableEntry& operator[](usize idx) { return _entries[idx]; }

        auto va_to_pa(uptr address) const -> uptr {
            // Page table indexed by bits 12-21 of the address
            usize const entry_idx = address & 0x3FF000;
            auto const& pte = _entries[entry_idx];
            return pte.page_address();
        }

      private:
        Array<PageTableEntry, NUM_ENTRIES> _entries;
    };


    class PageDirectoryEntry {
      public:
        PageDirectoryEntry() : _flags(0), _avl_addr_lower(0), _addr_higher(0) {}

        /// Return whether this directory is [P]resent.
        [[nodiscard]] auto constexpr present()       const -> bool  { return _flags & 0b1; }
        /// Return whether this directory is [W]ritable.
        [[nodiscard]] auto constexpr writable()      const -> bool  { return _flags & 0b10; }
        /// Return whether this directory is [U]ser-accessible.
        [[nodiscard]] auto constexpr user()          const -> bool  { return _flags & 0b100; }
        /// Return whether this directory uses write-through caching.
        [[nodiscard]] auto constexpr write_through() const -> bool  { return _flags & 0b1000; }
        /// Return whether this directory has disabled caching.
        [[nodiscard]] auto constexpr cache_disable() const -> bool  { return _flags & 0b10000; }
        /// Return whether this directory was read during virtual address translation.
        [[nodiscard]] auto constexpr accessed()      const -> bool  { return _flags & 0b100000; }

        /// Return the address of the pagetable pointed by this PD Entry.
        [[nodiscard]] auto constexpr pt_address() const -> uptr { 
            return (usize(_addr_higher) << 16) | (_avl_addr_lower & 0xF0);
        }

        [[nodiscard]] auto get_pt() const -> PageTable& {
            auto const pt_addr = reinterpret_cast<PageTable*>(pt_address());
            return *pt_addr;
        }

      private:
        u8 _flags;
        u8 _avl_addr_lower;
        u16 _addr_higher;
    } __attribute__((packed));

    class alignas(PAGESIZE) PageDirectory {
      public:
        static usize constexpr const NUM_ENTRIES = 1024;

        auto get_entry(usize idx)       -> PageDirectoryEntry& { return _entries[idx]; }
        auto get_entry(usize idx) const -> PageDirectoryEntry const& { return _entries[idx]; }

        auto map(uptr virtual_addr, uptr physical_addr) -> i8;

        void set_page_directory() const;

        auto va_to_pa(uptr address) const -> uptr {
            // PD entry indexed by top 10 bits in address
            usize const pd_idx = address & 0xFFC00000;
            auto const& pd = _entries[pd_idx];
            auto const& pt = pd.get_pt();
            return pt.va_to_pa(address);
        }

        PageDirectoryEntry& operator[](usize idx) { return _entries[idx]; }
        PageDirectoryEntry const& operator[](usize idx) const { return _entries[idx]; }  

      private:
        Array<PageDirectoryEntry, NUM_ENTRIES> _entries;
    };
};

