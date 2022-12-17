#pragma once
#include "int.hh"
#include "array.hh"

class alignas(8) IdtEntry {
  public:
    IdtEntry() : _isr_low(0), _kernel_cs(0), _reserved(0), _attributes(0), _isr_high(0) {}
    void set(void* handler, u8 flags) {
        auto handler_address = reinterpret_cast<uptr>(handler);
        _isr_low = handler_address & 0xFFFF;
        // Kernel code selector is 0x08 offset (first after null)
        _kernel_cs = 0x08;
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
    static usize const MAX_NUM_DESCRIPTORS = 256;
    static usize const NUM_RESERVED        = 32;

    void idt_set_descriptor(u8 vector, void* handler, u8 flags) {
        _idt[vector].set(handler, flags);
    }

    void init();
    Array<IdtEntry, 256> _idt;

    
  private:
};

extern Idt idt;

struct regstate {
public:
    usize reg_eax;
    usize reg_edx;
    usize reg_ebx;
    usize reg_ecx;
    usize vector_code;
    usize error_code;
    usize reg_eip;
    usize reg_cs;
    usize reg_eflags;
private:
} __attribute__((packed));
