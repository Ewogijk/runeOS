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

#include <Test/Heimdall/HString.h>

#include <format>
#include <string>
#include <vector>

namespace Heimdall {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      String Wrapper
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    struct HString::StringDetail {
        std::string str;
    };

    HString::HString() : _str_detail(new StringDetail{std::string()}) {}
    HString::HString(const char* c_str) : _str_detail(new StringDetail{std::string(c_str)}) {}
    HString::~HString() { delete _str_detail; }

    HString::HString(const HString& other)
        : _str_detail(new StringDetail{other._str_detail->str}) {}

    HString::HString(HString&& other) noexcept
        : _str_detail(new(std::nothrow) StringDetail{other._str_detail->str}) {

        delete other._str_detail;
        other._str_detail = nullptr;
    }

    auto HString::operator=(const HString& other) -> HString& {
        if (this == &other) return *this;
        HString tmp(other);
        swap(*this, tmp);
        return *this;
    }

    auto HString::operator=(HString&& other) noexcept -> HString& {
        if (this == &other) return *this;
        HString tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    void swap(HString& fst, HString& sec) noexcept {
        using std::swap;
        swap(fst._str_detail->str, sec._str_detail->str);
    }

    auto HString::number_to_string(size_t count) -> HString {
        std::string num_str = std::to_string(count);
        return {num_str.c_str()};
    }

    auto HString::size() const -> size_t { return _str_detail->str.size(); }

    auto HString::is_empty() const -> bool { return _str_detail->str.empty(); }

    auto HString::operator+(const char* o) const -> HString {
        return {std::string(_str_detail->str + o).c_str()};
    }

    auto HString::operator+(const HString& o) const -> HString {
        return {std::string(_str_detail->str + o._str_detail->str).c_str()};
    }

    auto HString::operator+(const HString&& o) const -> HString {
        return {std::string(_str_detail->str + o._str_detail->str).c_str()};
    }

    auto HString::operator+(char o) const -> HString {
        return {std::string(_str_detail->str + o).c_str()};
    }

    auto HString::to_c_str() const -> const char* { return _str_detail->str.c_str(); }

    auto operator==(const HString& fst, const HString& sec) -> bool {
        return fst._str_detail->str == sec._str_detail->str;
    }
    auto operator!=(const HString& fst, const HString& sec) -> bool {
        return fst._str_detail->str != sec._str_detail->str;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  HString List
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    struct HStringList::HStringListDetail {
        std::vector<HString> list;
    };

    HStringList::HStringList() : _list_detail(new HStringList::HStringListDetail()) {}

    HStringList::~HStringList() { delete _list_detail; }

    HStringList::HStringList(const HStringList& other)
        : _list_detail(new HStringListDetail{other._list_detail->list}) {}

    HStringList::HStringList(HStringList&& other) noexcept
        : _list_detail(new(std::nothrow) HStringListDetail{other._list_detail->list}) {
        delete other._list_detail;
        other._list_detail = nullptr;
    }

    auto HStringList::operator=(const HStringList& other) -> HStringList& {
        if (this == &other) return *this;
        HStringList tmp(other);
        swap(*this, tmp);
        return *this;
    }

    auto HStringList::operator=(HStringList&& other) noexcept -> HStringList& {
        if (this == &other) return *this;
        HStringList tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    void swap(HStringList& fst, HStringList& sec) noexcept {
        using std::swap;
        swap(fst._list_detail->list, sec._list_detail->list);
    }

    auto HStringList::size() const -> size_t { return _list_detail->list.size(); }

    void HStringList::insert(const HString& str) {
        _list_detail->list.emplace_back(str.to_c_str());
    }

    auto HStringList::operator[](size_t index) const -> HString {
        return _list_detail->list[index];
    }
} // namespace Heimdall
