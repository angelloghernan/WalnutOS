#pragma once
#include "int.hh"
#include "array.hh"

class IdtEntry {
  public:
    

  private:
    u16 isr_low;
    u16 kernel_cs;
    u8 reserved;
    u8 attributes;
    u16 isr_high;

} __attribute__((packed));

__attribute__((aligned(0x10)))
extern Array<IdtEntry, 256> idt;
