
/*
 *  Copyright 2025 Ewogijk
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef RUNEOS_UTILITY_H
#define RUNEOS_UTILITY_H

#include <stddef.h>

#include <KRE/CppLanguageSupport.h>
#include <KRE/TypeTraits.h>
#include <KRE/Utility.h>

namespace Rune {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      General Stuff
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    template <typename T> struct RemoveRef {
        using Type = T;
    };

    template <typename T> struct RemoveRef<T&> {
        using Type = T;
    };

    template <typename T> struct RemoveRef<T&&> {
        using Type = T;
    };

    /**
     * Implementation of std::move.
     *
     * @tparam T
     * @param param
     * @return
     */
    template <typename T> auto move(T&& param) -> typename RemoveRef<T>::Type&& { // NOLINT
        return static_cast<typename RemoveRef<T>::Type&&>(param);
    }

    /**
     * Implementation of std::forward.
     *
     * @tparam T
     * @param param
     * @return
     */
    template <typename T> constexpr auto forward(typename RemoveRef<T>::Type& param) -> T&& {
        return static_cast<T&&>(param);
    }

    /**
     * Implementation of std::forward.
     *
     * @tparam T
     * @param param
     * @return
     */
    template <typename T>
    constexpr auto forward(typename RemoveRef<T>::Type&& param) -> T&& { // NOLINT
        return static_cast<T&&>(param);
    }

    /**
     * Swap the position of the two pointers.
     *
     * @param one
     * @param two
     */
    template <typename T> void swap(T& one, T& two) noexcept {
        T temp = move(one);
        one    = move(two);
        two    = move(temp);
    }

    /**
     * @brief An RGBA pixel.
     */
    struct Pixel {
        U8 red   = 0;
        U8 green = 0;
        U8 blue  = 0;
        U8 alpha = 0;

        auto operator==(const Pixel& other) const -> bool;

        auto operator!=(const Pixel& other) const -> bool;
    };

    /**
     * @brief Common pixel colors.
     */
    namespace Pixie {
        constexpr Pixel BLACK = {.red = 0, .green = 0, .blue = 0, .alpha = 0};
        constexpr Pixel WHITE = {.red = 255, .green = 255, .blue = 255, .alpha = 0};
        constexpr Pixel RED   = {.red = 255, .green = 0, .blue = 0, .alpha = 0};
        constexpr Pixel GREEN = {.red = 0, .green = 255, .blue = 0, .alpha = 0};
        constexpr Pixel BLUE  = {.red = 0, .green = 0, .blue = 255, .alpha = 0};

        constexpr Pixel VSCODE_CYAN   = {.red = 17, .green = 168, .blue = 205, .alpha = 0};
        constexpr Pixel VSCODE_BLUE   = {.red = 36, .green = 114, .blue = 200, .alpha = 0};
        constexpr Pixel VSCODE_YELLOW = {.red = 229, .green = 229, .blue = 16, .alpha = 0};
        constexpr Pixel VSCODE_WHITE  = {.red = 229, .green = 229, .blue = 229, .alpha = 0};
        constexpr Pixel VSCODE_RED    = {.red = 205, .green = 49, .blue = 49, .alpha = 0};
    } // namespace Pixie

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                               Array operations
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    template <typename T> auto partition(T array[], int left, int high) -> int {
        T   pivot = array[high];
        int idx   = left - 1;
        for (int j = left; j <= high - 1; j++) {
            if (array[j] <= pivot) {
                idx++;
                swap(array[idx], array[j]);
            }
        }
        swap(array[idx + 1], array[high]);
        return idx + 1;
    }

    template <typename T> void quick_sort(T array[], int low, int high) {
        if (low < high) {
            const int pivot = partition(array, low, high);
            quick_sort(array, low, pivot - 1);
            quick_sort(array, pivot + 1, high);
        }
    }

    /**
     * Sort the given array of objects inplace that must overload the "<=" operator.
     *
     * @tparam T        Type of the array objects.
     * @param array     Array to be sorted.
     * @param arr_size   Size of the array.
     */
    template <typename T> void sort(T array[], const size_t arr_size) {
        quick_sort(array, 0, arr_size - 1);
    }

    /**
     * Delete the requested number of elements starting from the given idx from the array.
     *
     * @tparam T
     * @param arr
     * @param idx   Starting point of deletion.
     * @param count Number of elements to delete.
     */
    template <typename T> void array_delete(T arr[], size_t idx, size_t& count) {
        memmove(arr + idx, arr + idx + 1, sizeof(T) * (count - idx - 1));
        count--;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                               Bit Manipulation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     *
     * @tparam T     Number type.
     * @param num
     * @param offset Bit offset.
     * @return True: The bit at offset is set, False: The bit is not set.
     */
    template <Integer T> auto bit_check(T num, size_t offset) -> bool { return num >> offset & 1; }

    /**
     * Set the bit at offset and leave all other bits as they are.
     *
     * @tparam T     Number type.
     * @param num
     * @param offset Bit offset.
     * @return The number with the bit at offset set.
     */
    template <Integer T> auto bit_set(T num, const size_t offset) -> T { return num | 1 << offset; }

    /**
     * Clear the bit at offset and leave all other bits as they are.
     *
     * @tparam T     Number type.
     * @param num
     * @param offset Bit offset.
     * @return The number with the bit at offset cleared.
     */
    template <Integer T> auto bit_clear(T num, const size_t offset) -> T {
        return num & ~(1 << offset);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Byte Order
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

#define BYTE_ORDERS(X)                                                                             \
    X(ByteOrder, LITTLE_ENDIAN, 1)                                                                 \
    X(ByteOrder, BIG_ENDIAN, 2)

    DECLARE_TYPED_ENUM(ByteOrder, U8, BYTE_ORDERS, 0) // NOLINT

    /**
     * Little endian conversions.
     */
    class LittleEndian {
      public:
        /**
         * Interpret the next two bytes in the buf as little endian encoded unsigned 16 bit integer.
         *
         * @param buf
         *
         * @return Little endian encoded unsigned 16 bit integer.
         */
        static auto to_U16(const U8* buf) -> U16;

        /**
         * Interpret the next four bytes in the buf as little endian encoded unsigned 32 bit
         * integer.
         *
         * @param buf
         *
         * @return Little endian encoded unsigned 32 bit integer.
         */
        static auto to_U32(const U8* buf) -> U32;

        /**
         * Interpret the next eight bytes in the buf as little endian encoded unsigned 64 bit
         * integer.
         *
         * @param buf
         *
         * @return Little endian encoded unsigned 64 bit integer.
         */
        static auto to_U64(const U8* buf) -> U64;

        /**
         * Set the next two values in the buf to the little endian encoded bytes of the unsigned 16
         * bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U16 num, U8* buf);

        /**
         * Set the next four values in the buf to the little endian encoded bytes of the unsigned 32
         * bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U32 num, U8* buf);

        /**
         * Set the next eight values in the buf to the little endian encoded bytes of the unsigned
         * 64 bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U64 num, U8* buf);
    };

    /**
     * Big endian conversions.
     */
    class BigEndian {
      public:
        /**
         * Interpret the next two bytes in the buf as big endian encoded unsigned 16 bit integer.
         *
         * @param buf
         *
         * @return Big endian encoded unsigned 16 bit integer.
         */
        static auto to_U16(const U8* buf) -> U16;

        /**
         * Interpret the next four bytes in the buf as big endian encoded unsigned 32 bit integer.
         *
         * @param buf
         *
         * @return Big endian encoded unsigned 32 bit integer.
         */
        static auto to_U32(const U8* buf) -> U32;

        /**
         * Interpret the next eight bytes in the buf as big endian encoded unsigned 64 bit integer.
         *
         * @param buf
         *
         * @return Big endian encoded unsigned 64 bit integer.
         */
        static auto to_U64(const U8* buf) -> U64;

        /**
         * Set the next two values in the buf to the big endian encoded bytes of the unsigned 16 bit
         * integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U16 num, U8* buf);

        /**
         * Set the next four values in the buf to the big endian encoded bytes of the unsigned 32
         * bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U32 num, U8* buf);

        /**
         * Set the next eight values in the buf to the big endian encoded bytes of the unsigned 64
         * bit integer value.
         *
         * @param num
         * @param buf
         */
        static void to_bytes(U64 num, U8* buf);
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                               std::hash Port
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Provides hash support for a type.
     *
     * A hash implementation must satisfy the following requirements:
     * <ol>
     * <li><a
     * href="https://en.cppreference.com/w/cpp/named_req/DefaultConstructible">DefaultConstructible</a></li>
     * <li><a
     * href="https://en.cppreference.com/w/cpp/named_req/CopyAssignable>CopyAssignable</a></li>
     * <li>Implement the "size_t operator()(const int& key) const" function making the hash
     * calculation.</li>
     * </ol>
     * @tparam K Hash type.
     */
    template <class K> struct Hash;

    template <> struct Hash<signed char> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const signed char& key) const -> size_t { return key; }
    };

    template <> struct Hash<char> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const char& key) const -> size_t { return key; }
    };

    template <> struct Hash<short> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const short& key) const -> size_t { return key; }
    };

    template <> struct Hash<int> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const int& key) const -> size_t { return key; }
    };

    template <> struct Hash<long> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const long& key) const -> size_t { return key; }
    };

    template <> struct Hash<long long> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const long long& key) const -> size_t { return key; }
    };

    template <> struct Hash<unsigned char> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned char& key) const -> size_t { return key; }
    };

    template <> struct Hash<unsigned short> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned short& key) const -> size_t { return key; }
    };

    template <> struct Hash<unsigned int> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned int& key) const -> size_t { return key; }
    };

    template <> struct Hash<unsigned long> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned long& key) const -> size_t { return key; }
    };

    template <> struct Hash<unsigned long long> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned long long& key) const -> size_t { return key; }
    };

    template <> struct Hash<float> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const float& key) const -> size_t {
            // Calc hash for floats up to 10 digit precision
            float            num   = key;
            static const int pow10 = 1000000000;
            long             whole = (long) num;
            float            frac  = (num - (float) whole) * (float) pow10;
            size_t           hash  = 7 * whole + (size_t) (7 * frac);
            return hash;
        }
    };

    template <> struct Hash<double> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const double& key) const -> size_t {
            // Calc hash for floats up to 10 digit precision
            double           num   = key;
            static const int pow10 = 1000000000;
            long             whole = (long) num;
            double           frac  = (num - (double) whole) * (double) pow10;
            size_t           hash  = 7 * whole + (size_t) (7 * frac);
            return hash;
        }
    };

    template <> struct Hash<long double> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const long double& key) const -> size_t {
            // Calc hash for floats up to 10 digit precision
            long double      num   = key;
            static const int pow10 = 1000000000;
            long             whole = (long) num;
            long double      frac  = (num - (long double) whole) * (long double) pow10;
            size_t           hash  = 7 * whole + (size_t) (7 * frac);
            return hash;
        }
    };

    template <> struct Hash<bool> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const bool& key) const -> size_t { return (size_t) key; }
    };

    template <> struct Hash<const char*> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const char* key) const -> size_t {
            size_t      size  = 0;
            const char* c_pos = key;
            while (*c_pos != 0) {
                c_pos++;
                size++;
            }

            size_t hash = 2383;
            for (size_t i = 0; i < size; i++) hash += 101 * key[i];
            return hash;
        }
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                               std::function Port
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    template <typename R, typename... A> class ICallable {
      public:
        virtual ~ICallable() = default;

        virtual auto operator()(A...) const -> R = 0;

        virtual void copy_to(void* dest) const = 0;

        virtual auto clone() const -> ICallable<R, A...>* = 0;
    };

    template <typename F, typename R, typename... A> class Callable : public ICallable<R, A...> {
        F _function;

      public:
        explicit Callable(F function) : _function(function) {}

        auto operator()(A... args) const -> R override { return _function(forward<A>(args)...); }

        void copy_to(void* destination) const override { new (destination) Callable(_function); }

        auto clone() const -> ICallable<R, A...>* override { return new Callable(_function); }
    };

    template <typename> class Function;

    template <typename R, typename... A> class Function<R(A...)> {
        static constexpr size_t STACK_LIMIT = 24;

        ICallable<R, A...>* _function;
        unsigned char       _stack_ptr[STACK_LIMIT];

      public:
        template <typename Func> Function(Func function) : _function(nullptr), _stack_ptr{} {
            if (sizeof(Callable<Func, R, A...>) <= STACK_LIMIT) {
                _function = (decltype(_function)) &_stack_ptr;
                new (_function) Callable<Func, R, A...>(function);
            } else {
                _function = new Callable<Func, R, A...>(function);
            }
        }

        ~Function() {
            if (_function == (decltype(_function)) &_stack_ptr) {
                _function->~ICallable();
            } else {
                delete _function;
            }
        }

        Function(const Function& other) : _function(nullptr), _stack_ptr() {
            if (other._function != nullptr) {
                if (other._function == (decltype(other._function)) &other._stack_ptr) {
                    _function = (decltype(_function)) &_stack_ptr;
                    other._function->copy_to(_function);
                } else {
                    _function = other._function->clone();
                }
            }
        }

        auto operator=(const Function& other) -> Function& {
            if (_function != nullptr) {
                if (_function == (decltype(_function)) &_stack_ptr) {
                    _function->~ICallable();
                } else {
                    delete _function;
                }
            }

            if (other._function != nullptr) {
                if (other._function == (decltype(other._function)) &other._stack_ptr) {
                    _function = (decltype(_function)) &_stack_ptr;
                    other._function->copy_to(_function);
                } else {
                    _function = other._function->clone();
                }
            } else {
                _function = nullptr;
            }
            return *this;
        }

        Function(Function&& other) noexcept : _function(nullptr), _stack_ptr() {
            if (other._function == (decltype(other._function)) &other._stack_ptr) {
                _function = (decltype(_function)) &_stack_ptr;
                other._function->copy_to(_function);
                other._function->~ICallable();
                other._function = nullptr;
            } else if (other._function) {
                swap(_function, other._function);
            }
        }

        auto operator=(Function&& other) noexcept -> Function& {
            if (_function != nullptr) {
                if (_function == (decltype(_function)) &_stack_ptr) {
                    _function->~ICallable();
                } else {
                    delete _function;
                }
            }

            if (other._function != nullptr) {
                if (other._function == (decltype(other._function)) &other._stack_ptr) {
                    _function = (decltype(_function)) &_stack_ptr;
                    other._function->copy_to(_function);
                    other._function->~ICallable();
                } else {
                    swap(_function, other._function);
                    // delete o._function; dont need -> points to old _function
                }
            } else {
                _function = nullptr;
            }
            other._function = nullptr;
            return *this;
        }

        auto operator()(A... args) const -> R { return (*_function)(forward<A>(args)...); }
    };
} // namespace Rune

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Initializer List
// initializer_list needs to be in the "std" namespace because this is a compiler requirement
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

namespace std {
    template <class _E> class initializer_list { // NOLINT
      public:
        typedef _E        value_type;      // NOLINT
        typedef const _E& reference;       // NOLINT
        typedef const _E& const_reference; // NOLINT
        typedef size_t    size_type;       // NOLINT
        typedef const _E* iterator;        // NOLINT
        typedef const _E* const_iterator;  // NOLINT

      private:
        iterator  _M_array; // NOLINT
        size_type _M_len;   // NOLINT

        // The compiler can call a private constructor.
        constexpr initializer_list(const_iterator __a, size_type __l) // NOLINT
            : _M_array(__a),
              _M_len(__l) {}

      public:
        constexpr initializer_list() noexcept : _M_array(0), _M_len(0) {}

        // Number of elements.
        constexpr size_type size() const noexcept { return _M_len; } // NOLINT

        // First element.
        constexpr const_iterator begin() const noexcept { return _M_array; } // NOLINT

        // One past the last element.
        constexpr const_iterator end() const noexcept { return begin() + size(); } // NOLINT
    };

    template <class _Tp>
    constexpr const _Tp* begin(initializer_list<_Tp> __ils) noexcept { // NOLINT
        return __ils.begin();
    }

    template <class _Tp> constexpr const _Tp* end(initializer_list<_Tp> __ils) noexcept { // NOLINT
        return __ils.end();
    }
} // namespace std

#endif // RUNEOS_UTILITY_H
