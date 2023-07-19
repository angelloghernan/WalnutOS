#include "console.hh"
#include "../klib/assert.hh"
#include "../klib/array.hh"

wlib::console::Console terminal;

namespace wlib::console {
    void Console::put_char(char const ch) {
        if (ch == '\n') {
            new_line();
            return;
        }

        auto const full_ch = Console::create_char(ch, Color::White, Color::Black);
        console_page[m_col + m_row * Console::MAX_COLS] = full_ch;

        m_col += 1;
        if (m_col == Console::MAX_COLS) {
            new_line();
        }
    }

    void Console::put_color(char const ch, Color const fg, Color const bg) {
        if (ch == '\n') {
            new_line();
            return;
        }

        auto const full_ch = Console::create_char(ch, fg, bg);
        console_page[m_col + m_row * Console::MAX_COLS] = full_ch;

        m_col += 1;
        if (m_col == Console::MAX_COLS) {
            new_line();
        }
    }

    void Console::move(i8 amt) {
        i32 new_col = m_col + amt;
        if (new_col < 0) {
            m_col = 0;
            if (m_row > 0) {
                m_row -= 1;
            }
        } else if (new_col >= Console::MAX_COLS) {
            m_col = new_col - Console::MAX_COLS;
            m_row += 1;
            if (m_row == Console::MAX_ROWS) {
                scroll();
            }
        } else {
            m_col = new_col;
        }
    }

    void Console::put_char_back(char const ch) {
        auto const full_ch = Console::create_char(ch, Color::White, Color::Black);
        console_page[m_col + m_row * Console::MAX_COLS] = full_ch;
        if (m_col == 0) {
            if (m_row != 0) {
                m_col = Console::MAX_COLS - 1;
                --m_row;
            }
        } else {
            --m_col;
        }
    }


    void Console::print_back_char(char const ch) {
        if (m_col == 0) {
            if (m_row != 0) {
                m_col = Console::MAX_COLS - 1;
                --m_row;
            }
        } else {
            --m_col;
        }
        auto const full_ch = Console::create_char(ch, Color::White, Color::Black);
        console_page[m_col + m_row * Console::MAX_COLS] = full_ch;
        move_cursor(m_row, m_col);
    }

    void Console::print_char(char const ch) {
        put_char(ch);
        move_cursor(m_row, m_col);
    }

    void Console::clear() {
        for (auto i = 0; i < MAX_ROWS * MAX_COLS; ++i) {
            console_page[i] = Console::create_char(' ', Color::White, Color::Black);
        }

        m_col = m_row = 0;
        move_cursor(m_row, m_col);
    }

    void Console::put(str const string) {
        for (auto const ch : string) {
            put_char(ch);
        }  
    } 

    void Console::put(u32 num) {
        auto const digits = num_digits(num, 10);
        move(digits - 1);
        do {
            auto const digit = char(num % 10 + '0');
            put_char_back(digit);
            num /= 10;
        } while (num > 0);
        move(digits + 1);
    }

    void Console::put(bool const b) {
        static constexpr Array<str const, 2> outputs {"false", "true"};
        put(outputs[b]);
    }

    void Console::put(void* const ptr) {
        static constexpr Array<char const, 16> hex_values {'0', '1', '2', '3', '4', '5', '6', '7',
                                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        auto constexpr prefix = str("0x");
        put(prefix);
        auto addr = reinterpret_cast<uptr>(ptr);
        auto const digits = num_digits(addr, 16);
        move(digits - 1);
        do {
            auto const ch = char(hex_values[addr % 16]);
            put_char_back(ch);
            addr /= 16;
        } while (addr > 0);
        move(digits + 1);
    }

    auto constexpr Console::num_digits(usize num, u8 base) -> u8 {
        u8 counter = 0;
        do {
            num /= base;
            ++counter;
        } while (num > 0);
        return counter;
    }
    
    void Console::put_color(str const string, Color const fg, Color const bg) {
        for (auto const ch : string) {
            put_color(ch, fg, bg);
        }
    }
}
