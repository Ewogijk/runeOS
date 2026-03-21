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

#include <Ember/Ember.h>

#include <KRE/Collections/LinkedList.h>
#include <KRE/Logging.h>
#include <KRE/Math.h>
#include <KRE/System/System.h>

#include <Memory/MemoryModule.h>
#include <Memory/Paging.h>

#include <CPU/CPUModule.h>
#include <CPU/IO.h>
#include <CPU/Interrupt/Exception.h>

#include <Device/PCI.h>

CLINK {
#include <Device/ACPI/ACPICA/acpi.h>

    using namespace Rune;

    const SharedPointer<Logger> LOGGER = LogContext::instance().get_logger("Device.OSL");

    // Not implemented yet:
    // 1.  acpi_to_rune_get_line
    // 2.  acpi_to_rune_get_table_by_address
    // 3.  acpi_to_rune_get_table_by_index
    // 4.  acpi_to_rune_get_table_by_name
    // 5.  acpi_to_rune_redirect_output
    // 6.  acpi_to_rune_predefined_override
    // 7.  acpi_to_rune_table_override
    // 8. acpi_to_rune_physical_table_override

    // ========================================================================================== //
    // OSL Configuration
    // ========================================================================================== //

    /// @brief Stores the callback function, function context and thread start info.
    struct ACPIThreadContext {
        String         m_func_addr;
        String         m_ctx_addr;
        char*          m_argv[3]{};
        CPU::StartInfo m_start_info;
    };

    /// @brief Stores ACPICA and runeOS interrupt handler information.
    struct ACPIInterruptHandlerContext {
        U16              dev_handle;
        String           dev_name;
        ACPI_OSD_HANDLER handler;
        void*            context;

        friend bool operator==(const ACPIInterruptHandlerContext& lhs,
                               const ACPIInterruptHandlerContext& rhs) {
            return lhs.handler == rhs.handler;
        }
        friend bool operator!=(const ACPIInterruptHandlerContext& lhs,
                               const ACPIInterruptHandlerContext& rhs) {
            return lhs.handler != rhs.handler;
        }
    };

    /// @brief The configuration of the ACPI memory and threads.
    struct OSLConfig {
        /// @brief Maximum memory ACPI is allowed to use.
        static constexpr size_t ACPI_MEMORY_SIZE = 128 * MemoryUnit::MiB;

        /// @brief Total ACPI memory region.
        MemoryRegion m_acpi_memory = {};
        /// @brief All memory after this address is definitely unused.
        VirtualAddr m_acpi_memory_limit = 0;
        /// @brief List of gaps of unused memory regions that are smaller than the
        ///         m_acpi_memory_limit.
        LinkedList<MemoryRegion> m_acpi_memory_gaps;
        /// @brief References to all ACPI allocated object caches...
        LinkedList<Memory::ObjectCache*> m_acpi_caches;

        /// @brief Increasing counter for threads names, e.g. ACPI-Thread-0, ACPI-Thread-1, ...
        U64 m_thread_name_counter = 0;
        /// @brief List of all handles to threads that have been started by the ACPI subsystem but
        ///         not yet joined with. Associated threads may be running but may as well have
        ///         finished execution already.
        HashMap<CPU::ThreadHandle, ACPIThreadContext> m_acpi_threads;

        /// @brief Increasing counter for threads names, e.g. ACPI-Mutex-0, ACPI-Mutex-1, ...
        U64 m_mutex_name_counter = 0;
        /// @brief Stores recursion depth associated with the address of a mutex.
        ///
        /// ACPICA requires recursive mutexes which the kernel does not support, so the OSL must
        /// extend the current mutex to be recursive.
        HashMap<VirtualAddr, int> m_mutex_recursion_depth;

        /// @brief Increasing counter for threads names, e.g. ACPI-Semaphore-0, ACPI-Semaphore-1,
        ///         ...
        U64 m_semaphore_name_counter = 0;

        /// @brief Increasing counter for threads names, e.g. ACPI-Spinlock-0, ACPI-Spinlock-1, ...
        U64 m_spinlock_name_counter = 0;

        LinkedList<ACPIInterruptHandlerContext> m_interrupt_handlers;
        U16                                     m_interrupt_handler_counter = 0;
    };
    OSLConfig g_osl_config;

    // ========================================================================================== //
    // Environmental and ACPI Table Interfaces
    // ========================================================================================== //

    ACPI_STATUS acpi_to_rune_initialize() {
        auto* mem_module =
            System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        g_osl_config.m_acpi_memory       = {.start       = Memory::get_virtual_kernel_space_layout().acpi,
                                            .size        = OSLConfig::ACPI_MEMORY_SIZE,
                                            .memory_type = MemoryRegionType::ACPI};
        g_osl_config.m_acpi_memory_limit = g_osl_config.m_acpi_memory.start;

        g_osl_config.m_thread_name_counter = 0;

        if (!mem_module->get_virtual_memory_map().claim(g_osl_config.m_acpi_memory,
                                                        Memory::get_page_size()))
            return AE_ERROR;

        return AE_OK;
    }

    ACPI_STATUS acpi_to_rune_terminate() { return AE_OK; }

    ACPI_STATUS AcpiOsEnterSleep(UINT8 SleepState, UINT32 RegaValue, UINT32 RegbValue)
    {
        return AE_OK;
    }

    ACPI_PHYSICAL_ADDRESS acpi_to_rune_get_root_pointer() {
        return System::instance().get_boot_info().rsdp_addr;
    }

    ACPI_STATUS acpi_to_rune_predefined_override(const ACPI_PREDEFINED_NAMES* predefined_object,
                                                 ACPI_STRING*                 new_value) {
        return AE_NOT_IMPLEMENTED;
    }

    ACPI_STATUS acpi_to_rune_table_override(ACPI_TABLE_HEADER * existing_table,
                                            ACPI_TABLE_HEADER * *new_table) {
        return AE_NOT_IMPLEMENTED;
    }

    ACPI_STATUS acpi_to_rune_physical_table_override(ACPI_TABLE_HEADER * existing_table,
                                                     ACPI_PHYSICAL_ADDRESS * new_address,
                                                     UINT32 * new_table_length) {
        return AE_NOT_IMPLEMENTED;
    }

    // ========================================================================================== //
    // Memory Management
    // ========================================================================================== //

    auto check_acpi_memory_available(ACPI_SIZE req_memory) -> bool {
        return req_memory
               < abs(g_osl_config.m_acpi_memory.end() - g_osl_config.m_acpi_memory_limit);
    }

    ACPI_STATUS acpi_to_rune_create_cache(char*          cache_name,
                                          UINT16         object_size,
                                          UINT16         cache_depth,
                                          ACPI_CACHE_T** return_cache) {
        if (object_size < 16) return AE_BAD_PARAMETER;
        auto* mem_module =
            System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto* cache = mem_module->get_heap()->create_new_cache(object_size, 0, false);
        g_osl_config.m_acpi_caches.add_back(cache);
        *return_cache = reinterpret_cast<ACPI_CACHE_T*>(cache);
        return AE_OK;
    }

    ACPI_STATUS acpi_to_rune_delete_cache(ACPI_CACHE_T * cache) {
        if (cache == nullptr) return AE_BAD_PARAMETER;
        auto* mem_module =
            System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto* cache_ptr = reinterpret_cast<Memory::ObjectCache*>(cache);
        mem_module->get_heap()->destroy_cache(cache_ptr);
        g_osl_config.m_acpi_caches.add_back(cache_ptr);
        return AE_OK;
    }

    ACPI_STATUS acpi_to_rune_purge_cache(ACPI_CACHE_T * cache) {
        if (cache == nullptr) return AE_BAD_PARAMETER;
        reinterpret_cast<Memory::ObjectCache*>(cache)->purge();
        return AE_OK;
    }

    void* acpi_to_rune_acquire_object(ACPI_CACHE_T * cache) {
        if (cache == nullptr) return nullptr;
        auto* obj = reinterpret_cast<Memory::ObjectCache*>(cache)->allocate();
        if (obj != nullptr) {
            memset(obj, 0, reinterpret_cast<Memory::ObjectCache*>(cache)->get_object_size());
        }
        return obj;
    }

    ACPI_STATUS acpi_to_rune_release_object(ACPI_CACHE_T * cache, void* object) {
        if (cache == nullptr || object == nullptr) return AE_BAD_PARAMETER;
        reinterpret_cast<Memory::ObjectCache*>(cache)->free(object);
        return AE_OK;
    }

    void* acpi_to_rune_map_memory(ACPI_PHYSICAL_ADDRESS phys, ACPI_SIZE length) {
        auto* mem_module =
            System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto page_size    = Memory::get_page_size();
        auto aligned_size = memory_align(length, page_size, true);
        auto page_count   = aligned_size / page_size;

        // Check if enough ACPI memory is available. If yes, get a virtual address
        VirtualAddr v_addr              = 0x0;
        bool        memory_gap_selected = false;
        if (g_osl_config.m_acpi_memory_gaps.is_empty()) {
            // No free memory gaps -> Allocate at memory limit
            if (!check_acpi_memory_available(aligned_size)) return nullptr; // Out of virtual memory
            v_addr = g_osl_config.m_acpi_memory_limit;
        } else {
            // Search large enough memory gap
            MemoryRegion free_reg{};
            for (auto& reg : g_osl_config.m_acpi_memory_gaps) {
                if (reg.size <= aligned_size) {
                    free_reg = reg;
                    break;
                }
            }
            if (free_reg.size == 0) {
                // No memory region has enough memory available -> alloc at memory limit
                if (!check_acpi_memory_available(aligned_size))
                    return nullptr; // Out of virtual memory
                v_addr = g_osl_config.m_acpi_memory_limit;
            } else {
                // Use the memory gap
                v_addr              = free_reg.start;
                memory_gap_selected = true;
                g_osl_config.m_acpi_memory_gaps.remove(free_reg);
                if (free_reg.size < aligned_size) {
                    // Memory gap has leftover space -> Shrink it and add it again to memory gap
                    // list
                    free_reg.start += aligned_size;
                    free_reg.size  -= aligned_size;
                    g_osl_config.m_acpi_memory_gaps.add_back(free_reg);
                }
            }
        }

        // Request physical memory
        bool         allocate_page_frame = true;
        auto*        pmm                 = mem_module->get_physical_memory_manager();
        MemoryRegion req_region          = {
                     .start = phys,
                     .size  = aligned_size,
        };
        // ACPI will request reserved physical memory regions which the PMM does not allow to be
        // allocated and would be bad if done since it is firmware-initialized memory pointing to
        // say ACPI tables -> Thus, we do only page frame allocation on usable memory regions
        auto req_region_type = mem_module->get_physical_memory_map().find_type_of(req_region);
        if (req_region_type != MemoryRegionType::RESERVED
            && req_region_type != MemoryRegionType::NONE) {
            // The physical memory is not reserved -> Allocate a page frame
            if (!pmm->allocate_explicit(phys, page_count)) {
                if (memory_gap_selected)
                    // Selected memory gap is unused -> Add back to memory gaps list
                    g_osl_config.m_acpi_memory_gaps.add_back(
                        {.start = v_addr, .size = aligned_size});
                return nullptr;
            }
        }

        // Map read/write kernel memory. Should this be DMA memory?
        U16 page_flags = Memory::PageFlag::PRESENT | Memory::PageFlag::WRITE_ALLOWED;

        // Try to map the physical memory
        auto base_pt    = Memory::get_base_page_table();
        int  alloc_fail = 0;
        for (int i = 0; i < page_count; i++) {
            auto pta = Memory::allocate_page(base_pt,
                                             v_addr + (i * page_size),
                                             phys + (i * page_size),
                                             page_flags,
                                             pmm);
            if (pta.status != Memory::PageTableAccessStatus::OKAY) {
                alloc_fail = i;
                break;
            }
        }
        if (alloc_fail > 0) {
            for (int i = 0; i < alloc_fail; i++) {
                if (Memory::free_page(base_pt, v_addr + (i * page_size), pmm).status
                    != Memory::PageTableAccessStatus::OKAY) {
                    LOGGER->warn("Page free failed: {:#16x}", v_addr + (i * page_size));
                }
            }
            if (memory_gap_selected)
                // Selected memory gap is unused -> Add back to memory gaps list
                g_osl_config.m_acpi_memory_gaps.add_back({.start = v_addr, .size = aligned_size});
            return nullptr;
        }
        // The physical memory address is not guaranteed to be page frame aligned
        // -> Add page frame offset to the virtual address
        U16 page_frame_offset             = phys & 0xFFF;
        g_osl_config.m_acpi_memory_limit += aligned_size;
        return reinterpret_cast<void*>(v_addr + page_frame_offset);
    }

    void acpi_to_rune_unmap_memory(void* virt, ACPI_SIZE length) {
        // Memory address is not guaranteed to be page aligned
        // -> Align it first
        VirtualAddr v_addr =
            memory_align(memory_pointer_to_addr<void>(virt), Memory::get_page_size(), false);
        PhysicalAddr p_addr;
        if (!Memory::virtual_to_physical_address(v_addr, p_addr)) return;

        auto* mem_module =
            System::instance().get_module<Memory::MemoryModule>(ModuleSelector::MEMORY);
        auto* pmm          = mem_module->get_physical_memory_manager();
        auto  page_size    = Memory::get_page_size();
        auto  aligned_size = memory_align(length, page_size, true);
        auto  page_count   = aligned_size / page_size;
        auto  base_pt      = Memory::get_base_page_table();

        // Free virtual memory
        for (int i = 0; i < page_count; i++) {
            if (Memory::free_page(base_pt, v_addr + (i * page_size), pmm).status
                != Memory::PageTableAccessStatus::OKAY) {
                LOGGER->warn("Page free failed: {:0=#16x}", v_addr + (i * page_size));
            }
        }

        // Free physical memory
        MemoryRegion req_region = {
            .start = p_addr,
            .size  = aligned_size,
        };
        auto req_region_type = mem_module->get_physical_memory_map().find_type_of(req_region);
        if (req_region_type != MemoryRegionType::RESERVED
            && req_region_type != MemoryRegionType::NONE) {
            // Only free not reserved memory
            if (!mem_module->get_physical_memory_manager()->free(p_addr, page_count)) {
                LOGGER->warn("Page frame free failed: {:0=#16x}-{:0=#16x}",
                             p_addr,
                             p_addr + (page_count * page_size));
                return;
            }
        }
        // ACPICA will repeatedly map->unmap->map->... pages, hence different page frames will be
        // mapped to the same page. Therefore, flush the page from the TLB so that the old memory
        // mapping is not used in the future
        Memory::invalidate_page(v_addr);

        MemoryRegion free_reg{
            .start = v_addr,
            .size  = aligned_size,
        };
        if (free_reg.end() < g_osl_config.m_acpi_memory_limit)
            // free_reg forms a gab in the acpi memory -> add to memory gap list
            g_osl_config.m_acpi_memory_gaps.add_back(free_reg);
        else if (free_reg.end() == g_osl_config.m_acpi_memory_limit)
            // free_reg is the region right before the memory limit -> decrease memory limit
            g_osl_config.m_acpi_memory_limit = free_reg.start;
    }

    ACPI_STATUS acpi_to_rune_get_physical_address(void* virt, ACPI_PHYSICAL_ADDRESS* phys_out) {
        if (virt == nullptr || phys_out == nullptr) return AE_BAD_PARAMETER;

        PhysicalAddr phys;
        if (!Memory::virtual_to_physical_address(memory_pointer_to_addr<void>(virt), phys))
            return AE_ERROR;

        *phys_out = phys;
        return AE_OK;
    }

    void* acpi_to_rune_allocate(ACPI_SIZE size) {
        auto a = System::instance()
                     .get_module<Memory::MemoryModule>(ModuleSelector::MEMORY)
                     ->get_heap()
                     ->allocate(size);
        return a;
    }

    void acpi_to_rune_free(void* ptr) {
        System::instance()
            .get_module<Memory::MemoryModule>(ModuleSelector::MEMORY)
            ->get_heap()
            ->free(ptr);
    }

    BOOLEAN acpi_to_rune_readable(void* memory, ACPI_SIZE length) {
        auto page_size    = Memory::get_page_size();
        auto aligned_size = memory_align(length, page_size, true);
        auto page_count   = aligned_size / page_size;
        auto base_pt      = Memory::get_base_page_table();

        VirtualAddr v_addr = memory_pointer_to_addr<void>(memory);
        for (int i = 0; i < page_count; i++) {
            auto pta = Memory::find_page(base_pt, v_addr + (i * page_size));
            if (pta.status != Memory::PageTableAccessStatus::OKAY)
                // Virtual addr is not mapped aka not readable
                return false;
        }
        return true;
    }

    BOOLEAN acpi_to_rune_writable(void* memory, ACPI_SIZE length) {
        auto page_size    = Memory::get_page_size();
        auto aligned_size = memory_align(length, page_size, true);
        auto page_count   = aligned_size / page_size;
        auto base_pt      = Memory::get_base_page_table();

        VirtualAddr v_addr = memory_pointer_to_addr<void>(memory);
        for (int i = 0; i < page_count; i++) {
            auto pta = Memory::find_page(base_pt, v_addr + (i * page_size));
            if (pta.status != Memory::PageTableAccessStatus::OKAY)
                // Virtual addr is not mapped aka not readable
                return false;
            if (!pta.path[0].is_write_allowed()) return false;
        }
        return true;
    }

    // ========================================================================================== //
    // Multithreading and Scheduling Services
    // ========================================================================================== //

    ACPI_THREAD_ID acpi_to_rune_get_thread_id() {
        CPU::ThreadHandle h = System::instance()
                                  .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                                  ->get_scheduler()
                                  ->get_running_thread()
                                  ->get_handle();
        return static_cast<ACPI_THREAD_ID>(h);
    }

    ACPI_STATUS acpi_to_rune_execute(ACPI_EXECUTE_TYPE      type,
                                     ACPI_OSD_EXEC_CALLBACK func,
                                     void*                  ctx) {
        if (func == nullptr) return AE_BAD_PARAMETER;

        static constexpr uintptr_t RADIX_HEX = 16;
        ACPIThreadContext          t_ctx;
        // Provide the addresses to func and ctx as hex strings to thread main
        t_ctx.m_func_addr = int_to_string(memory_pointer_to_addr(func), RADIX_HEX);
        t_ctx.m_ctx_addr  = int_to_string(memory_pointer_to_addr(ctx), RADIX_HEX);

        t_ctx.m_argv[0]         = const_cast<char*>(t_ctx.m_func_addr.to_cstr());
        t_ctx.m_argv[1]         = const_cast<char*>(t_ctx.m_ctx_addr.to_cstr());
        t_ctx.m_argv[2]         = nullptr;
        t_ctx.m_start_info.argc = 2;
        t_ctx.m_start_info.argv = t_ctx.m_argv;
        t_ctx.m_start_info.main = [](CPU::StartInfo* start_info) -> int {
            // Parse the hex string addresses of func and ctx and cast to their respective types
            uintptr_t ptr = 0;
            if (!parse_int<uintptr_t>(start_info->argv[0], RADIX_HEX, ptr)) return -1;
            auto func = reinterpret_cast<ACPI_OSD_EXEC_CALLBACK>(ptr);

            if (!parse_int<uintptr_t>(start_info->argv[1], RADIX_HEX, ptr)) return -1;
            auto* ctx = reinterpret_cast<void*>(ptr);

            func(ctx);
            return 0;
        };

        CPU::ThreadHandle h =
            System::instance()
                .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                ->schedule_new_thread(
                    String::format("ACPI-Thread-{}", g_osl_config.m_thread_name_counter++),
                    &t_ctx.m_start_info,
                    Memory::get_base_page_table_address(),
                    CPU::SchedulingPolicy::LOW_LATENCY,
                    {});
        if (h == Resource<CPU::ThreadHandle>::HANDLE_NONE) return AE_BAD_PARAMETER;

        g_osl_config.m_acpi_threads.put(h, t_ctx);
        return AE_OK;
    }

    void acpi_to_rune_sleep(UINT64 ms) {
        System::instance()
            .get_module<CPU::CPUModule>(ModuleSelector::CPU)
            ->get_system_timer()
            ->sleep_milli(ms);
    }

    void acpi_to_rune_stall(UINT64 us) {
        System::instance()
            .get_module<CPU::CPUModule>(ModuleSelector::CPU)
            ->get_system_timer()
            ->stall_micro(us);
    }

    void acpi_to_rune_wait_events_complete() {
        auto* cpu_module = System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU);
        for (auto& started_threads : g_osl_config.m_acpi_threads)
            cpu_module->join_thread(*started_threads.key);

        // At this point all threads have finished execution or had already finished
        g_osl_config.m_acpi_threads.clear();
    }

    // ========================================================================================== //
    // Mutual Exclusion and Synchronization
    // ========================================================================================== //

    ACPI_STATUS acpi_to_rune_mutex_create(ACPI_MUTEX * out_handle) {
        if (out_handle == nullptr) return AE_BAD_PARAMETER;

        auto m = System::instance()
                     .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                     ->create_mutex(
                         String::format("ACPI-Mutex-{}", g_osl_config.m_mutex_name_counter++));
        if (!m) return AE_NO_MEMORY;
        g_osl_config.m_mutex_recursion_depth.put(memory_pointer_to_addr(m.get()), 0);
        *out_handle = m.get();
        return AE_OK;
    }

    void acpi_to_rune_mutex_delete(ACPI_MUTEX mutex) {
        System::instance()
            .get_module<CPU::CPUModule>(ModuleSelector::CPU)
            ->release_mutex(reinterpret_cast<CPU::Mutex*>(mutex)->get_handle());
        g_osl_config.m_mutex_recursion_depth.remove(memory_pointer_to_addr(mutex));
    }

    ACPI_STATUS acpi_to_rune_mutex_acquire(ACPI_MUTEX mutex, UINT16 timeout_ms) {
        if (mutex == nullptr) return AE_BAD_PARAMETER;
        auto* m = reinterpret_cast<CPU::Mutex*>(mutex);

        if (m->get_owner() != nullptr
            && m->get_owner()->get_handle() == acpi_to_rune_get_thread_id()) {
            // Mutex is already locked
            g_osl_config.m_mutex_recursion_depth[memory_pointer_to_addr(mutex)]++;
            return AE_OK;
        }
        if (timeout_ms == 0xFFFF) {
            // Lock without timeout
            m->lock();
            g_osl_config.m_mutex_recursion_depth[memory_pointer_to_addr(mutex)] = 1;
            return AE_OK;
        }
        auto* timer =
            System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU)->get_system_timer();
        constexpr U32 MILLI_TO_NANO = 1000000;
        U64           end           = timer->get_time_since_start() + (MILLI_TO_NANO * timeout_ms);
        bool          locked        = false;
        while (timer->get_time_since_start() < end) {
            locked = m->try_lock();
            if (locked) break;
        }
        if (locked) g_osl_config.m_mutex_recursion_depth[memory_pointer_to_addr(mutex)] = 1;
        return timer->get_time_since_start() >= end ? AE_TIME : AE_OK;
    }

    void acpi_to_rune_mutex_release(ACPI_MUTEX mutex) {
        if (mutex) {
            if (g_osl_config.m_mutex_recursion_depth[memory_pointer_to_addr(mutex)] == 0) return;

            g_osl_config.m_mutex_recursion_depth[memory_pointer_to_addr(mutex)]--;

            if (g_osl_config.m_mutex_recursion_depth[memory_pointer_to_addr(mutex)] == 0)
                reinterpret_cast<CPU::Mutex*>(mutex)->unlock();
        }
    }

    ACPI_STATUS acpi_to_rune_semaphore_create(UINT32 max_units,
                                              UINT32 initial_units,
                                              ACPI_SEMAPHORE * out_handle) {
        if (out_handle == nullptr || initial_units > max_units) return AE_BAD_PARAMETER;

        auto sem = System::instance()
                       .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                       ->create_semaphore(String::format("ACPI-Semaphore-{}",
                                                         g_osl_config.m_semaphore_name_counter++),
                                          initial_units,
                                          max_units);
        if (!sem) return AE_NO_MEMORY;
        *out_handle = sem.get();
        return AE_OK;
    }

    ACPI_STATUS acpi_to_rune_semaphore_delete(ACPI_SEMAPHORE sem) {
        return System::instance()
                       .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                       ->free_semaphore(reinterpret_cast<CPU::Semaphore*>(sem)->get_handle())
                   ? AE_OK
                   : AE_BAD_PARAMETER;
    }

    ACPI_STATUS acpi_to_rune_semaphore_wait(ACPI_SEMAPHORE sem, UINT32 units, UINT16 timeout_ms) {
        if (sem == nullptr) return AE_BAD_PARAMETER;
        auto*  s       = reinterpret_cast<CPU::Semaphore*>(sem);
        UINT32 c_units = 0; // Currently acquired units
        if (timeout_ms == 0xFFFF) {
            // Lock without timeout
            while (c_units < units) {
                s->lock();
                c_units++;
            }
            return AE_OK;
        }
        if (timeout_ms == 0) {
            // Need explicit timeout_ms = 0 support
            // Lock success -> AE_OK, Lock fail -> AE_TIME
            while (c_units < units) {
                if (!s->try_lock()) return AE_TIME;
                c_units++;
            }
            return AE_OK;
        }
        // Try lock with arbitrary
        auto* timer =
            System::instance().get_module<CPU::CPUModule>(ModuleSelector::CPU)->get_system_timer();
        constexpr U32 MILLI_TO_NANO = 1000000;
        U64           end           = timer->get_time_since_start() + (MILLI_TO_NANO * timeout_ms);
        while (timer->get_time_since_start() < end) {
            if (s->try_lock()) {
                c_units++;
                if (c_units >= units) break;
            }
        }
        return c_units < units ? AE_TIME : AE_OK;
    }

    ACPI_STATUS acpi_to_rune_semaphore_signal(ACPI_SEMAPHORE sem, UINT32 units) {
        if (sem == nullptr) return AE_BAD_PARAMETER;
        auto*  s       = reinterpret_cast<CPU::Semaphore*>(sem);
        UINT32 c_units = 0;
        while (c_units < units) {
            if (s->get_available_units() >= s->get_unit_max()) return AE_LIMIT;
            s->unlock();
            c_units++;
        }
        return AE_OK;
    }

    ACPI_STATUS acpi_to_rune_spinlock_create(ACPI_SPINLOCK * out_handle) {
        if (out_handle == nullptr) return AE_BAD_PARAMETER;

        auto sp = System::instance()
                      .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                      ->create_spinlock(String::format("ACPI-Spinlock-{}",
                                                       g_osl_config.m_spinlock_name_counter++));
        if (!sp) return AE_NO_MEMORY;
        *out_handle = sp.get();
        return AE_OK;
    }

    void acpi_to_rune_spinlock_delete(ACPI_SPINLOCK lock) {
        System::instance()
            .get_module<CPU::CPUModule>(ModuleSelector::CPU)
            ->free_spinlock(reinterpret_cast<CPU::Spinlock*>(lock)->get_handle());
    }

    ACPI_CPU_FLAGS acpi_to_rune_spinlock_acquire(ACPI_SPINLOCK lock) {
        return reinterpret_cast<CPU::Spinlock*>(lock)->lock_safe();
    }

    void acpi_to_rune_spinlock_release(ACPI_SPINLOCK lock, ACPI_CPU_FLAGS flags) {
        reinterpret_cast<CPU::Spinlock*>(lock)->unlock_safe(flags);
    }

    // ========================================================================================== //
    // Interrupt Handling
    // ========================================================================================== //

    ACPI_STATUS acpi_to_rune_install_interrupt_handler(UINT32           irq,
                                                       ACPI_OSD_HANDLER handler,
                                                       void*            ctx) {
        if (handler == nullptr || irq >= CPU::irq_get_line_limit()) return AE_BAD_PARAMETER;
        U16                         dev_handle = g_osl_config.m_interrupt_handler_counter++;
        ACPIInterruptHandlerContext int_ctx{.dev_handle = dev_handle,
                                            .dev_name   = String::format("GSI-{}", dev_handle),
                                            .handler    = handler,
                                            .context    = ctx};

        bool registered =
            System::instance()
                .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                ->install_irq_handler(irq, int_ctx.dev_handle, int_ctx.dev_name, [&handler, &ctx] {
                    int ret = handler(ctx);
                    return ret == ACPI_INTERRUPT_HANDLED ? CPU::IRQState::HANDLED
                                                         : CPU::IRQState::PENDING;
                });

        if (!registered) return AE_ALREADY_EXISTS;
        g_osl_config.m_interrupt_handlers.add_back(int_ctx);
        return AE_OK;
    }

    ACPI_STATUS acpi_to_rune_remove_interrupt_handler(UINT32 irq, ACPI_OSD_HANDLER handler) {
        if (handler == nullptr || irq >= CPU::irq_get_line_limit()) return AE_BAD_PARAMETER;

        ACPIInterruptHandlerContext* int_ctx = nullptr;
        for (auto& ctx : g_osl_config.m_interrupt_handlers) {
            if (ctx.handler == handler) {
                int_ctx = &ctx;
                break;
            }
        }
        if (int_ctx == nullptr) return AE_NOT_EXIST;
        bool unregistered = System::instance()
                                .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                                ->uninstall_irq_handler(irq, int_ctx->dev_handle);
        if (!unregistered) return AE_NOT_EXIST;
        g_osl_config.m_interrupt_handlers.remove(*int_ctx);
        return AE_OK;
    }

    // ========================================================================================== //
    // Memory Access and Memory Mapped I/O
    // ========================================================================================== //

    extern ACPI_STATUS acpi_to_rune_read_memory(ACPI_PHYSICAL_ADDRESS addr,
                                                UINT64 * val,
                                                UINT32 width) {
        if (addr == 0x0 || val == nullptr
            || (width != 8 && width != 16 && width != 32 && width != 64))
            return AE_BAD_PARAMETER;

        VirtualAddr v_addr = Memory::physical_to_virtual_address(addr);
        if (width == 8)
            *val = *memory_addr_to_pointer<U8>(v_addr);
        else if (width == 16)
            *val = *memory_addr_to_pointer<U16>(v_addr);
        else if (width == 32)
            *val = *memory_addr_to_pointer<U32>(v_addr);
        else if (width == 64)
            *val = *memory_addr_to_pointer<U64>(v_addr);
        return AE_OK;
    }

    extern ACPI_STATUS acpi_to_rune_write_memory(ACPI_PHYSICAL_ADDRESS addr,
                                                 UINT64                val,
                                                 UINT32                width) {
        if (addr == 0x0 || (width != 8 && width != 16 && width != 32 && width != 64))
            return AE_BAD_PARAMETER;

        VirtualAddr v_addr = Memory::physical_to_virtual_address(addr);
        if (width == 8)
            *memory_addr_to_pointer<U8>(v_addr) = val;
        else if (width == 16)
            *memory_addr_to_pointer<U16>(v_addr) = val;
        else if (width == 32)
            *memory_addr_to_pointer<U32>(v_addr) = val;
        else if (width == 64)
            *memory_addr_to_pointer<U64>(v_addr) = val;
        return AE_OK;
    }

    // ========================================================================================== //
    // Port Input/Output
    // ========================================================================================== //

    extern ACPI_STATUS acpi_to_rune_read_port(ACPI_IO_ADDRESS port, UINT32 * val, UINT32 width) {
        if (val == nullptr || (width != 8 && width != 16 && width != 32)) return AE_BAD_PARAMETER;
        if (width == 8)
            *val = CPU::in_b(port);
        else if (width == 16)
            *val = CPU::in_w(port);
        else
            *val = CPU::in_dw(port);
        return AE_OK;
    }

    extern ACPI_STATUS acpi_to_rune_write_port(ACPI_IO_ADDRESS port, UINT32 val, UINT32 width) {
        if (width != 8 && width != 16 && width != 32) return AE_BAD_PARAMETER;
        if (width == 8)
            CPU::out_b(port, static_cast<U8>(val));
        else if (width == 16)
            CPU::out_w(port, static_cast<U16>(val));
        else
            CPU::out_dw(port, static_cast<U32>(val));
        return AE_OK;
    }

    // ========================================================================================== //
    // PCI Configuration Space Access
    // ========================================================================================== //

    ACPI_STATUS
    acpi_to_rune_read_pci_config(ACPI_PCI_ID pci_id, UINT32 reg, UINT64 * val, UINT32 width) {
        if (val == nullptr || (width != 8 && width != 16 && width != 32 && width != 64))
            return AE_BAD_PARAMETER;
        if (reg > (0x100 - (width / 8)))
            // Reading the PCI configuration space with the I/O port method only supports 256 bytes
            // -> Report error because we cannot serve this request
            return AE_ERROR;

        if (width == 8)
            *val = Device::PCI::read_byte(pci_id.Bus, pci_id.Device, pci_id.Function, reg);
        else if (width == 16)
            *val = Device::PCI::read_word(pci_id.Bus, pci_id.Device, pci_id.Function, reg);
        else if (width == 32)
            *val = Device::PCI::read_dword(pci_id.Bus, pci_id.Device, pci_id.Function, reg);
        else {
            *val = static_cast<U64>(
                       Device::PCI::read_dword(pci_id.Bus, pci_id.Device, pci_id.Function, reg + 4))
                       << SHIFT_32
                   | Device::PCI::read_dword(pci_id.Bus, pci_id.Device, pci_id.Function, reg);
        }
        return AE_OK;
    }

    ACPI_STATUS acpi_to_rune_write_pci_config(ACPI_PCI_ID pci_id,
                                              UINT32      reg,
                                              UINT64      val,
                                              UINT32      width) {
        if (width != 8 && width != 16 && width != 32 && width != 64) return AE_BAD_PARAMETER;
        if (reg > (0x100 - (width / 8)))
            // Writing the PCI configuration space with the I/O port method only supports 256 bytes
            // -> Report error because we cannot serve this request
            return AE_ERROR;

        if (width == 8)
            Device::PCI::write_byte(pci_id.Bus, pci_id.Device, pci_id.Function, reg, val);
        else if (width == 16)
            Device::PCI::write_word(pci_id.Bus, pci_id.Device, pci_id.Function, reg, val);
        else if (width == 32)
            Device::PCI::write_dword(pci_id.Bus, pci_id.Device, pci_id.Function, reg, val);
        else {
            Device::PCI::write_dword(pci_id.Bus,
                                     pci_id.Device,
                                     pci_id.Function,
                                     reg + 4,
                                     dword_get(val, 1));
            Device::PCI::write_dword(pci_id.Bus,
                                     pci_id.Device,
                                     pci_id.Function,
                                     reg,
                                     dword_get(val, 0));
        }
        return AE_OK;
    }

    // ========================================================================================== //
    // Formatted Output
    // ========================================================================================== //

    void acpi_to_rune_vprintf(const char* format, va_list args) {
        constexpr size_t MAX_ARGS = 16;
        Argument         f_args[MAX_ARGS];
        size_t           arg_count = 0;
        String           c_fstr(format);
        String           r_fstr("");

        for (int i = 0; i < (int) c_fstr.size(); i++) {
            char c = c_fstr[i];
            if (c != '%') {
                r_fstr += c;
                continue;
            }

            c = c_fstr[++i];

            // --- Flags ---
            // '#' flag: use kernel prefix notation
            bool alt_form = false;
            // '0' flag: zero-pad with '=' alignment
            bool zero_pad = false;
            // '-' flag: left-align (kernel '<')
            bool left_align = false;

            bool parsing_flags = true;
            while (parsing_flags) {
                switch (c) {
                    case '#':
                        alt_form = true;
                        c        = c_fstr[++i];
                        break;
                    case '0':
                        zero_pad = true;
                        c        = c_fstr[++i];
                        break;
                    case '-':
                        left_align = true;
                        c          = c_fstr[++i];
                        break;
                    case '+': /* not supported, skip */ c = c_fstr[++i]; break;
                    case ' ': /* not supported, skip */ c = c_fstr[++i]; break;
                    default:  parsing_flags = false; break;
                }
            }

            // --- Width ---
            String width_str("");
            while (c >= '0' && c <= '9') {
                width_str += c;
                c          = c_fstr[++i];
            }

            // --- Precision ---
            String precision_str("");
            if (c == '.') {
                c = c_fstr[++i];
                while (c >= '0' && c <= '9') {
                    precision_str += c;
                    c              = c_fstr[++i];
                }
            }

            // --- Length modifier ---
            // hh -> signed char / unsigned char
            // h  -> short / unsigned short
            // l  -> long / unsigned long
            // ll -> long long / unsigned long long
            // L  -> long double
            enum class Length { NONE, HH, H, L, LL, LD };
            Length length = Length::NONE;
            if (c == 'h') {
                c = c_fstr[++i];
                if (c == 'h') {
                    length = Length::HH;
                    c      = c_fstr[++i];
                } else {
                    length = Length::H;
                }
            } else if (c == 'l') {
                c = c_fstr[++i];
                if (c == 'l') {
                    length = Length::LL;
                    c      = c_fstr[++i];
                } else {
                    length = Length::L;
                }
            } else if (c == 'L') {
                length = Length::LD;
                c      = c_fstr[++i];
            }

            // --- Build the kernel format placeholder ---
            // {:<fill><align><#><width>.<precision><type>}
            // We only emit parts that are actually specified.
            auto build_placeholder = [&](const char* type_str) -> String {
                String ph("{:");

                // fill + align
                if (left_align) {
                    ph += "<";
                } else if (zero_pad) {
                    // '0=' means right-align with '0' fill between prefix and digits
                    ph += "0=";
                }
                // (no flag → default alignment, omit)

                if (alt_form) ph += "#";

                ph += width_str;

                if (!precision_str.is_empty()) {
                    ph += ".";
                    ph += precision_str;
                }

                ph += type_str;
                ph += "}";
                return ph;
            };

            // --- Conversion specifier ---
            if (c == '%') {
                r_fstr += '%';
            } else if (c == 'd' || c == 'i') {
                // Signed integer
                switch (length) {
                    case Length::HH: f_args[arg_count++] = (signed char) va_arg(args, int); break;
                    case Length::H:  f_args[arg_count++] = (short) va_arg(args, int); break;
                    case Length::L:  f_args[arg_count++] = va_arg(args, long); break;
                    case Length::LL: f_args[arg_count++] = va_arg(args, long long); break;
                    default:         f_args[arg_count++] = va_arg(args, int); break;
                }
                precision_str = ""; // Precision isn't allowed for integers
                r_fstr += build_placeholder("");
            } else if (c == 'u') {
                // Unsigned decimal
                switch (length) {
                    case Length::HH:
                        f_args[arg_count++] = (unsigned char) va_arg(args, unsigned int);
                        break;
                    case Length::H:
                        f_args[arg_count++] = (unsigned short) va_arg(args, unsigned int);
                        break;
                    case Length::L:  f_args[arg_count++] = va_arg(args, unsigned long); break;
                    case Length::LL: f_args[arg_count++] = va_arg(args, unsigned long long); break;
                    default:         f_args[arg_count++] = va_arg(args, unsigned int); break;
                }
                precision_str = ""; // Precision isn't allowed for integers
                r_fstr += build_placeholder("");
            } else if (c == 'o') {
                // Octal
                switch (length) {
                    case Length::HH:
                        f_args[arg_count++] = (unsigned char) va_arg(args, unsigned int);
                        break;
                    case Length::H:
                        f_args[arg_count++] = (unsigned short) va_arg(args, unsigned int);
                        break;
                    case Length::L:  f_args[arg_count++] = va_arg(args, unsigned long); break;
                    case Length::LL: f_args[arg_count++] = va_arg(args, unsigned long long); break;
                    default:         f_args[arg_count++] = va_arg(args, unsigned int); break;
                }
                precision_str = ""; // Precision isn't allowed for integers
                r_fstr += build_placeholder("o");
            } else if (c == 'x' || c == 'X') {
                // Hex (kernel only has lowercase 'x'; uppercase is not supported, skip distinction)
                switch (length) {
                    case Length::HH:
                        f_args[arg_count++] = (unsigned char) va_arg(args, unsigned int);
                        break;
                    case Length::H:
                        f_args[arg_count++] = (unsigned short) va_arg(args, unsigned int);
                        break;
                    case Length::L:  f_args[arg_count++] = va_arg(args, unsigned long); break;
                    case Length::LL: f_args[arg_count++] = va_arg(args, unsigned long long); break;
                    default:         f_args[arg_count++] = va_arg(args, unsigned int); break;
                }
                if (!width_str.is_empty()) zero_pad = true;
                precision_str = ""; // Precision isn't allowed for integers
                r_fstr += build_placeholder("x");
            } else if (c == 'f' || c == 'F' || c == 'e' || c == 'E' || c == 'g' || c == 'G'
                       || c == 'a' || c == 'A') {
                // Floating point — scientific/hex/shortest not supported by kernel, use default
                if (length == Length::LD)
                    f_args[arg_count++] = va_arg(args, long double);
                else
                    f_args[arg_count++] = va_arg(args, double);
                r_fstr += build_placeholder("");
            } else if (c == 'c') {
                f_args[arg_count++]  = (char) va_arg(args, int);
                r_fstr              += build_placeholder("");
            } else if (c == 's') {
                f_args[arg_count++]  = va_arg(args, const char*);
                r_fstr              += build_placeholder("");
            } else if (c == 'p') {
                // Pointer — print as hex with prefix
                alt_form             = true;
                f_args[arg_count++]  = (unsigned long) va_arg(args, void*);
                precision_str = ""; // Precision isn't allowed for integers
                r_fstr              += build_placeholder("x");
            } else {
                // '%n' and anything unknown — not supported, consume the argument if needed
                if (c == 'n') va_arg(args, int*);
            }
        }
        LOGGER->log(LogLevel::INFO, r_fstr, f_args, arg_count);
    }

    void acpi_to_rune_redirect_output(void* destination) {
        // Not supported
    }

    // ========================================================================================== //
    // System ACPI Table Access
    // ========================================================================================== //

    ACPI_STATUS acpi_to_rune_get_table_by_address(ACPI_PHYSICAL_ADDRESS address,
                                                  ACPI_TABLE_HEADER * *out_table) {
        return AE_NOT_IMPLEMENTED;
    }

    ACPI_STATUS acpi_to_rune_get_table_by_index(UINT32 table_index,
                                                ACPI_TABLE_HEADER * *out_table,
                                                ACPI_PHYSICAL_ADDRESS * *out_address) {
        return AE_NOT_IMPLEMENTED;
    }

    ACPI_STATUS acpi_to_rune_get_table_by_name(char*                  signature,
                                               UINT32                 instance,
                                               ACPI_TABLE_HEADER**    out_table,
                                               ACPI_PHYSICAL_ADDRESS* out_address) {
        return AE_NOT_IMPLEMENTED;
    }

    // ========================================================================================== //
    // Miscellaneous
    // ========================================================================================== //

    UINT64 acpi_to_rune_get_timer() {
        return System::instance()
                   .get_module<CPU::CPUModule>(ModuleSelector::CPU)
                   ->get_system_timer()
                   ->get_time_since_start()
               / 100;
    }

    ACPI_STATUS acpi_to_rune_signal(UINT32 func, void* info) {
        if (func == ACPI_SIGNAL_FATAL) {
            ACPI_SIGNAL_FATAL_INFO* fatal_info = reinterpret_cast<ACPI_SIGNAL_FATAL_INFO*>(info);
            System::instance().panic("ACPI FATAL: Type={}, Code={}, Argument={}",
                                     fatal_info->Type,
                                     fatal_info->Code,
                                     fatal_info->Argument);
        }
        // ACPI_SIGNAL_BREAKPOINT is not supported
        return AE_OK;
    }

    ACPI_STATUS acpi_to_rune_get_line(char* buffer, UINT32 buffer_length, UINT32* bytes_read) {
        return AE_NOT_IMPLEMENTED; // Dont care about the ACPI debugger atm
    }
}
