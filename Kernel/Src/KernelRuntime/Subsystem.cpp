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

#include <KernelRuntime/Subsystem.h>


namespace Rune {
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Version
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    String Version::to_string() const {
        return pre_release.is_empty()
               ? String::format("{}.{}.{}", major, minor, patch)
               : String::format("{}.{}.{}-{}", major, minor, patch, pre_release);
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Kernel Subsystem Registry
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    DEFINE_ENUM(KernelSubsystem, K_SUBSYSTEMS, 0x0)


    SubsystemRegistry::SubsystemRegistry(Subsystem** k_subsys_registry, size_t k_subsys_count) :
            _k_subsys_registry(k_subsys_registry),
            _k_subsys_count(k_subsys_count) {

    }


    size_t SubsystemRegistry::size() const {
        return _k_subsys_count;
    }


    Subsystem* SubsystemRegistry::operator[](size_t index) const {
        return index < _k_subsys_count ? _k_subsys_registry[index] : nullptr;
    }


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          Kernel Subsystem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    void Subsystem::fire(const String& evt_hook, void* evt_context) {
        auto it = _event_hook_table.find(evt_hook);
        if (it == _event_hook_table.end())
            return;

        for (auto& entry: *it->value) {
            entry.notified++;
            entry.handler(forward<void*>(evt_context));
        }
    }


    Subsystem::Subsystem() :
            _logger(),
            _event_hook_table(),
            _event_hook_handle_counter() {

    }


    SharedPointer<Logger> Subsystem::get_logger() const {
        return _logger;
    }


    LinkedList<EventHookTableEntry> Subsystem::get_event_hook_table() {
        LinkedList<EventHookTableEntry> evt_hook_tbl;
        for (auto& e: _event_hook_table) {
            EventHookTableEntry evt_hook_tbl_e;
            evt_hook_tbl_e.event_hook = *e.key;

            LinkedList<Column<EventHandlerStats>> evt_h_cols;
            evt_h_cols.add_back(Column<EventHandlerStats>::make_handle_column_table(56));
            evt_h_cols.add_back(
                    {
                            "Notified",
                            10,
                            [](EventHandlerStats* stats) {
                                return String::format("{}", stats->notified);
                            }
                    }
            );
            evt_hook_tbl_e.formatter.configure(String::format("{} Event Hook", evt_hook_tbl_e.event_hook), evt_h_cols);

            for (auto& ee: *e.value) {
                evt_hook_tbl_e.event_handler_table.add_back({ ee.handle, ee.name, ee.notified });
            }
            evt_hook_tbl.add_back(evt_hook_tbl_e);
        }

        return evt_hook_tbl;
    }


    U16 Subsystem::install_event_handler(
            const String& event_hook,
            const String& evt_handler_name,
            const EventHandler& handler
    ) {
        if (!_event_hook_handle_counter.has_more_handles())
            return 0;

        auto it = _event_hook_table.find(event_hook);
        if (it == _event_hook_table.end())
            return 0;

        U16 evt_handler_id = _event_hook_handle_counter.acquire_handle();
        it->value->add_back({ evt_handler_id, evt_handler_name, 0, handler });
        return evt_handler_id;
    }


    bool Subsystem::uninstall_event_handler(const String& event_hook, U16 evt_handler_id) {
        auto it = _event_hook_table.find(event_hook);
        if (it == _event_hook_table.end())
            return false;

        EventHandlerTableEntry to_remove;
        for (const auto& entry: *it->value) {
            if (entry.handle == evt_handler_id) {
                it->value->remove(entry);
                return true;
            }
        }
        return false;
    }
}
