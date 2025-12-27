
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

#ifndef HEIMDALL_HSTRING_H
#define HEIMDALL_HSTRING_H

#include <stddef.h> // NOLINT cstddef is missing

namespace Heimdall {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      String Wrapper
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief A portable string implementation.
    /// Note: The class is part of the heimdall runtime environment (hre).
    class HString {
        struct StringDetail;
        StringDetail* _str_detail;

      public:
        HString();
        HString(const char* c_str); // NOLINT want implicit conversion from const char*
        ~HString();

        HString(const HString& other);
        HString(HString&& other) noexcept;
        auto operator=(const HString& other) -> HString&;
        auto operator=(HString&& other) noexcept -> HString&;

        friend void swap(HString& fst, HString& sec) noexcept;

        static auto number_to_string(size_t count) -> HString;

        [[nodiscard]] auto size() const -> size_t;
        [[nodiscard]] auto is_empty() const -> bool;

        auto operator+(const HString& o) const -> HString;

        auto operator+(const HString&& o) const -> HString;

        auto operator+(const char* o) const -> HString;

        auto operator+(char o) const -> HString;

        [[nodiscard]] auto to_c_str() const -> const char*;

        friend auto operator==(const HString& fst, const HString& sec) -> bool;
        friend auto operator!=(const HString& fst, const HString& sec) -> bool;
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                List Wrapper for HString
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /// @brief A portable list of strings.
    /// Note: The class is part of the heimdall runtime environment (hre).
    class HStringList {
        struct HStringListDetail;
        HStringListDetail* _list_detail;

      public:
        HStringList();
        ~HStringList();

        HStringList(const HStringList& other);
        HStringList(HStringList&& other) noexcept;
        auto operator=(const HStringList& other) -> HStringList&;
        auto operator=(HStringList&& other) noexcept -> HStringList&;

        friend void swap(HStringList& fst, HStringList& sec) noexcept;

        [[nodiscard]] auto size() const -> size_t;
        void               insert(const HString& str);
        auto               operator[](size_t index) const -> HString;
    };
} // namespace Heimdall

#endif // HEIMDALL_HSTRING_H
