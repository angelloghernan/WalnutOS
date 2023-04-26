#include "../klib/strings.hh"
#include "../klib/console.hh"

extern "C" void kernel_main() {
    using console::Color;
    terminal.clear();
    terminal.print_line("Hello, world!");
    terminal.print_line_color(Color::LightGreen, Color::Black, "Test passed!");
    __asm__ volatile("hlt");
}
