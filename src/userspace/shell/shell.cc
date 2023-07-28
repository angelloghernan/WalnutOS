#include "klib/console.hh"
#include "klib/assert.hh"
#include "klib/ps2/keyboard.hh"
#include "klib/ahci/ahci.hh"
#include "kernel/ext2/ext2_util.hh"

// Note: This is not going to run in userspace just yet. This program will first
// run in kernel space and will be used to aid in creating some programs on the file system
// Then, it will be ported to a user-mode version of itself using syscalls instead of directly
// invoking kernel functions.

using namespace wlib;
typedef Array<char, 512> InputBuffer;

void parse_input(InputBuffer& buffer, u16 end) {
    // For now, we are going to treat this entire thing as one buffer
    auto const as_slice = Slice<char>(buffer, 0, end);
    if (str("Hello") == as_slice) {
        terminal.print_line("Hello");
    } else if (str("dir") == as_slice) {
        
    } else if (str("mkdir") == as_slice) {
        // bleh
    } else if (str("mkfile") == as_slice) {
        // bleh
    }
}


void shell_main() {
    assert(sata_disk0.some(), "SATA disk must be made first");
    terminal.print('>');

    bool left_shift_pressed = false;
    bool right_shift_pressed = false;
    bool extended = false;

    InputBuffer input_buffer;
    u16 buf_ptr = 0;

    while (true) {
        using enum wlib::ps2::KeyboardResponse;
        for (auto maybe_key_code = keyboard.pop_response(); 
             maybe_key_code.some();
             maybe_key_code = keyboard.pop_response()) {
            auto key_code = maybe_key_code.unwrap();
            auto key = [&]{
                if (!left_shift_pressed && !right_shift_pressed) {
                    return ps2::Ps2Keyboard::response_to_char(key_code);
                } else {
                    return ps2::Ps2Keyboard::response_to_shifted_char(key_code);
                }
            }();

            if (key != '\0') {
                if (buf_ptr < input_buffer.len()) {
                    input_buffer[buf_ptr] = key;
                    ++buf_ptr;
                    terminal.print(key);
                }
                continue;
            }

            if (key_code == EnterDown) {
                terminal.print_line();
                parse_input(input_buffer, buf_ptr);
                terminal.print('>');
                buf_ptr = 0;
                continue;
            }

            if (!extended) {
                switch (key_code) {
                    case BackspaceDown:
                        terminal.print_back_char(' ');
                        break;
                    case LeftShiftDown:
                        left_shift_pressed = true;
                        break;
                    case RightShiftDown:
                        right_shift_pressed = true;
                        break;
                    case LeftShiftUp:
                        left_shift_pressed = false;
                        break;
                    case RightShiftUp:
                        right_shift_pressed = false;
                        break;
                    case NextIsExtended:
                        extended = true;
                        break;
                    default:
                        break;
                }
            } else {
                switch (key_code) {
                    case ExCursorUpDown:
                        terminal.row_up();
                        break;
                    case ExCursorDownDown:
                        terminal.row_down();
                        break;
                    case ExCursorLeftDown:
                        terminal.col_back();
                        break;
                    case ExCursorRightDown:
                        terminal.col_forward();
                        break;
                    default:
                        break;
                }
                extended = false;
            }
        }
        __asm__ volatile ("hlt");
    }
}
