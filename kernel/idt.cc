#include "../klib/idt.hh"
#include "../klib/console.hh"
#include "../klib/assert.hh"
#include "../klib/console.hh"
#include "../klib/pic.hh"

extern "C" void exception_handler(regstate* regs) {
    // terminal.print_line("Exception ", usize(regs->vector_code), " at ", 
                        //reinterpret_cast<void*>(regs->reg_eip));

    Pic::end_of_interrupt(regs->vector_code);
}

