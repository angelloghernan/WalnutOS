#pragma once
#include "klib/slice.hh"
#include "klib/result.hh"
#include "klib/ahci/ahci.hh"

namespace kernel::vfs {
    enum class ReadError : u8 {
        CacheFull,
    };
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
        wlib::ahci::AHCIState& _drive;
        u32 _file_id;
        u32 _position;
        u32 _sector;
        FileHandle(wlib::ahci::AHCIState* drive, u32 file_id, u32 sector) 
            : _drive(*drive), _file_id(file_id), _position(0), _sector(sector) {};
        friend class wlib::Result<FileHandle, FileError>;
    };

}; // namespace kernel::vfs
