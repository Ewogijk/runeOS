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

#include <Test/Heimdall/Test.h>

#include <KRE/Collections/HashMap.h>
#include <KRE/String.h>

namespace Heimdall {

    struct TestTracker::DictDetail {
        Rune::HashMap<Rune::String, Rune::LinkedList<Test>> map;
    };

    TestTracker::TestTracker() : _dict_detail(new DictDetail()) {}

    TestTracker::~TestTracker() { delete _dict_detail; }

    TestTracker::TestTracker(const TestTracker& other)
        : _dict_detail(new DictDetail{other._dict_detail->map}) {}

    TestTracker::TestTracker(TestTracker&& other) noexcept
        : _dict_detail(new DictDetail{other._dict_detail->map}) { // NOLINT cannot throw
        delete other._dict_detail;
        other._dict_detail = nullptr;
    }

    auto TestTracker::operator=(const TestTracker& other) -> TestTracker& {
        if (this == &other) return *this;
        TestTracker tmp(other);
        swap(*this, tmp);
        return *this;
    }

    auto TestTracker::operator=(TestTracker&& other) noexcept -> TestTracker& {
        if (this == &other) return *this;
        TestTracker tmp(Rune::move(other));
        swap(*this, tmp);
        return *this;
    }

    void swap(TestTracker& fst, TestTracker& sec) noexcept {
        using Rune::swap;
        swap(fst._dict_detail->map, sec._dict_detail->map);
    }

    auto TestTracker::keys() const -> HStringList {
        HStringList result;
        for (auto entry : _dict_detail->map)
            result.insert(HString(entry.key->to_cstr())); // NOLINT is never null..
        return result;
    }

    auto TestTracker::find(const HString& test_suite) const -> TestList {
        auto maybe_list = _dict_detail->map.find(Rune::String(test_suite.to_c_str()));
        if (maybe_list == _dict_detail->map.end()) return {};
        TestList result;
        for (const auto& t : *maybe_list->value) result.insert(t);
        return result;
    }

    auto TestTracker::contains(const HString& test_suite) const -> bool {
        auto maybe_list = _dict_detail->map.find(Rune::String(test_suite.to_c_str()));
        return maybe_list != _dict_detail->map.end();
    }

    void TestTracker::create_test_suite(const HString& test_suite) {
        _dict_detail->map.put(Rune::String(test_suite.to_c_str()), Rune::LinkedList<Test>());
    }

    void TestTracker::insert_test(const HString& test_suite, const Test& test) {
        auto maybe_list = _dict_detail->map.find(Rune::String(test_suite.to_c_str()));
        if (maybe_list == _dict_detail->map.end()) return;
        maybe_list->value->add_back(test);
    }
} // namespace Heimdall