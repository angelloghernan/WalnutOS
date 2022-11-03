#include "../klib/strings.hh"
#include "../klib/console.hh"
#include "../klib/array.hh"
#include "../klib/idt.hh"
console::Console terminal;

extern "C" void kernel_main() {
    idt.init();
    terminal.clear();
    terminal.print_line("Hello, World!");
    Array<int, 10> arr {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    for (auto const ele : arr) {
        char const ch = ele + '0';
        terminal.print_char(ch);
        terminal.print_char('\n');
    }
}
