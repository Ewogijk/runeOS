
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

#ifndef RUNEOS_MODULE_H
#define RUNEOS_MODULE_H

#include <Ember/Enum.h>

#include <KRE/Memory.h>

#include <KRE/Collections/HashMap.h>

#include <KRE/System/EventHook.h>
#include <KRE/System/FrameBuffer.h>
#include <KRE/System/Resource.h>

namespace Rune {
    /**
     * Information provided by the boot phase 1.
     */
    struct BootInfo {
        const char*  boot_loader_name    = "";
        const char*  boot_loader_version = "";
        MemoryMap    physical_memory_map = {};
        FrameBuffer  framebuffer;
        PhysicalAddr base_page_table_addr   = 0;
        U64          stack                  = 0;
        U8           physical_address_width = 0;
    };

    /**
     * @brief A version compliant to the <a href="https://semver.org/">semantic versioning</a>
     * scheme.
     */
    struct Version {
        U16    major       = 0;
        U16    minor       = 0;
        U16    patch       = 0;
        String pre_release = "";

        [[nodiscard]] auto to_string() const -> String;
    };

    /**
     * A kernel module is a major component of the kernel e.g. the memory management.
     */
    class Module {
      protected:
        HashMap<String, LinkedList<EventHandlerTableEntry>> _event_hook_table;            // NOLINT
        IDCounter<U16>                                      _event_hook_handle_counter{}; // NOLINT

        /**
         * @brief Fire an event for the given evtHook with the evtContext.
         * @param evt_hook
         * @param evt_context
         */
        void fire(const String& evt_hook, void* evt_context);

      public:
        explicit Module();

        virtual ~Module() = default;

        Module(const Module&)                    = delete;
        auto operator=(const Module&) -> Module& = delete;
        Module(Module&&)                         = delete;
        auto operator=(Module&&) -> Module&      = delete;

        /**
         *
         * @return Unique kernel module name.
         */
        [[nodiscard]] virtual auto get_name() const -> String = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief Load the kernel module and all registered plugins.
         *
         * <p>
         *  These are the steps of the start routine:
         *  <ol>
         *      <li>All supported event hooks are added to the event hook table.</li>
         *      <li>The implementation specific kernel module start routine.</li>
         *      <li>Start of all kernel plugins that are not running yet. This step succeeds if
         *          all kernel plugins start successfully.</li>
         *  </ol>
         * </p>
         *
         * @param boot_info    Information provided by the bootloader e.g. physical memory map.
         *
         * @return True: The module has loaded, False: It has not.
         */
        virtual auto load(const BootInfo& boot_info) -> bool = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                  Event Hook API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief The event hook table lists all event hooks and with their currently installed
         * event handlers.
         * @return The event hook table.
         */
        auto get_event_hook_table() -> LinkedList<EventHookTableEntry>;

        /**
         * @brief Try to install the given event handler on the requested event hook.
         *
         * If the event handler got successfully installed the assigned ID (< 0) will be returned if
         * the installation failed -1 is returned.
         *
         * @param event_hook
         * @param handler
         * @return ID > 0: The event handler will now receive events, ID == -1: The event handler
         * was not installed because the requested event hook is not supported.
         */
        auto install_event_handler(const String&       event_hook,
                                   const String&       evt_handler_name,
                                   const EventHandler& handler) -> U16;

        /**
         * @brief Try to uninstall the event handler with the given evtHandlerID from an event hook.
         * @param event_hook
         * @param evt_handler_id
         * @return True: The event handler will no longer receive events, False: The event handler
         * was not installed on this event hook or the event hook is not supported.
         */
        auto uninstall_event_handler(const String& event_hook, U16 evt_handler_id) -> bool;
    };
} // namespace Rune

#endif // RUNEOS_MODULE_H
