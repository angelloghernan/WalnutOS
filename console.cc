#include "console.hh"

namespace console {
    void Console::print_char(char ch, Color fg, Color bg) {
        auto const color = Console::combine_colors(fg, bg);
        u16 test = u16(color) << 1;
        u16 const full_ch = (u16(color) << 8) | u16(ch);

        console_page[col + row * Console::MAX_COLS] = full_ch;

        col += 1;
        if (col == Console::MAX_COLS) {
            col = 0;
            row += 1;
        }
    }

    void Console::print(str s, Color fg, Color bg) {
        for (auto const ch : s) {
            print_char(ch, fg, bg);
        }
    }

    void Console::print_line(str s, Color fg, Color bg) {
        print(s, fg, bg);
        col = 0;
        row += 1;
    }

    void Console::clear() {
        for (auto i = 0; i < MAX_ROWS * MAX_COLS; ++i) {
            console_page[i] = 0;
        }

        col = row = 0;
    }
}
