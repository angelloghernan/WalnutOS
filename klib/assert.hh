#pragma once
#include "../klib/console.hh"

inline void assert(bool const condition, char const* const file, i32 const line, 
                   char const* const function) {
    using namespace console;
    if (!condition) {
        terminal.print_line_color(Color::White, Color::Red, "Assertion failed! At ", function, " in ", file, ':', line);
    }
    while (true) {}
}

#define ASSERT(condition) assert(condition, __FILE__, __LINE__, __FUNCTION__)
