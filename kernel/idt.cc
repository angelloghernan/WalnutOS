#include "../klib/idt.hh"
#include "../klib/console.hh"
#include "../klib/assert.hh"

Idt idt;
static Idtr idtr;
extern void* isr_stub_table[];

extern "C" [[noreturn]] void exception_handler(regstate* regs) {
    __asm__ volatile("cli");
    terminal.print("Unhandled Exception!");
    __asm__ volatile("hlt");
    while (true) {}
}


void Idt::init() {
    idtr.set_base(reinterpret_cast<usize>(&_idt[0]));
    idtr.set_limit(sizeof(IdtEntry) * MAX_NUM_DESCRIPTORS - 1);
    for (usize i = 0; i < Idt::MAX_NUM_DESCRIPTORS; ++i) {
        _idt[i].set(isr_stub_table[i], 0x8E);
    }

    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}
