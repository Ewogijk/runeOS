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

#include <Test/Heimdall/TestTracker.h>

#include <vector>

namespace Heimdall {

    struct TestList::TestListDetail {
        std::vector<Test> list;
    };

    TestList::TestList() : _list_detail(new TestList::TestListDetail()) {}

    TestList::~TestList() { delete _list_detail; }

    TestList::TestList(const TestList& other)
        : _list_detail(new TestListDetail{other._list_detail->list}) {}

    TestList::TestList(TestList&& other) noexcept
        : _list_detail(new TestListDetail{other._list_detail->list}) {
        delete other._list_detail;
        other._list_detail = nullptr;
    }

    auto TestList::operator=(const TestList& other) -> TestList& {
        if (this == &other) return *this;
        TestList tmp(other);
        swap(*this, tmp);
        return *this;
    }

    auto TestList::operator=(TestList&& other) noexcept -> TestList& {
        if (this == &other) return *this;
        TestList tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    void swap(TestList& fst, TestList& sec) noexcept{
        using std::swap;
        swap(fst._list_detail->list, sec._list_detail->list);
    }

    auto TestList::size() const -> size_t { return _list_detail->list.size(); }

    void TestList::insert(const Test& test) { _list_detail->list.push_back(test); }

    auto TestList::operator[](size_t index) const -> Test {
        return _list_detail->list[index];
    }
} // namespace Heimdall
