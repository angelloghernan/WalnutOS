#pragma once
#include "../klib/console.hh"

inline void _assert(bool const condition, char const* const file, i32 const line,
                   char const* const function) {
    using namespace console;
    if (!condition) {
        terminal.print_line_color(Color::White, Color::Red, "Assertion failed! At ", 
                                  function, " in ", file, ':', line);
        while (true) {}
    }
}

inline void _warn(bool const condition, char const* const file, i32 const line,
                  char const* const function) {
    using namespace console;
    if (!condition) {
        terminal.print_line_color(Color::Black, Color::Yellow, "Warning at ", 
                                  function, " in ", file, ':', line);
    }
}

#define assert(condition) _assert(condition, __FILE__, __LINE__, __FUNCTION__)
#define warn_if(condition) _warn(!condition, __FILE__, __LINE__, __FUNCTION__)
#define warn_if_not(condition) _warn(condition, __FILE__, __LINE__, __FUNCTION__)
