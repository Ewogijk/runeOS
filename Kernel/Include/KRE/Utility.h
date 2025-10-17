
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

#include <KRE/CppRuntimeSupport.h>
#include <KRE/TypeTraits.h>

namespace Rune {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      General Stuff
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    template <typename T>
    struct RemoveRef {
        using Type = T;
    };

    template <typename T>
    struct RemoveRef<T&> {
        using Type = T;
    };

    template <typename T>
    struct RemoveRef<T&&> {
        using Type = T;
    };

    /**
     * Implementation of std::move.
     *
     * @tparam T
     * @param param
     * @return
     */
    template <typename T>
    auto move(T&& param) -> typename RemoveRef<T>::Type&& { // NOLINT
        return static_cast<typename RemoveRef<T>::Type&&>(param);
    }

    /**
     * Implementation of std::forward.
     *
     * @tparam T
     * @param param
     * @return
     */
    template <typename T>
    constexpr auto forward(typename RemoveRef<T>::Type& param) -> T&& {
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
    template <typename T>
    void swap(T& one, T& two) noexcept {
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

    template <typename T>
    auto partition(T array[], int left, int high) -> int {
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

    template <typename T>
    void quick_sort(T array[], int low, int high) {
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
    template <typename T>
    void sort(T array[], const size_t arr_size) {
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
    template <typename T>
    void array_delete(T arr[], size_t idx, size_t& count) {
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
    template <Integer T>
    auto bit_check(T num, size_t offset) -> bool {
        return num >> offset & 1;
    }

    /**
     * Set the bit at offset and leave all other bits as they are.
     *
     * @tparam T     Number type.
     * @param num
     * @param offset Bit offset.
     * @return The number with the bit at offset set.
     */
    template <Integer T>
    auto bit_set(T num, const size_t offset) -> T {
        return num | 1 << offset;
    }

    /**
     * Clear the bit at offset and leave all other bits as they are.
     *
     * @tparam T     Number type.
     * @param num
     * @param offset Bit offset.
     * @return The number with the bit at offset cleared.
     */
    template <Integer T>
    auto bit_clear(T num, const size_t offset) -> T {
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
    template <class K>
    struct Hash;

    template <>
    struct Hash<signed char> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const signed char& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<char> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const char& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<short> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const short& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<int> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const int& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<long> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const long& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<long long> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const long long& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<unsigned char> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned char& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<unsigned short> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned short& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<unsigned int> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned int& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<unsigned long> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned long& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<unsigned long long> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const unsigned long long& key) const -> size_t { return key; }
    };

    template <>
    struct Hash<float> {

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

    template <>
    struct Hash<double> {

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

    template <>
    struct Hash<long double> {

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

    template <>
    struct Hash<bool> {

        auto operator=(Hash&& other) -> Hash& = default;

        auto operator=(const Hash& other) -> Hash& = default;

        auto operator()(const bool& key) const -> size_t { return (size_t) key; }
    };

    template <>
    struct Hash<const char*> {

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

    template <typename R, typename... A>
    class ICallable {
      public:
        virtual ~ICallable() = default;

        virtual auto operator()(A...) const -> R = 0;

        virtual void copy_to(void* dest) const = 0;

        virtual auto clone() const -> ICallable<R, A...>* = 0;
    };

    template <typename F, typename R, typename... A>
    class Callable : public ICallable<R, A...> {
        F _function;

      public:
        explicit Callable(F function) : _function(function) {}

        auto operator()(A... args) const -> R override { return _function(forward<A>(args)...); }

        void copy_to(void* destination) const override { new (destination) Callable(_function); }

        auto clone() const -> ICallable<R, A...>* override { return new Callable(_function); }
    };

    template <typename>
    class Function;

    template <typename R, typename... A>
    class Function<R(A...)> {
        static constexpr size_t STACK_LIMIT = 24;

        ICallable<R, A...>* _function;
        unsigned char       _stack_ptr[STACK_LIMIT];

      public:
        template <typename Func>
        Function(Func function) : _function(nullptr),
                                  _stack_ptr{} {
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

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  std::optional port
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * A dummy arg useful to differentiate between functions when variadic arguments are used.
     */
    struct Inplace {
        explicit Inplace() = default;
    };

    constexpr Inplace INPLACE{};

    /**
     * A tagging class for an empty optional.
     */
    struct NullOptional {
        constexpr explicit NullOptional(int num) { SILENCE_UNUSED(num) };
    };

    constexpr NullOptional NULL_OPT{0};

    /**
     * Optional manages a value that may or may not be present.
     *
     * @tparam T
     */
    template <typename T>
    class Optional {
        union {
            T _data;
        };
        bool _has_value = false;

      public:
        Optional() noexcept {};

        template <typename U>
        Optional(const U& obj) : _has_value(true) {
            new (&_data) T(obj);
        }

        Optional(const NullOptional& null_opt) noexcept {
            SILENCE_UNUSED(null_opt)
        }

        Optional(const Optional& other) : _has_value(other._has_value) {
            if (other._has_value) {
                new (&_data) T(other.value());
            }
        }

        Optional(Optional&& other) noexcept : _has_value(other._has_value) {
            if (other._has_value) {
                new (&_data) T(move(other.value()));
                other.reset();
            }
        }

        template <typename... Args>
        Optional(Inplace inplace, Args&&... args) : _has_value(true) {
            SILENCE_UNUSED(inplace);
            new (&_data) T(forward<Args>(args)...);
        }

        auto operator=(const Optional<T>& other) -> Optional<T>& {
            if (this == &other) return *this;
            Optional tmp(other);
            swap(tmp, *this);
            return *this;
        }

        auto operator=(Optional<T>&& other) noexcept -> Optional<T>& {
            if (this == &other) return *this;
            Optional tmp(move(other));
            swap(tmp, *this);
            return *this;
        }

        ~Optional() { reset(); }

        /**
         *
         * @return True: The optional contains a value, False: The optional contains no value.
         */
        constexpr explicit operator bool() { return _has_value; }

        /**
         *
         * @return True: The optional contains a value, False: The optional contains no value.
         */
        [[nodiscard]] constexpr auto has_value() const -> bool { return _has_value; }

        /**
         *
         * @return True: The optional contains a value, False: The optional contains no value.
         */
        [[nodiscard]] constexpr auto has_value() -> bool { return _has_value; }

        /**
         * If the optional does not contain a value, empty() == true, then the behavior is not
         * defined.
         * @return A reference to the contained value.
         */
        auto value() const -> const T& { return _data; }

        /**
         * If the optional does not contain a value, empty() == true, then the behavior is not
         * defined.
         * @return A reference to the contained value.
         */
        auto value() -> T& { return _data; }

        /**
         *
         * @param default_value A default value to be returned if the optional is empty.
         * @return A reference to the contained value or the default value.
         */
        auto value_or(T&& default_value) -> T& { return has_value() ? _data : default_value; }

        /**
         * Apply the value of this optional to the function. If the optional is empty, an empty
         * optional is returned.
         *
         * @tparam U Return type of the function.
         * @tparam F A callable returning Optional<U>.
         * @param func
         * @return If empty: An empty optional, else: The result of calling func.
         */
        template <class U, class F>
        auto and_then(F&& func) -> Optional<U> {
            return has_value() ? func(forward<T>(_data)) : Optional<U>();
        }

        /**
         * If the optional is empty, return the result of calling func. Otherwise, return this
         * optional.
         *
         * @tparam F A callable returning Optional<T>.
         * @param func
         * @return If empty: The result of calling func, else: An empty optional.
         */
        template <class F>
        auto or_else(F&& func) -> Optional<T> {
            return has_value() ? *this : func(forward<T>(_data));
        }

        /**
         * Apply the value of this optional to the function. If the optional is empty, an empty
         * optional is returned.
         *
         * @tparam U Return type of the function.
         * @tparam F A callable returning U.
         * @param func
         * @return If empty: The result of calling func wrapped in an optional,
         *          else: An empty optional.
         */
        template <typename U, typename F>
        auto transform(F&& func) -> Optional<U> {
            return has_value() ? Optional<U>(func(forward<T>(_data))) : Optional<U>();
        }

        /**
         * If the optional contains a value, call the value's destructor to empty the optional. If
         * the optional is already empty this function will do nothing.
         */
        void reset() noexcept {
            if (_has_value) {
                _data.~T();
                _has_value = false;
            }
        }

        friend void swap(Optional& fst, Optional& sec) noexcept {
            // Both optionals have no value -> nothing to do
            if (!fst._has_value && !sec._has_value) return;

            if (fst._has_value && sec._has_value) {
                // Both optionals have a value -> just swap
                using Rune::swap;
                swap(fst.value(), sec.value());
            } else {
                if (fst._has_value) {
                    // Move fst value to sec and reset fst
                    new (&sec._data) T(move(fst.value()));
                    sec._has_value = true;
                    fst.reset(); // sets fst._has_value = false
                } else {
                    // Move sec value to fst and reset sec
                    new (&fst._data) T(move(sec.value()));
                    fst._has_value = true;
                    sec.reset(); // sets sec._has_value = false
                }
            }
        }

        friend auto operator==(const Optional& fst, const Optional& sec) -> bool {
            if (fst.has_value() && sec.has_value()) return fst.value() == sec.value();
            if (fst.has_value() || sec.has_value()) return false;
            return false;
        }

        friend auto operator!=(const Optional& fst, const Optional& sec) -> bool {
            if (fst.has_value() && sec.has_value()) return fst.value() != sec.value();
            if (fst.has_value() || sec.has_value()) return true;
            return true;
        }
    };

    /**
     *
     * @param value
     * @return
     */
    template <typename T>
    auto make_optional(T&& value) -> Optional<T> {
        return Optional<T>(forward<T>(value));
    }

    /**
     *
     * @tparam Args
     * @param args
     * @return
     */
    template <typename T, typename... Args>
    auto make_optional(Args&&... args) -> Optional<T> {
        return Optional<T>(INPLACE, forward<Args>(args)...);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  std::expected Port
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * Tagging class to represent an unexpected value.
     */
    struct UnexpectTag {
        explicit UnexpectTag() = default;
    };

    constexpr UnexpectTag UNEXPECT{};

    /**
     * Convenience class to create Expected objects that contain an error.
     *
     * @tparam E Error type.
     */
    template <typename E>
    class Unexpected {
        E _error;

      public:
        constexpr explicit Unexpected(E&& error) : _error(error) {}

        template <typename... Args>
        constexpr explicit Unexpected(Inplace inplace, Args&&... args) {
            SILENCE_UNUSED(inplace);
            new (_error) E(forward(args)...);
        }

        constexpr Unexpected(const Unexpected& other) = default;
        constexpr Unexpected(Unexpected&& other)      = default;

        ~Unexpected() noexcept { _error.~E(); }

        auto operator=(const Unexpected& other) -> Unexpected& = default;
        auto operator=(Unexpected&& other) -> Unexpected&      = default;

        /**
         *
         * @return The error value.
         */
        constexpr auto error() & -> E& { return _error; }

        /**
         *
         * @return The error value.
         */
        constexpr auto error() const& -> const E& { return _error; }

        /**
         *
         * @return The error value.
         */
        constexpr auto error() && -> E&& { return _error; }

        /**
         *
         * @return The error value.
         */
        constexpr auto error() const&& -> const E&& { return _error; }

        friend void swap(Unexpected& fst, Unexpected& sec) noexcept {
            using Rune::swap;
            swap(fst._error, sec._error);
        }

        friend auto operator==(const Unexpected& fst, const Unexpected& sec) -> bool {
            return fst._error == sec._error;
        }

        friend auto operator!=(const Unexpected& fst, const Unexpected& sec) -> bool {
            return fst._error != sec._error;
        }
    };

    template <typename T, typename E>
    class Expected {
        union {
            T _value;
            E _error;
        };
        bool _has_value = false;

        void reset() {
            if (_has_value) {
                _value.~T();
            } else {
                _error.~E();
            }
            _has_value = false;
        }

      public:
        constexpr Expected() = default;

        template <typename U>
        constexpr Expected(const U& obj) : _has_value(true) {
            new (&_value) T(obj);
        }

        template <typename... Args>
        constexpr Expected(Inplace inplace, Args&&... args) : _has_value(true) {
            SILENCE_UNUSED(inplace);
            new (&_value) T(forward(args)...);
        }

        Expected(UnexpectTag unexpected) {}

        template <typename... Args>
        Expected(UnexpectTag unexpected, Args&&... args) {
            new (&_error) E(forward<Args>(args)...);
        }

        template <typename U>
        Expected(Unexpected<U>&& unexpected) {
            new (&_error) E(unexpected.error());
        }

        Expected(const Expected& other) : _has_value(other._has_value) {
            if (other._has_value) {
                new (&_value) T(other._value);
            } else {
                new (&_error) E(other._error);
            }
        }

        Expected(Expected&& other) noexcept : _has_value(other._has_value) {
            if (other._has_value) {
                new (&_value) T(other._value);
            } else {
                new (&_error) E(other._error);
            }
            other.reset();
        }

        ~Expected() noexcept { reset(); }

        auto operator=(const Expected& other) -> Expected& {
            if (this == &other) return *this;
            Expected tmp(other);
            swap(*this, tmp);
            return *this;
        }

        auto operator=(Expected&& other) noexcept -> Expected& {
            if (this == &other) return *this;
            Expected tmp(move(other));
            swap(*this, tmp);
            return *this;
        }

        /**
         *
         * @return True: The expected contains a value. False: The expected contains an error.
         */
        constexpr explicit operator bool() const noexcept { return _has_value; }

        /**
         *
         * @return True: The expected contains a value. False: The expected contains an error.
         */
        [[nodiscard]] constexpr auto has_value() noexcept -> bool { return _has_value; }

        /**
         *
         * @return True: The expected contains a value. False: The expected contains an error.
         */
        [[nodiscard]] constexpr auto has_value() const noexcept -> bool { return _has_value; }

        /**
         * If the expected does not contain a value the behavior is undefined.
         * @return A reference to the value.
         */
        constexpr auto value() -> T& { return _value; }

        /**
         * If the expected does not contain a value the behavior is undefined.
         * @return A reference to the value.
         */
        constexpr auto value() const -> const T& { return _value; }

        /**
         * Return the value of the expected or the default value if it does not have a value.
         * @param default_value A value to be returned when the expected does not contain a value.
         * @return A reference to the value.
         */
        constexpr auto value_or(T&& default_value) -> T& {
            return has_value() ? _value : default_value;
        }

        /**
         * If the expected does contain a value the behavior is undefined.
         * @return A reference to the error.
         */
        constexpr auto error() -> E& { return _error; }

        /**
         * If the expected does contain a value the behavior is undefined.
         * @return A reference to the error.
         */
        constexpr auto error() const -> const E& { return _error; }

        /**
         * Return the error of the expected or the default error if it does have a value.
         * @param default_error An error to be returned when the expected does contain a value.
         * @return A reference to the error.
         */
        constexpr auto error_or(E&& default_error) -> T& {
            return !has_value() ? _error : default_error;
        }

        /**
         * If this expected contains a value, invoke the function on the value. Otherwise, return an
         * expected with the error of this expected.
         * @tparam U Value type of the expected returned by the function.
         * @tparam F Type of the function.
         * @param func A function Expected<U, E>(T).
         * @return The return value of the function or an expected with this error value.
         */
        template <typename U, typename F>
        constexpr auto and_then(F&& func) -> Expected<U, E> {
            return has_value() ? func(forward<T>(_value)) : Expected<U, E>(UNEXPECT, _error);
        }

        /**
         * If this expected contains a value, invoke the function on the value. Otherwise, return an
         * expected with the error of this expected.
         * @tparam U Value type of the expected returned by the function.
         * @tparam F Type of the function.
         * @param func A function Expected<U, E>(T).
         * @return The return value of the function or an expected with this error value.
         */
        template <typename U, typename F>
        constexpr auto and_then(F&& func) const -> Expected<U, E> {
            return has_value() ? func(forward<T>(_value)) : Expected<U, E>(UNEXPECT, _error);
        }

        /**
         * If this expected contains an error, invoke the function with this error and return the
         * result. Otherwise, return this expected.
         * @tparam U Value type of the expected returned by the function.
         * @tparam F Type of the function.
         * @param func A function Expected<U, E>(E).
         * @return The return value of the function or this expected.
         */
        template <typename U, typename F>
        constexpr auto or_else(F&& func) -> Expected<U, E> {
            return has_value() ? *this : func(forward<E>(_error));
        }

        /**
         * If this expected contains an error, invoke the function with this error and return the
         * result. Otherwise, return this expected.
         * @tparam U Value type of the expected returned by the function.
         * @tparam F Type of the function.
         * @param func A function Expected<U, E>(E).
         * @return The return value of the function or this expected.
         */
        template <typename U, typename F>
        constexpr auto or_else(F&& func) const -> Expected<U, E> {
            return has_value() ? *this : func(forward<E>(_error));
        }

        /**
         * If this expected contains a value, invoke the function on the value and return the
         * result as an expected. Otherwise, return an expected with this error.
         * @tparam U Type of the value returned by the function.
         * @tparam F Type of the function.
         * @param func A function U(T).
         * @return The return value of the function or an expected with this error value.
         */
        template <typename U, typename F>
        constexpr auto transform(F&& func) const -> Expected<U, E> {
            return has_value() ? Expected<U, E>(func(forward<T>(_value)))
                               : Expected<U, E>(UNEXPECT, _error);
        }

        /**
         * If this expected contains a value, invoke the function on the value and return the
         * result as an expected. Otherwise, return an expected with this error.
         * @tparam U Type of the value returned by the function.
         * @tparam F Type of the function.
         * @param func A function U(T).
         * @return The return value of the function or an expected with this error value.
         */
        template <typename U, typename F>
        constexpr auto transform(F&& func) -> Expected<U, E> {
            return has_value() ? Expected<U, E>(func(forward<T>(_value)))
                               : Expected<U, E>(UNEXPECT, _error);
        }

        /**
         * If this expected contains an error, invoke the function on the error and return the
         * result as an expected. Otherwise, return an expected with this value.
         * @tparam U Ttype of the error returned by the function.
         * @tparam F Type of the function.
         * @param func A function U(E).
         * @return The return value of the function or an expected with this error value.
         */
        template <typename U, typename F>
        constexpr auto transform_error(F&& func) const -> Expected<T, U> {
            return has_value() ? Expected<T, U>(_value)
                               : Expected<T, U>(UNEXPECT, func(forward<E>(_error)));
        }

        /**
         * If this expected contains an error, invoke the function on the error and return the
         * result as an expected. Otherwise, return an expected with this value.
         * @tparam U Type of the error returned by the function.
         * @tparam F Type of the function.
         * @param func A function U(E).
         * @return The return value of the function or an expected with this error value.
         */
        template <typename U, typename F>
        constexpr auto transform_error(F&& func) -> Expected<T, U> {
            return has_value() ? Expected<T, U>(_value)
                               : Expected<T, U>(UNEXPECT, func(forward<E>(_error)));
        }

        friend void swap(Expected& fst, Expected& sec) noexcept {
            if (fst._has_value && sec._has_value) {
                using Rune::swap;
                swap(fst._value, sec._value);
            } else if (fst._has_value && !sec._has_value) {
                T tmp_val(move(fst._value));
                fst._value.~T();
                new (&fst._error) E(sec._error);
                sec._error.~E();
                new (&sec._value) T(tmp_val);

                fst._has_value = false;
                sec._has_value = true;
            } else if (!fst._has_value && sec._has_value) {
                E tmp_err(move(fst._error));
                fst._error.~E();
                new (&fst._value) T(sec._value);
                sec._value.~T();
                new (&sec._error) E(tmp_err);

                fst._has_value = true;
                sec._has_value = false;
            } else {
                using Rune::swap;
                swap(fst._error, sec._error);
            }
        }

        friend auto operator==(const Expected& fst, const Expected& sec) -> bool {
            if (fst.has_value() && sec.has_value()) return fst._value == sec._value;
            if (!fst.has_value() && !sec.has_value()) return fst._error == sec._error;
            return false;
        }

        friend auto operator!=(const Expected& fst, const Expected& sec) -> bool {
            if (fst.has_value() && sec.has_value()) return fst._value != sec._value;
            if (!fst.has_value() && !sec.has_value()) return fst._error != sec._error;
            return true;
        }
    };
} // namespace Rune

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Initializer List
// initializer_list needs to be in the "std" namespace because this is a compiler requirement
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

namespace std {
    template <class _E>
    class initializer_list { // NOLINT
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

    template <class _Tp>
    constexpr const _Tp* end(initializer_list<_Tp> __ils) noexcept { // NOLINT
        return __ils.end();
    }
} // namespace std

#endif // RUNEOS_UTILITY_H
