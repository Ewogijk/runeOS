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

#include <Test/Heimdall/Configuration.h>

#include <KRE/Collections/LinkedList.h>

namespace Heimdall {
    struct OptionList::OptionListDetail {
        Rune::LinkedList<Option> list;
    };

    OptionList::OptionList() : _list_detail(new OptionList::OptionListDetail()) {}

    OptionList::~OptionList() { delete _list_detail; }

    OptionList::OptionList(const OptionList& other)
        : _list_detail(new OptionListDetail{other._list_detail->list}) {}

    OptionList::OptionList(OptionList&& other) noexcept
        : _list_detail(new OptionListDetail{other._list_detail->list}) {
        delete other._list_detail;
        other._list_detail = nullptr;
    }

    auto OptionList::operator=(const OptionList& other) -> OptionList& {
        if (this == &other) return *this;
        OptionList tmp(other);
        swap(*this, tmp);
        return *this;
    }

    auto OptionList::operator=(OptionList&& other) noexcept -> OptionList& {
        if (this == &other) return *this;
        OptionList tmp(Rune::move(other));
        swap(*this, tmp);
        return *this;
    }

    void swap(OptionList& fst, OptionList& sec) {
        using Rune::swap;
        swap(fst._list_detail->list, sec._list_detail->list);
    }

    auto OptionList::size() const -> size_t { return _list_detail->list.size(); }

    void OptionList::insert(const Option& option) { _list_detail->list.add_back(option); }

    auto OptionList::operator[](size_t index) const -> Option {
        Option* t = _list_detail->list[index];
        return t ? *t : Option();
    }
}