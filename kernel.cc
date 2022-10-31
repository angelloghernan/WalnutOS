#include "strings.hh"
#include "console.hh"
void nothing() {}

extern "C" void kernel_main() {
    console::Console console;
    console.clear();
    console.print_line("Hello, World!");
    for (auto i = 0; i < 10; ++i) {
        console.print_line("What's up");
    }

    for (auto j = 0; j < 20; ++j) {
        console.print_line("The end");
    }
}
