#include "strings.hh"
#include "console.hh"
void nothing() {}

extern "C" void kernel_main() {
    using namespace console;
    Console console;
    console.clear();
    console.print("Hello, World!");
}
