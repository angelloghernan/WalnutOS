#pragma once
#include "klib/console.hh"

inline void _assert(bool const condition, wlib::str const file, i32 const line,
                    wlib::str const function, wlib::str const message) {
    using namespace wlib::console;
    if (!condition) [[unlikely]] {
        terminal.print_line_color(Color::White, Color::Red,
                                  "Assertion failed: ", message, " at ",
                                  function, " in ", file, ':', line);
        while (true) {
        }
    }
}

inline void _warn(bool const condition, wlib::str const file, i32 const line,
                  wlib::str const function, wlib::str const message) {
    using namespace wlib::console;
    if (!condition) [[unlikely]] {
        terminal.print_line_color(Color::Black, Color::Yellow,
                                  "Warning: ", message, " at ", function,
                                  " in ", file, ':', line);
    }
}

#define assert(condition, message)                                             \
    _assert(condition, __FILE__, __LINE__, __FUNCTION__, message)
#define warn_if(condition, message)                                            \
    _warn(!condition, __FILE__, __LINE__, __FUNCTION__, message)
#define warn_if_not(condition, message)                                        \
    _warn(condition, __FILE__, __LINE__, __FUNCTION__, message)

#ifdef DEBUG
#define assert_debug(condition, message)                                       \
    _assert(condition, __FILE__, __LINE__, __FUNCTION__, message)
#else
#define assert_debug(condition, message)                                       \
    {}
#endif
