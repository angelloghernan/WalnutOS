#include "../klib/strings.hh"
#include "../klib/console.hh"

extern "C" void kernel_main() {
    terminal.clear();
    terminal.print_line("Hello, world!");
    __asm__ volatile("hlt");
}
