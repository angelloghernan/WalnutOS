#pragma once
#include "klib/slice.hh"
#include "klib/result.hh"
#include "klib/strings.hh"
#include "klib/nullable.hh"
#include "klib/ahci/ahci.hh"

namespace kernel::vfs {
    enum class ReadError : u8 {
        CacheFull,
        EndOfFile,
    };

    enum class WriteError : u8 {
        FSError,
        FileTooBig,
        OutOfContiguousSpace,
        DiskError,
    };

    enum class FileError : u8 {
        FSError,
        BitmapFull,
    };

    class FileHandle {
      public:
        FileHandle(wlib::ahci::AHCIState* drive, u32 file_id, u32 sector, u32 size)
            : _drive(drive), _file_id(file_id), _position(0), _size(size), _sector(sector) {};

        FileHandle(FileHandle&& handle) 
            : _drive(handle._drive), _file_id(handle._file_id), 
              _position(handle._position), _size(handle._size), _sector(handle._sector) {
            handle._file_id = u32(-1);
        } 

        void operator=(FileHandle&& handle) {
            _drive = handle._drive;
            _file_id = handle._file_id;
            _position = handle._position;
            _sector = handle._sector;
            _size = handle._size;
            handle._file_id = u32(-1);
        }

        auto static create(wlib::ahci::AHCIState* drive, 
                           wlib::str const name) -> wlib::Result<FileHandle, FileError>;

        auto static open(wlib::ahci::AHCIState* drive, 
                         u32 file_id) -> wlib::Result<FileHandle, FileError>;

        auto read(wlib::Slice<u8>& buffer) -> wlib::Result<u16, ReadError> ;

        auto write(wlib::Slice<u8> const& buffer)-> wlib::Result<u32, WriteError>;

        auto inline constexpr is_initialized() -> bool {
            return _drive != nullptr;
        }

      private:
        wlib::ahci::AHCIState* _drive;
        u32 _file_id;
        u32 _position;
        u32 _size;
        u32 _sector;
        
        auto sector_of_position() -> wlib::Nullable<u32, u32(-1)>; 

        friend class wlib::Result<FileHandle, FileError>;
    };

}; // namespace kernel::vfs
