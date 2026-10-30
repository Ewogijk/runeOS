//  Copyright 2026 Ewogijk
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef RUNEOS_TYPETRAITSTEST_H
#define RUNEOS_TYPETRAITSTEST_H

#include <KRE/String.h>
#include <KRE/TypeTraits.h>

using namespace Rune;

namespace TypeTraitsTest {
    struct PlainData {
        int         m_value;
        const char* m_name;
    };

    struct WithString {
        String m_name;
    };

    class WithVirtualDestructor {
      public:
        virtual ~WithVirtualDestructor() = default;
    };

    class WithUserCopy {
      public:
        WithUserCopy() = default;
        WithUserCopy(const WithUserCopy& other) { SILENCE_UNUSED(other) }
    };

    static_assert(is_trivially_copyable<int>::value);
    static_assert(is_trivially_copyable<PlainData>::value);
    static_assert(!is_trivially_copyable<WithString>::value);
    static_assert(!is_trivially_copyable<WithVirtualDestructor>::value);
    static_assert(!is_trivially_copyable<WithUserCopy>::value);

    static_assert(TriviallyCopyable<PlainData>);
    static_assert(!TriviallyCopyable<WithString>);
} // namespace TypeTraitsTest

#endif // RUNEOS_TYPETRAITSTEST_H
