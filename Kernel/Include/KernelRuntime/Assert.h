//  Copyright 2025 Ewogijk
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

#ifndef RUNEOS_ASSERT_H
#define RUNEOS_ASSERT_H

#include <KernelRuntime/Memory.h>
#include <KernelRuntime/Stream.h>
#include <KernelRuntime/String.h>

namespace Rune {
    /**
     * Configure assert statements to log to the given stream.
     * @param stream Text stream for assertion error messages.
     */
    void assert_configure(const SharedPointer<TextStream>& stream);

    /**
     * Assert that the condition is true, if not log given message and go into an endless loop.
     * @param condition Condition that must be true.
     * @param file      Name of the source file.
     * @param message   Error message to log if !condition.
     */
    void assert(bool condition, const String& file, const String& message);

    /**
     * Assert that the condition is true, if not go into an endless loop.
     * @param condition Condition that must be true.
     * @param file      Name of the source file.
     */
    void assert(bool condition, const String& file);
} // namespace Rune

#endif // RUNEOS_ASSERT_H
