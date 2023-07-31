#pragma once
#include "klib/slice.hh"
#include "klib/result.hh"
#include "klib/strings.hh"
#include "klib/ahci/ahci.hh"

namespace kernel::vfs {
    enum class ReadError : u8 {
        CacheFull,
        EndOfFile,
    };
    enum class WriteError : u8 {};
    enum class FileError : u8 {
        FSError,
        BitmapFull,
    };

    class FileHandle {
      public:
        auto static create(wlib::ahci::AHCIState* drive, 
                           wlib::str const name) -> wlib::Result<FileHandle, FileError>;

        auto static open(wlib::ahci::AHCIState* drive, 
                         u32 file_id) -> wlib::Result<FileHandle, FileError>;

        auto read(wlib::Slice<u8>& buffer) -> wlib::Result<u16, ReadError> ;

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
