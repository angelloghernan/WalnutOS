#include "console.hh"

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

    void Console::print(str const s, Color const fg, Color const bg) {
        for (auto const ch : s) {
            put_char(ch, fg, bg);
        }
        move_cursor(row, col);
    }

    void Console::print_line(str const s, Color const fg, Color const bg) {
        print(s, fg, bg);
        new_line();
        move_cursor(row, 0);
    }

    void Console::clear() {
        for (auto i = 0; i < MAX_ROWS * MAX_COLS; ++i) {
            console_page[i] = Console::create_char(' ', Color::White, Color::Black);
        }

        col = row = 0;
        move_cursor(row, col);
    }
}
