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
    struct JUnitTestSuiteList::JUnitTestSuiteListDetail {
        std::vector<JUnitTestSuite> list;
    };

    JUnitTestSuiteList::JUnitTestSuiteList()
        : _list_detail(new JUnitTestSuiteList::JUnitTestSuiteListDetail()) {}

    JUnitTestSuiteList::~JUnitTestSuiteList() { delete _list_detail; }

    JUnitTestSuiteList::JUnitTestSuiteList(const JUnitTestSuiteList& other)
        : _list_detail(new JUnitTestSuiteListDetail{other._list_detail->list}) {}

    JUnitTestSuiteList::JUnitTestSuiteList(JUnitTestSuiteList&& other) noexcept
        : _list_detail(new JUnitTestSuiteListDetail{other._list_detail->list}) {
        delete other._list_detail;
        other._list_detail = nullptr;
    }

    auto JUnitTestSuiteList::operator=(const JUnitTestSuiteList& other) -> JUnitTestSuiteList& {
        if (this == &other) return *this;
        JUnitTestSuiteList tmp(other);
        swap(*this, tmp);
        return *this;
    }

    auto JUnitTestSuiteList::operator=(JUnitTestSuiteList&& other) noexcept -> JUnitTestSuiteList& {
        if (this == &other) return *this;
        JUnitTestSuiteList tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    void swap(JUnitTestSuiteList& fst, JUnitTestSuiteList& sec) noexcept {
        using std::swap;
        swap(fst._list_detail->list, sec._list_detail->list);
    }

    auto JUnitTestSuiteList::size() -> size_t { return _list_detail->list.size(); }

    void JUnitTestSuiteList::insert(const JUnitTestSuite& test) {
        _list_detail->list.push_back(test);
    }

    auto JUnitTestSuiteList::operator[](size_t idx) -> JUnitTestSuite {
        return _list_detail->list[idx];
    }
} // namespace Heimdall
