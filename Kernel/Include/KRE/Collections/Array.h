//
// Created by ewogijk on 9/23/25.
//

#ifndef RUNEOS_ARRAY_H
#define RUNEOS_ARRAY_H

#include <KRE/TypeTraits.h>
#include <KRE/Utility.h>

namespace Rune {
    template <typename T>
    class ArrayIterator {
        T*     _data_buffer;
        size_t _position;
        size_t _arr_size;

      public:
        explicit ArrayIterator(T* data_buffer, size_t position, size_t arr_size)
            : _data_buffer(data_buffer),
              _position(position),
              _arr_size(arr_size) {}

        [[nodiscard]]
        auto has_next() const -> bool {
            return _position < _arr_size;
        }

        auto operator*() const -> T& { return _data_buffer[_position]; }

        auto operator->() const -> T* { return has_next() ? _data_buffer[_position] : nullptr; }

        // pre-increment
        auto operator++() -> ArrayIterator<T>& {
            ++_position;
            return *this;
        }

        // post-increment
        auto operator++(int) -> ArrayIterator<T> {
            ArrayIterator<T> tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator==(const ArrayIterator<T>& other) const -> bool {
            // null == null
            if (!has_next() && !other.has_next()) return true;
            // _data_buffer[_position] == null || null == _data_buffer[_position]
            if (!has_next() || !other.has_next()) return false;
            return _data_buffer[_position] == other._data_buffer[_position];
        }

        auto operator!=(const ArrayIterator<T>& other) const -> bool {
            // null == null
            if (!has_next() && !other.has_next()) return false;
            // _data_buffer[_position] == null || null == _data_buffer[_position]
            if (!has_next() || !other.has_next()) return true;
            return _data_buffer[_position] != other._data_buffer[_position];
        }
    };

    /**
     * Port of std::array.
     * @tparam N
     * @tparam T
     */
    template <typename T, size_t N>
    class Array {
        T _array[N]; // NOLINT

        void copy(const Array<T, N>& other) {
            for (size_t i = 0; i < N; i++) _array[i] = other._array[i];
        }

        void move_array(const Array<T, N>& other) {
            for (size_t i = 0; i < N; i++) _array[i] = move(other._array[i]);
        }

      public:
        explicit Array() = default;

        constexpr Array(const Array<T, N>& other) noexcept { copy(other); }

        constexpr Array(std::initializer_list<T> init) noexcept {
            size_t idx = 0;
            for (auto ele : init) {
                if (idx > N) break;
                _array[idx++] = ele;
            }
        }

        auto operator=(const Array<T, N>& other) noexcept -> Array<T, N>& {
            if (this == &other) {
                return *this;
            }
            copy(other);
            return *this;
        }

        Array(Array<T, N>&& other) noexcept { move_array(other); }

        auto operator=(Array<T, N>&& other) noexcept -> Array<T, N>& {
            move_array(other);
            return *this;
        }

        /**
         *
         * @return True: The array is empty, False: It is not empty.
         */
        auto is_empty() -> bool { return N == 0; }

        /**
         *
         * @return Number of elements in the array.
         */
        auto size() -> size_t { return N; }

        /**
         *
         * @return Pointer to the data buffer.
         */
        auto data() -> T* { return _array; }

        /**
         * If index>=N the behavior is undefined.
         * @param index
         * @return Element at index.
         */
        auto operator[](size_t index) -> T& { return _array[index]; }

        /**
         * If index>=N the behavior is undefined.
         * @param index
         * @return Element at index.
         */
        auto operator[](size_t index) const -> const T& { return _array[index]; }

        /**
         *
         * @return Iterator to the first element.
         */
        auto begin() -> ArrayIterator<T> { return ArrayIterator<T>(_array, 0, N); }

        /**
         *
         * @return Iterator to the element after the last element.
         */
        auto end() -> ArrayIterator<T> { return ArrayIterator<T>(_array, N, N); }
    };
} // namespace Rune

#endif // RUNEOS_ARRAY_H
