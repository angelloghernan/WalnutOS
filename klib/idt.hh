#pragma once
#include "int.hh"
#include "array.hh"

class IdtEntry {
  public:
    void set(void* handler, u8 flags) {
        auto handler_address = reinterpret_cast<uptr>(handler);
        _isr_low = handler_address & 0xFFFF;
        // Kernel code selector is 0x08
        _kernel_cs = 0x08;
        _reserved = 0;
        _attributes = flags;
        _isr_high = (handler_address >> 16) & 0xFFFF;
    }

  private:
    u16 _isr_low;
    u16 _kernel_cs;
    u8 _reserved;
    u8 _attributes;
    u16 _isr_high;

} __attribute__((packed));

class Idtr {
public:
    void set_base(usize base) {
        _base = base;
    }

    void set_limit(u16 limit) {
        _limit = limit;
    }

private:
    u16 _limit;
    usize _base;
} __attribute__((packed));

class Idt {
  public:
    static usize const MAX_NUM_DESCRIPTORS = 256;

    void idt_set_descriptor(u8 vector, void* handler, u8 flags) {
        _idt[vector].set(handler, flags);
    }

    void init();

    
  private:
    Array<IdtEntry, 256> _idt;
} __attribute__((aligned(0x10)));

extern Idt idt;
