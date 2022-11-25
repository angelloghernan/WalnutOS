#pragma once
#include "strings.hh"
#include "int.hh"
#include "ports.hh"

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
        void print_char(char const ch, Color const fg = Color::White, Color const bg = Color::Black);
        void print_addr(uptr const addr, Color const fg = Color::White, Color const bg = Color::Black);
        void put_char_back(char const ch, Color const fg = Color::White, Color const bg = Color::Black);
        void clear();

        void print() {
            move_cursor(row, col);
        }

        template<typename T, typename... Types>
        void print(T&& var1, Types&&... var2) {
            put(var1);
            print(var2...);
        }
        
        template<typename... Types>
        void print_line(Types&&... var2) {
            print(var2...);
            new_line();
            move_cursor(row, 0);
        }

        template<typename... Types>
        void print_color(Color const fg, Color const bg) {}

        template<typename T, typename... Types>
        void print_color(Color const fg, Color const bg, T&& var1, Types&&... var2) {
            put(var1, fg, bg);
            print_color(fg, bg, var2...);
        }

        template<typename... Types>
        void print_line_color(Color const fg, Color const bg, Types&&... var2) {
            print_color(fg, bg, var2...);
            new_line();
            move_cursor(row, 0);
        }

        Console() : col(0), row(0), console_page(reinterpret_cast<u16 *const>(0xb8000)) {
            ports::outb(Console::SET_REGISTER, Console::CURSOR_START);
            // Upper two bits are reserved
            auto existing = ports::inb(Console::CURSOR_CONTROL) & 0xC;
            // Enable cursor (bit 5 set to 0) and set start position to 0
            ports::outb(Console::CURSOR_CONTROL, existing | 0);

            ports::outb(Console::SET_REGISTER, Console::CURSOR_END);
            // Upper three bits are reserved for cursor end
            existing = ports::inb(Console::CURSOR_CONTROL) & 0xE;
            // Set end position to 15 (take up entire block)
            ports::outb(Console::CURSOR_CONTROL, existing | 15);
        }
        
      private:
        u8 col;
        u8 row;
        u16 *const console_page;

        static u8 const MAX_ROWS = 25;
        static u8 const MAX_COLS = 80;

        static u16 const SET_REGISTER = 0x3D4;
        static u16 const CURSOR_CONTROL = 0x3D5;
        static u16 const CURSOR_START = 0x0A;
        static u16 const CURSOR_END = 0xB;
        static u16 const CURSOR_LOCATION_HIGH = 0xE;
        static u16 const CURSOR_LOCATION_LOW = 0xF;

        void put_char(char const ch, Color const fg = Color::White, Color const bg = Color::Black);
        void put(str const string, Color const fg = Color::White, Color const bg = Color::Black);
        void put(void* const ptr, Color const fg = Color::White, Color const bg = Color::Black);
        void put(char const ch, Color const fg = Color::White, Color const bg = Color::Black);
        void put(i32 num, Color const fg = Color::White, Color const bg = Color::Black);
        void put(u32 num, Color const fg = Color::White, Color const bg = Color::Black);
        void put(u8 num, Color const fg = Color::White, Color const bg = Color::Black);
        void put(i8 num, Color const fg = Color::White, Color const bg = Color::Black);
        void move(i8 amt);
        auto constexpr static num_digits(usize num, u8 base) -> i8;

        auto static create_char(char const ch, Color const fg, Color const bg) -> u16 {
            auto const upper = static_cast<u8>(bg);
            auto const lower = static_cast<u8>(fg);

            u16 const color = (upper << 4) | (lower);

            return (color << 8) | ch; 
        }

        void new_line() {
            col = 0;
            row += 1;
            if (row == Console::MAX_ROWS) {
                row -= 1;
                scroll();
            }
        }
        
        void scroll() {
            for (auto i = 0; i < Console::MAX_ROWS - 1; ++i) {
                for (auto j = 0; j < Console::MAX_COLS; ++j) {
                    auto const ch_below = console_page[(i + 1) * Console::MAX_COLS + j];
                    console_page[i * Console::MAX_COLS + j] = ch_below;
                }
            }
            
            auto const blank = Console::create_char(' ', Color::White, Color::Black);
            for (auto j = 0; j < Console::MAX_COLS; ++j) {
                console_page[(Console::MAX_ROWS - 1) * Console::MAX_COLS + j] = blank;
            }
        }

        void move_cursor(u16 row, u16 col) {
            u16 const pos = row * Console::MAX_COLS + col;
            u8 const pos_lo = pos & 0xFF;
            u8 const pos_hi = (pos >> 8) & 0xFF;

            ports::outb(Console::SET_REGISTER, Console::CURSOR_LOCATION_LOW);
            ports::outb(Console::CURSOR_CONTROL, pos_lo);
            ports::outb(Console::SET_REGISTER, Console::CURSOR_LOCATION_HIGH);
            ports::outb(Console::CURSOR_CONTROL, pos_hi);
        }
    };
}

extern console::Console terminal;
