#include "console.hh"
#include "../klib/assert.hh"
#include "../klib/array.hh"

console::Console terminal;
namespace console {
    void Console::put_char(char ch, Color const fg, Color const bg) {
        if (ch == '\n') {
            new_line();
            return;
        }

        auto const full_ch = Console::create_char(ch, fg, bg);
        console_page[col + row * Console::MAX_COLS] = full_ch;

        col += 1;
        if (col == Console::MAX_COLS) {
            new_line();
        }
    }

    void Console::move(i8 amt) {
        i32 new_col = col + amt;
        if (new_col < 0) {
            col = 0;
            if (row > 0) {
                row -= 1;
            }
        } else if (new_col >= Console::MAX_COLS) {
            col = new_col - Console::MAX_COLS;
            row += 1;
            if (row == Console::MAX_ROWS) {
                scroll();
            }
        } else {
            col = new_col;
        }
    }

    void Console::put_char_back(char const ch, Color const fg, Color const bg) {
        auto const full_ch = Console::create_char(ch, fg, bg);
        console_page[col + row * Console::MAX_COLS] = full_ch;
        if (col == 0) {
            if (row != 0) {
                col = Console::MAX_COLS - 1;
                --row;
            }
        } else {
            --col;
        }
    }

    void Console::print_char(char const ch, Color const fg, Color const bg) {
        put_char(ch, fg, bg);
        move_cursor(row, col);
    }

    void Console::clear() {
        for (auto i = 0; i < MAX_ROWS * MAX_COLS; ++i) {
            console_page[i] = Console::create_char(' ', Color::White, Color::Black);
        }

        col = row = 0;
        move_cursor(row, col);
    }

    void Console::put(str const string, Color const fg, Color const bg) {
        for (auto const ch : string) {
            put_char(ch, fg, bg);
        }  
    } 

    void Console::put(u32 num, Color const fg, Color const bg) {
        auto const digits = num_digits(num, 10);
        move(digits - 1);
        do {
            auto const digit = char(num % 10 + '0');
            put_char_back(digit, fg, bg);
            num /= 10;
        } while (num > 0);
        move(digits + 1);
        move_cursor(row, col);
    }

    void Console::put(i32 const num, Color const fg, Color const bg) {
        put(u32(num), fg, bg);
    }

    void Console::put(char const ch, Color const fg, Color const bg) {
        put_char(ch, fg, bg);
    }

    void Console::put(u8 const num, Color const fg, Color const bg) {
        put(u32(num), fg, bg);
    }

    void Console::put(i8 const num, Color const fg, Color const bg) {
        put(u32(num), fg, bg);
    }

    void Console::put(void* const ptr, Color const fg, Color const bg) {
        static constexpr Array<char const, 16> hex_values {'0', '1', '2', '3', '4', '5', '6', '7',
                                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        auto addr = reinterpret_cast<uptr>(ptr);
        auto const digits = num_digits(addr, 16);
        move(digits - 1);
        do {
            auto const ch = char(hex_values[addr % 16]);
            put_char_back(ch);
            addr /= 16;
        } while (addr > 0);
        move(digits + 1);
        move_cursor(row, col);
    }

    auto constexpr Console::num_digits(usize num, u8 base) -> i8 {
        i8 counter = 0;
        do {
            num /= base;
            ++counter;
        } while (num > 0);
        return counter;
    }
    
}
