#pragma once
#include "klib/int.hh"
#include "klib/array.hh"
#include "klib/ps2/keyboard.hh"

namespace wlib {
    namespace interrupts {
        void sleep(usize miliseconds);
    };

    class alignas(8) IdtEntry {
      public:
        IdtEntry() : _isr_low(0), _kernel_cs(0), _reserved(0), _attributes(0), _isr_high(0) {}
        void set(void* handler, u8 flags, u16 code_segment) {
            auto handler_address = reinterpret_cast<uptr>(handler);
            _isr_low = handler_address & 0xFFFF;
            // Kernel code selector is 0x08 offset (first after null)
            _kernel_cs = code_segment;
            _reserved = 0;
            _attributes = flags;
            _isr_high = (handler_address >> 16) & 0xFFFF;
        }

      private:
        u16 _isr_low = 0;
        u16 _kernel_cs = 0;
        u8 _reserved = 0;
        u8 _attributes = 0;
        u16 _isr_high = 0;

    } __attribute__((packed));

    class Idtr {
    public:
        constexpr void set_base(usize base) {
            _base = base;
        }

        constexpr void set_limit(u16 limit) {
            _limit = limit;
        }

    private:
        u16 _limit = 0;
        u32 _base = 0;
    } __attribute__((packed));

    class alignas(16) Idt {
      public:
        Idt() {}
        Idt(Idt const&) = delete;
        Idt& operator=(Idt const&) = delete;
        static usize const MAX_NUM_DESCRIPTORS = 256;
        static usize const NUM_RESERVED        = 32;

        inline void idt_set_descriptor(u8 vector, void* handler, u8 flags, u16 code_segment) {
            _idt[vector].set(handler, flags, code_segment);
        }

        inline static void enable_interrupts() {
            __asm__ volatile ("sti");
        }

        inline static void disable_interrupts() {
            __asm__ volatile ("cli");
        }

        void init();
        Array<IdtEntry, 256> _idt;

        
      private:
    };

    /// InterruptGuard: Disables interrupts when constructed and then 
    /// enables interrupts at the end of its scope.
    class InterruptGuard {
        InterruptGuard(InterruptGuard const&) = delete;
        InterruptGuard& operator=(InterruptGuard const&) = delete;

        public:
            InterruptGuard() {
                Idt::disable_interrupts();
            }
            ~InterruptGuard() {
                Idt::enable_interrupts();
            }
        private:
    };


    struct regstate {
    public:
        usize reg_edi;
        usize reg_esi;
        usize reg_ebp;
        usize reg_esp;
        usize reg_ebx;
        usize reg_edx;
        usize reg_ecx;
        usize reg_eax;
        usize vector_code;
        usize error_code;
        usize reg_eip;
        usize reg_cs;
        usize reg_eflags;
    private:
    } __attribute__((packed));


}; // namespace wlib

extern "C" void keyboard_handler();
extern usize timer;
extern wlib::Idt idt;
