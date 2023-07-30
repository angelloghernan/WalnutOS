#pragma once
#include "klib/slice.hh"
#include "klib/result.hh"
#include "klib/ahci/ahci.hh"

namespace kernel::vfs {
    enum class ReadError : u8 {};
    enum class WriteError : u8 {};
    enum class FileError : u8 {
        FSError,
    };

    class FileHandle {
      public:
        auto static create(wlib::ahci::AHCIState* drive, 
                           u32 file_id) -> wlib::Result<FileHandle, FileError>;

        auto read(wlib::Slice<u8>& buffer) -> wlib::Result<wlib::Null, ReadError>;


        auto write(wlib::Slice<u8> const& buffer)-> wlib::Result<wlib::Null, WriteError>;

      private:
        u32 _file_id;
        u32 _position;
        wlib::ahci::AHCIState& _drive;
        FileHandle(wlib::ahci::AHCIState* drive, u32 file_id) 
            : _file_id(file_id), _position(0), _drive(*drive) {};
    };

}; // namespace kernel::vfs
