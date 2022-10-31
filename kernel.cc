#include "strings.hh"
#include "console.hh"
void nothing() {}
console::Console terminal;

extern "C" void kernel_main() {
    terminal.clear();
    terminal.print_line("Hello, World!");
    for (auto i = 0; i < 10; ++i) {
        terminal.print_line("What's up");
    }

    for (auto j = 0; j < 20; ++j) {
        terminal.print_line("The end");
    }
}
