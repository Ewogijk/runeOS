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

#include <KRE/System/EventHook.h>

namespace Rune {
    auto operator==(const EventHandlerTableEntry& one, const EventHandlerTableEntry& two) -> bool {
        return one.handle == two.handle;
    }

    auto operator!=(const EventHandlerTableEntry& one, const EventHandlerTableEntry& two) -> bool {
        return one.handle != two.handle;
    }

    void EventHookTableEntry::dump(const SharedPointer<TextStream>& stream) const {
        Table<EventHandlerStats, 2>::make_table(
            [](const EventHandlerStats stats) -> Array<String, 2> {
                return {String::format("{}-{}", stats.handle, stats.name),
                        String::format("{}", stats.notified)};
            })
            .with_headers({
            "ID-Name",
            "Notified",
            })
            .with_data(event_handler_table)
            .print(stream);
    }

    auto operator==(const EventHookTableEntry& one, const EventHookTableEntry& two) -> bool {
        return one.event_hook == two.event_hook;
    }

    auto operator!=(const EventHookTableEntry& one, const EventHookTableEntry& two) -> bool {
        return one.event_hook != two.event_hook;
    }
} // namespace Rune