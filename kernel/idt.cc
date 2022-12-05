#include "../klib/idt.hh"
#include "../klib/console.hh"
#include "../klib/assert.hh"
#include "../klib/console.hh"


extern "C" void exception_handler(regstate& regs) {
    terminal.print_line("Exception ", usize(regs.vector_code));
}

