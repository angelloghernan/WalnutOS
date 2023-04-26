#pragma once
#include "strings.hh"
#include "int.hh"
#include "ports.hh"

namespace console {
    enum class Color : u8 {
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
        void print_char(char const ch);
        void print_char_color(char const ch, Color const fg = Color::White, Color const bg = Color::Black);
        void print_addr(uptr const addr);
        void put_char_back(char const ch);
        void print_back_char(char const ch);
        void clear();

        void print() {
            move_cursor(m_row, m_col);
        }

        template<typename T, typename... Types>
        void print(T&& var1, Types&&... var2) {
            put(var1);
            print(var2...);
        }

        template<size_t S, typename... Types>
        void print(char const (&var1)[S], Types&&... var2) {
            print(str(var1, S), var2...);
        }
        
        template<typename... Types>
        void print_line(Types&&... var2) {
            print(var2...);
            new_line();
            move_cursor(m_row, m_col);
        }
        template<typename... Types>
        void print_debug(Types&&... var2) {
            #ifdef DEBUG
                print(var2...);
                new_line();
                move_cursor(m_row, m_col);
            #endif
        }

        template<typename... Types>
        void print_color(Color const fg, Color const bg) {}

        template<typename T, typename... Types>
        void print_color(Color const fg, Color const bg, T&& var1, Types&&... var2) {
            put_color(var1, fg, bg);
            print_color(fg, bg, var2...);
        }

        template<typename... Types>
        void print_line_color(Color const fg, Color const bg, Types&&... var2) {
            print_color(fg, bg, var2...);
            new_line();
            move_cursor(m_row, m_col);
        }

        Console() : m_col(0), m_row(0), console_page(reinterpret_cast<u16 *const>(0xb8000)) {
            ports::outb(Console::SET_REGISTER, Console::CURSOR_START);
            ports::outb(Console::CURSOR_CONTROL, 0x20);
            ports::outb(Console::SET_REGISTER, Console::CURSOR_START);
            // Upper two bits are reserved
            auto existing = ports::inb(Console::CURSOR_CONTROL) & 0xC;
            // Enable cursor (bit 5 set to 0) and set start position to 0
            ports::outb(Console::CURSOR_CONTROL, existing | 0);

            ports::outb(Console::SET_REGISTER, Console::CURSOR_END);
            // Upper three bits are reserved for cursor end
            existing = ports::inb(Console::CURSOR_CONTROL) & 0xE;
            // Set end position to 15 (take up entire block)
            ports::outb(Console::CURSOR_CONTROL, existing | 0);
        }

        void row_up() {
            if (m_row > 0) {
                --m_row;
                move_cursor(m_row, m_col);
            }
        }

        void row_down() {
            if (m_row < MAX_ROWS - 1) {
                ++m_row;
                move_cursor(m_row, m_col);
            }
        }

        void col_back() {
            if (m_col > 0) {
                --m_col;
                move_cursor(m_row, m_col);
            }
        }

        void col_forward() {
            if (m_col < MAX_COLS - 1) {
                ++m_col;
                move_cursor(m_row, m_col);
            }
        }

        void put_char(char const ch);
        void put_color(char const ch, Color const fg, Color const bg);
        void put_color(str const string, Color const fg, Color const bg);
        void put(str const string);
        void put(void* const ptr);
        void put(u32 num);
        void put(i32 const num) {
            put(u32(num));
        }

        void put(u16 const num) {
            put(u32(num));
        }

        void put(i16 const num) {
            put(u32(num));
        }

        void put(char const ch) {
            put_char(ch);
        }

        void put(u8 const num) {
            put(u32(num));
        }

        void put(i8 const num) {
            put(u32(num));
        }

        void put(bool b);
        
      private:
        u8 m_col;
        u8 m_row;
        u16 *const console_page;

        static u8 const MAX_ROWS = 25;
        static u8 const MAX_COLS = 80;

        static u16 const SET_REGISTER = 0x3D4;
        static u16 const CURSOR_CONTROL = 0x3D5;
        static u16 const CURSOR_START = 0x0A;
        static u16 const CURSOR_END = 0xB;
        static u16 const CURSOR_LOCATION_HIGH = 0xE;
        static u16 const CURSOR_LOCATION_LOW = 0xF;

        void move(i8 amt);
        auto constexpr static num_digits(usize num, u8 base) -> u8;

        auto constexpr static create_char(char const ch, Color const fg, Color const bg) -> u16 {
            auto const upper = static_cast<u8>(bg);
            auto const lower = static_cast<u8>(fg);

            u16 const color = (upper << 4) | (lower);

            return (color << 8) | ch; 
        }

        void new_line() {
            m_col = 0;
            m_row += 1;
            if (m_row == Console::MAX_ROWS) {
                m_row -= 1;
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
            
            auto constexpr blank = Console::create_char(' ', Color::White, Color::Black);
            for (auto j = 0; j < Console::MAX_COLS; ++j) {
                console_page[(Console::MAX_ROWS - 1) * Console::MAX_COLS + j] = blank;
            }
        }

        [[gnu::always_inline]] void move_cursor(u16 m_row, u16 m_col)  {
            u16 const pos = m_row * Console::MAX_COLS + m_col;
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
