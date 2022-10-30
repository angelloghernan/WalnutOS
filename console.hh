#pragma once
#include "strings.hh"
#include "int.hh"

namespace console {
    enum class Color {
        Black = 0x0,
        Blue = 0x1,
        Green = 0x2,
        Cyan = 0x3,
        Red = 0x4,
        Magenta = 0x5,
        Brown = 0x6,
        LightGray = 0x7,
        DarkGray = 0x8,
        LightBlue = 0x9,
        LightGreen = 0xA,
        LightCyan = 0xB,
        LightRed = 0xC,
        Pink = 0xD,
        Yellow = 0xE,
        White = 0xF,
    };

    class Console {
      public:
        void print_char(char ch, Color fg = Color::White, Color bg = Color::Black);
        void print(str s, Color fg = Color::White, Color bg = Color::Black);
        void print_line(str s, Color fg = Color::White, Color bg = Color::Black);
        void clear();

        Console() : col(0), row(0), console_page(reinterpret_cast<u16 *const>(0xb8000)) {}
        
      private:
        u8 col;
        u8 row;
        u16 *const console_page;
        static u8 const MAX_ROWS = 25;
        static u8 const MAX_COLS = 80;

        static u8 combine_colors(Color fg, Color bg) {
            auto const upper = static_cast<u8>(bg);
            auto const lower = static_cast<u8>(fg);

            return (upper << 4) | (lower);
        }
    };
}
