#include "console.hh"
#include "../klib/assert.hh"

console::Console terminal;
namespace console {
    void Console::put_char(char ch, Color fg, Color bg) {
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

    void Console::print_char(char const ch, Color const fg, Color const bg) {
        put_char(ch, fg, bg);
        move_cursor(row, col);
    }

    void Console::print_addr(uptr addr, Color const fg, Color const bg) {
        do {
            auto const digit = char(addr % 10 + '0');
            put_char(digit, fg, bg);
            addr /= 10;
        } while (addr > 0);
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
        do {
            auto const digit = char(num % 10 + '0');
            put_char(digit, fg, bg);
            num /= 10;
        } while (num > 0);
        move_cursor(row, col);
    }

    void Console::put(i32 num, Color const fg, Color const bg) {
        put(u32(num), fg, bg);
    }

    void Console::put(char const ch, Color const fg, Color const bg) {
        put_char(ch, fg, bg);
    }

    void Console::put(u8 num, Color const fg, Color const bg) {
        put(u32(num), fg, bg);
    }

    void Console::put(i8 num, Color const fg, Color const bg) {
        put(u32(num), fg, bg);
    }
    
}
