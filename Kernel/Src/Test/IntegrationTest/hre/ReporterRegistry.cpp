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

#include <Test/Heimdall/Reporter.h>

#include <memory>
#include <vector>

namespace Heimdall {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                  Reporter Registry
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    struct ReporterRegistry::ReporterListDetail {
        std::vector<std::shared_ptr<Reporter>> list;
    };

    ReporterRegistry::ReporterRegistry()
        : _list_detail(new ReporterRegistry::ReporterListDetail()) {}

    ReporterRegistry::~ReporterRegistry() { delete _list_detail; }

    ReporterRegistry::ReporterRegistry(const ReporterRegistry& other)
        : _list_detail(new ReporterListDetail{other._list_detail->list}) {}

    ReporterRegistry::ReporterRegistry(ReporterRegistry&& other) noexcept
        : _list_detail(new ReporterListDetail{other._list_detail->list}) {
        delete other._list_detail;
        other._list_detail = nullptr;
    }

    auto ReporterRegistry::operator=(const ReporterRegistry& other) -> ReporterRegistry& {
        if (this == &other) return *this;
        ReporterRegistry tmp(other);
        swap(*this, tmp);
        return *this;
    }

    auto ReporterRegistry::operator=(ReporterRegistry&& other) noexcept -> ReporterRegistry& {
        if (this == &other) return *this;
        ReporterRegistry tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    void swap(ReporterRegistry& fst, ReporterRegistry& sec) noexcept {
        using std::swap;
        swap(fst._list_detail->list, sec._list_detail->list);
    }

    auto ReporterRegistry::is_empty() const -> bool { return _list_detail->list.empty(); }

    auto ReporterRegistry::size() const -> size_t { return _list_detail->list.size(); }

    void ReporterRegistry::insert(Reporter* reporter) {
        _list_detail->list.push_back(std::shared_ptr<Reporter>(reporter));
    }

    auto ReporterRegistry::operator[](size_t index) const -> Reporter* {
        return _list_detail->list[index].get();
    }
} // namespace Heimdall
