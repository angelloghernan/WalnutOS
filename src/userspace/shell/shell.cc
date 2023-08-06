#include "klib/console.hh"
#include "klib/assert.hh"
#include "klib/ps2/keyboard.hh"
#include "klib/ahci/ahci.hh"
#include "kernel/vfs/vfs.hh"
#include "kernel/ext2/ext2_util.hh"

// Note: This is not going to run in userspace just yet. This program will first
// run in kernel space and will be used to aid in creating some programs on the file system
// Then, it will be ported to a user-mode version of itself using syscalls instead of directly
// invoking kernel functions.

using namespace wlib;
typedef Array<char, 512> InputBuffer;
using kernel::vfs::FileHandle;
FileHandle file(nullptr, 0, 0);

static void parse_input(InputBuffer& buffer, u16 end) {
    // For now, we are going to treat this entire thing as one buffer
    // TODO: Add splitting to strings
    auto as_str = str(buffer, 0, end);
    auto space_split = as_str.split(' ');

    if (space_split.done()) {
        return;
    }

    auto const command = space_split.next().unwrap();

    if (command == "Hello") {
        terminal.print_line("Hello");
    } else if (command == "dir") {
    } else if (command == "read") {
        if (!file.is_initialized()) {
            return;
        }
        
        Array<u8, 10> buf;
        Slice slice(buf);

        auto result = file.read(slice);
        if (result.is_err()) {
            terminal.print_line("Error reading from file");
        } else {
            for (auto ch : buf) {
                terminal.print(char(ch));
            }
            terminal.print_line();
        }
        
    } else if (command == "mkdir") {
    } else if (command == "mkfile") {
        auto maybe_arg = space_split.next();
        if (maybe_arg.none()) {
            return;
        }

        auto& arg = maybe_arg.unwrap();

        auto result = FileHandle::create(&sata_disk0.unwrap(), arg);

        if (result.is_ok()) {
            terminal.print_line("File handle successfully created with name ", arg);
            auto result2 = result.as_ok().write(str("Hello, World!").as_slice().to_raw_bytes());
            if (result2.is_err()) {
                switch (result2.as_err()) {
                    case kernel::vfs::WriteError::DiskError:
                        terminal.print_line("Disk Error");
                        break;
                    case kernel::vfs::WriteError::FSError:
                        terminal.print_line("FS Error");
                        break;
                    case kernel::vfs::WriteError::FileTooBig:
                        terminal.print_line("File too big?");
                        break;
                    case kernel::vfs::WriteError::OutOfContiguousSpace:
                        terminal.print_line("Out of contig");
                        break;
                }
            }
            file = util::move(result.as_ok());
        } else {
            terminal.print_line("File handle encountered an error");
        }
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
                        --buf_ptr;
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
