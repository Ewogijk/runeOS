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

#include <KernelRuntime/EventHook.h>

namespace Rune {
    bool operator==(const EventHandlerTableEntry& a, const EventHandlerTableEntry& b) {
        return a.handle == b.handle;
    }

    bool operator!=(const EventHandlerTableEntry& a, const EventHandlerTableEntry& b) {
        return a.handle != b.handle;
    }

    void EventHookTableEntry::dump(const SharedPointer<TextStream>& stream) const {
        auto it = event_handler_table.begin();
        formatter.dump(stream, [&it] {
            EventHandlerStats* i = nullptr;
            if (it.has_next()) {
                i = &(*it);
                ++it;
            }
            return i;
        });
    }

    bool operator==(const EventHookTableEntry& a, const EventHookTableEntry& b) {
        return a.event_hook == b.event_hook;
    }

    bool operator!=(const EventHookTableEntry& a, const EventHookTableEntry& b) {
        return a.event_hook != b.event_hook;
    }
} // namespace Rune