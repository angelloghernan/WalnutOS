#pragma once
#include "../klib/console.hh"

inline void _assert(bool const condition, char const* const file, i32 const line,
                   char const* const function) {
    using namespace console;
    if (!condition) {
        terminal.print_line_color(Color::White, Color::Red, "Assertion failed! At ", function, " in ", file, ':', line);
        while (true) {}
    }
}

#define assert(condition) _assert(condition, __FILE__, __LINE__, __FUNCTION__)
