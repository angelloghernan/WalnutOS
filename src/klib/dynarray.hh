#pragma once
#include "klib/int.hh"
#include "klib/option.hh"
#include "klib/slice.hh"
#include "klib/util.hh"
#include "kernel/alloc.hh"

namespace wlib {
    // DynArray -- Like a regular vector, but with a fixed size. 
    // The size cannot be changed after initialization.
    // NOTE: It is much preferred to avoid dynamic allocation if at all possible
    // since we only have a page frame allocator (buddy allocator).
    // We need a slab allocator to make this cheaper to use
    template<typename T>
    class DynArray {
      public:
        // Deleting copy constructor because we don't want unnecessary allocations at this stage
        // Use memcpy if necessary or just manually loop over the arrays
        DynArray(DynArray const&) = delete;

        DynArray(DynArray&& other) {
            _array = other._array;
            _size = other._size;
            other._array = nullptr;
        }

        [[nodiscard]] auto static initialize(usize size) -> Option<DynArray> {
            auto const ptr = simple_allocator.kalloc(size);
            if (ptr.none()) {
                return Option<DynArray>::None();
            }
            
            return Option<DynArray>::Some((T*)(ptr.unwrap()), size);
        }

        void fill(T const& element) {
            for (auto i = 0; i < _size; ++i) {
                _array[i] = element;
            }
        }


        [[nodiscard]] auto operator[](usize idx) -> T& { return _array[idx]; }
        [[nodiscard]] auto operator[](usize idx) const -> T const& { return _array[idx]; }

        auto get(usize idx) -> Option<T> {
            if (idx < _size) {
                return Option<T>::Some(_array[idx]);
            } else {
                return Option<T>::None();
            }
        } 

        // Return the number of elements in the array.
        [[nodiscard]] auto constexpr len() const -> usize { return _size; }

        // Return the size in bytes.
        [[nodiscard]] auto constexpr size() const -> usize { return _size * sizeof(T); }

        [[nodiscard]] auto constexpr data() -> T* { return &_array[0]; }
        [[nodiscard]] auto constexpr data() const -> T const* { return &_array[0]; }

        [[nodiscard]] auto constexpr begin() const -> const_iterator<T> { return const_iterator<T>(&_array[0]); }
        [[nodiscard]] auto constexpr end() const -> const_iterator<T> { return const_iterator<T>(&_array[_size]); }

        [[nodiscard]] auto constexpr begin() -> iterator<T> { return iterator<T>(&_array[0]); }
        [[nodiscard]] auto constexpr end() -> iterator<T> { return iterator<T>(&_array[_size]); }
        
        [[nodiscard]] auto constexpr first() -> T& { return _array[0]; }
        [[nodiscard]] auto constexpr first() const -> T const& { return _array[0]; }

        [[nodiscard]] auto constexpr last() -> T& { return _array[_size - 1]; }
        [[nodiscard]] auto constexpr last() const -> T const& { return _array[_size - 1]; }

        [[nodiscard]] auto constexpr into_slice() -> Slice<T> {
            return Slice<T>(_array, _size);
        }

        [[nodiscard]] auto constexpr into_slice() const -> Slice<const T> {
            return Slice<const T>(_array, _size);
        }

        ~DynArray() {
            if (_array != nullptr) {
                simple_allocator.kfree(uptr(_array));
            }
        }

      private:
        DynArray(T* ptr, usize size) : _array(ptr), _size(size) {};
        T* _array;
        usize _size;

        friend class Option<DynArray>;
    };
}; // namespace wlib
