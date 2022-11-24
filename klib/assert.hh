#pragma once
#include "../klib/console.hh"

inline void _assert(bool const condition, char const* const file, i32 const line,
                   char const* const function, str const message) {
    using namespace console;
    if (!condition) {
        terminal.print_line_color(Color::White, Color::Red, "Assertion failed: ",
                                  message, " at ", function, " in ", file, ':', line);
        while (true) {}
    }
}

inline void _warn(bool const condition, char const* const file, i32 const line,
                  char const* const function, str const message) {
    using namespace console;
    if (!condition) {
        terminal.print_line_color(Color::Black, Color::Yellow, "Warning: ", message, 
                                  " at ", function, " in ", file, ':', line);
    }
}

#define assert(condition, message) _assert(condition, __FILE__, __LINE__, __FUNCTION__, message)
#define warn_if(condition, message) _warn(!condition, __FILE__, __LINE__, __FUNCTION__, message)
#define warn_if_not(condition, message) _warn(condition, __FILE__, __LINE__, __FUNCTION__, message)
