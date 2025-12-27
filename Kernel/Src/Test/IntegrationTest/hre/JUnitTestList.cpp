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

#include <Test/Heimdall/JUnitReporter.h>

#include <vector>

namespace Heimdall {
    struct JUnitTestList::JUnitTestListDetail {
        std::vector<JUnitTest> list;
    };

    JUnitTestList::JUnitTestList() : _list_detail(new JUnitTestList::JUnitTestListDetail()) {}

    JUnitTestList::~JUnitTestList() { delete _list_detail; }

    JUnitTestList::JUnitTestList(const JUnitTestList& other)
        : _list_detail(new JUnitTestListDetail{other._list_detail->list}) {}

    JUnitTestList::JUnitTestList(JUnitTestList&& other) noexcept
        : _list_detail(new(std::nothrow) JUnitTestListDetail{other._list_detail->list}) {
        delete other._list_detail;
        other._list_detail = nullptr;
    }

    auto JUnitTestList::operator=(const JUnitTestList& other) -> JUnitTestList& {
        if (this == &other) return *this;
        JUnitTestList tmp(other);
        swap(*this, tmp);
        return *this;
    }

    auto JUnitTestList::operator=(JUnitTestList&& other) noexcept -> JUnitTestList& {
        if (this == &other) return *this;
        JUnitTestList tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    void swap(JUnitTestList& fst, JUnitTestList& sec) noexcept {
        using std::swap;
        swap(fst._list_detail->list, sec._list_detail->list);
    }

    auto JUnitTestList::size() -> size_t { return _list_detail->list.size(); }

    void JUnitTestList::insert(const JUnitTest& test) { _list_detail->list.push_back(test); }

    auto JUnitTestList::operator[](size_t idx) -> JUnitTest { return _list_detail->list[idx]; }
} // namespace Heimdall
