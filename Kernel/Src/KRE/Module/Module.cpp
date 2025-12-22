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

#include <KRE/System/Module.h>

namespace Rune {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Version
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto Version::to_string() const -> String {
        return pre_release.is_empty()
                   ? String::format("{}.{}.{}", major, minor, patch)
                   : String::format("{}.{}.{}-{}", major, minor, patch, pre_release);
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Kernel Subsystem
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    void Module::fire(const String& evt_hook, void* evt_context) {
        auto it = _event_hook_table.find(evt_hook);
        if (it == _event_hook_table.end()) return;

        for (auto& entry : *it->value) {
            entry.notified++;
            entry.handler(forward<void*>(evt_context));
        }
    }

    Module::Module() = default;

    auto Module::get_event_hook_table() -> LinkedList<EventHookTableEntry> {
        LinkedList<EventHookTableEntry> evt_hook_tbl;
        for (const auto& e : _event_hook_table) {
            EventHookTableEntry evt_hook_tbl_e;
            evt_hook_tbl_e.event_hook = *e.key; // NOLINT only end() will be null
            for (auto& ee : *e.value) {
                evt_hook_tbl_e.event_handler_table.add_back(
                    {.handle = ee.handle, .name = ee.name, .notified = ee.notified});
            }
            evt_hook_tbl.add_back(evt_hook_tbl_e);
        }

        return evt_hook_tbl;
    }

    auto Module::install_event_handler(const String&       event_hook,
                                       const String&       evt_handler_name,
                                       const EventHandler& handler) -> U16 {
        if (!_event_hook_handle_counter.has_more()) return 0;

        auto it = _event_hook_table.find(event_hook);
        if (it == _event_hook_table.end()) return 0;

        U16 evt_handler_id = _event_hook_handle_counter.acquire();
        it->value->add_back({.handle   = evt_handler_id,
                             .name     = evt_handler_name,
                             .notified = 0,
                             .handler  = handler});
        return evt_handler_id;
    }

    auto Module::uninstall_event_handler(const String& event_hook, U16 evt_handler_id) -> bool {
        auto it = _event_hook_table.find(event_hook);
        if (it == _event_hook_table.end()) return false;

        EventHandlerTableEntry to_remove;
        for (const auto& entry : *it->value) {
            if (entry.handle == evt_handler_id) {
                it->value->remove(entry);
                return true;
            }
        }
        return false;
    }
} // namespace Rune
