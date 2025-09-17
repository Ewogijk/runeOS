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

#ifndef RUNEOS_EVENTHOOK_H
#define RUNEOS_EVENTHOOK_H

#include <Ember/Ember.h>

#include <KernelRuntime/Resource.h>
#include <KernelRuntime/Stream.h>
#include <KernelRuntime/String.h>

namespace Rune {

    /**
     * @brief An event handler is installed on an event hook and whenever this event occurs it will
     * be called with some event context that is defined by the subsystem.
     */
    using EventHandler = Function<void(void*)>;

    /**
     * @brief General information about an event handler.
     */
    struct EventHandlerStats {
        U16    handle   = 0;
        String name     = "";
        U64    notified = 0;
    };

    /**
     * @brief A entry in the event handler table for an event hook.
     */
    struct EventHandlerTableEntry {
        U16          handle   = 0;
        String       name     = "";
        U64          notified = 0;
        EventHandler handler  = [](void* evt_ctx) { SILENCE_UNUSED(evt_ctx) };

        friend bool operator==(const EventHandlerTableEntry& a, const EventHandlerTableEntry& b);

        friend bool operator!=(const EventHandlerTableEntry& a, const EventHandlerTableEntry& b);
    };

    /**
     * @brief An entry in the event hook table with the stats of all installed event handlers.
     */
    struct EventHookTableEntry {
        String                            event_hook = "";
        LinkedList<EventHandlerStats>     event_handler_table;
        TableFormatter<EventHandlerStats> formatter;

        void dump(const SharedPointer<TextStream>& stream) const;

        friend bool operator==(const EventHookTableEntry& a, const EventHookTableEntry& b);

        friend bool operator!=(const EventHookTableEntry& a, const EventHookTableEntry& b);
    };
} // namespace Rune

#endif // RUNEOS_EVENTHOOK_H
