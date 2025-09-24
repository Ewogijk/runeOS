
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

#ifndef RUNEOS_SUBSYSTEM_H
#define RUNEOS_SUBSYSTEM_H

#include <Ember/Enum.h>

#include <KRE/Logging.h>
#include <KRE/Memory.h>

#include <KRE/Collections/HashMap.h>

#include <KRE/System/FrameBuffer.h>
#include <KRE/System/Resource.h>
#include <KRE/System/EventHook.h>

namespace Rune {
    /**
     * Information provided by the low level boot routine to the high level boot routine.
     */
    struct BootLoaderInfo {
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

        [[nodiscard]]
        String to_string() const;
    };

    /**
     * A bigger part of the Kernel e.g. the Memory Management or Virtual FileSystem.
     */
    class Subsystem;

    /**
     * All kernel subsystems.
     */
#define K_SUBSYSTEMS(X)                                                                            \
    X(KernelSubsystem, MEMORY, 0x1)                                                                \
    X(KernelSubsystem, CPU, 0x2)                                                                   \
    X(KernelSubsystem, DEVICE, 0x3)                                                                \
    X(KernelSubsystem, VFS, 0x4)                                                                   \
    X(KernelSubsystem, APP, 0x5)                                                                   \
    X(KernelSubsystem, SYSTEMCALL, 0x6)

    DECLARE_ENUM(KernelSubsystem, K_SUBSYSTEMS, 0x0) // NOLINT

    class SubsystemRegistry {
        Subsystem** _k_subsys_registry; // Array of KernelSubsystem pointers
        size_t      _k_subsys_count;

      public:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //
        // Constructors&Destructors
        //
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        explicit SubsystemRegistry(Subsystem** k_subsys_registry, size_t k_subsys_count);

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //
        // Functions
        //
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @return Number of kernel subsystems in the registry.
         */
        [[nodiscard]]
        size_t size() const;

        /**
         *
         * @param index
         *
         * @return The kernel subsystem at index or null of the index is out of bounds.
         */
        Subsystem* operator[](size_t index) const;

        /**
         *
         * @tparam T
         * @param k_subsys
         * @return
         */
        template <typename T> T* get_as(KernelSubsystem k_subsys) const {
            return (T*) _k_subsys_registry[k_subsys.to_value() - 1];
        }
    };

    class Subsystem {
      protected:
        SharedPointer<Logger> _logger;

        HashMap<String, LinkedList<EventHandlerTableEntry>> _event_hook_table;
        HandleCounter<U16>                                  _event_hook_handle_counter;

        /**
         * @brief Fire an event for the given evtHook with the evtContext.
         * @param evt_hook
         * @param evt_context
         */
        void fire(const String& evt_hook, void* evt_context);

      public:
        explicit Subsystem();

        virtual ~Subsystem() = default;

        /**
         *
         * @return Unique Kernel Subsystem name.
         */
        [[nodiscard]]
        virtual String get_name() const = 0;

        /**
         *
         * @return Logger instance.
         */
        [[nodiscard]]
        SharedPointer<Logger> get_logger() const;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                              Functions
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief Start the Kernel SubSystem and all already registered Kernel Extensions.
         *
         * <p>
         *  These are the steps of the start routine:
         *  <ol>
         *      <li>All supported event hooks are added to the event hook table.</li>
         *      <li>The implementation specific Kernel Subsystem start routine.</li>
         *      <li>Start of all Kernel Extensions that are not running yet. This step succeeds if
         * all Kernel Extensions start successfully.</li>
         *  </ol>
         * </p>
         *
         * @param boot_info    Information provided by the bootloader e.g. physical memory map.
         * @param k_subsys_reg Gives access to other kernel subsystems.
         *
         * @return True: The subsystem has started, False: It has not.
         */
        virtual bool start(const BootLoaderInfo&    boot_info,
                           const SubsystemRegistry& k_subsys_reg) = 0;

        /**
         * Set the logger if no logger instance has been set yet.
         *
         * @param logger
         */
        virtual void set_logger(SharedPointer<Logger> logger) = 0;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Event Hook API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         * @brief The event hook table lists all event hooks and with their currently installed
         * event handlers.
         * @return The event hook table.
         */
        LinkedList<EventHookTableEntry> get_event_hook_table();

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
        U16 install_event_handler(const String&       event_hook,
                                  const String&       evt_handler_name,
                                  const EventHandler& handler);

        /**
         * @brief Try to uninstall the event handler with the given evtHandlerID from an event hook.
         * @param event_hook
         * @param evt_handler_id
         * @return True: The event handler will no longer receive events, False: The event handler
         * was not installed on this event hook or the event hook is not supported.
         */
        bool uninstall_event_handler(const String& event_hook, U16 evt_handler_id);
    };
} // namespace Rune

#endif // RUNEOS_SUBSYSTEM_H
