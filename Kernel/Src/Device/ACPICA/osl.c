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

#include <Device/ACPICA/acpi.h>

// ============================================================================================== //
// OSL-to-rune Bridge
// The abstraction layer between ACPICA and runeOS, it is needed because ACPICA is a C library
// while runeOS is implemented in C++. This means ACPICA is not allowed to reference runeOS,
// because gcc is not able to compile C++ code.
// ============================================================================================== //

/* Initialization */
extern unsigned int       acpi_to_rune_initialize(void);
extern unsigned int       acpi_to_rune_terminate(void);
extern unsigned long long acpi_to_rune_get_root_pointer(void);

/* Memory */
extern void* acpi_to_rune_map_memory(unsigned long long phys, unsigned long long length);
extern void  acpi_to_rune_unmap_memory(void* virt, unsigned long long length);
extern int   acpi_to_rune_get_physical_address(void* virt, unsigned long long* phys_out);
extern void* acpi_to_rune_allocate(unsigned long long size);
extern void  acpi_to_rune_free(void* ptr);

/* Threading */
extern unsigned long long acpi_to_rune_get_thread_id(void);
extern int                acpi_to_rune_execute(int type, void (*func)(void*), void* ctx);
extern void               acpi_to_rune_wait_events_complete(void);
extern void               acpi_to_rune_sleep(unsigned long long ms);
extern void               acpi_to_rune_stall(unsigned int us);

/* Mutexes */
extern void* acpi_to_rune_mutex_create(void);
extern void  acpi_to_rune_mutex_delete(void* mutex);
extern int   acpi_to_rune_mutex_acquire(void* mutex, unsigned short timeout_ms);
extern void  acpi_to_rune_mutex_release(void* mutex);

/* Semaphores */
extern void* acpi_to_rune_semaphore_create(unsigned int max_units, unsigned int initial_units);
extern int   acpi_to_rune_semaphore_delete(void* sem);
extern int   acpi_to_rune_semaphore_wait(void* sem, unsigned int units, unsigned short timeout_ms);
extern int   acpi_to_rune_semaphore_signal(void* sem, unsigned int units);

/* Spinlocks */
extern void*         acpi_to_rune_spinlock_create(void);
extern void          acpi_to_rune_spinlock_delete(void* lock);
extern unsigned long acpi_to_rune_spinlock_acquire(void* lock);
extern void          acpi_to_rune_spinlock_release(void* lock, unsigned long flags);

/* Interrupts */
extern int
acpi_to_rune_install_interrupt(unsigned int irq, unsigned int (*handler)(void*), void* ctx);
extern int acpi_to_rune_remove_interrupt(unsigned int irq, unsigned int (*handler)(void*));

/* I/O */
extern int
acpi_to_rune_read_memory(unsigned long long addr, unsigned long long* val, unsigned int width);
extern int
acpi_to_rune_write_memory(unsigned long long addr, unsigned long long val, unsigned int width);
extern int acpi_to_rune_read_port(unsigned short port, unsigned int* val, unsigned int width);
extern int acpi_to_rune_write_port(unsigned short port, unsigned int val, unsigned int width);

/* PCI */
extern int acpi_to_rune_read_pci_config(unsigned short      seg,
                                        unsigned short      bus,
                                        unsigned short      dev,
                                        unsigned short      func,
                                        unsigned int        reg,
                                        unsigned long long* val,
                                        unsigned int        width);
extern int acpi_to_rune_write_pci_config(unsigned short     seg,
                                         unsigned short     bus,
                                         unsigned short     dev,
                                         unsigned short     func,
                                         unsigned int       reg,
                                         unsigned long long val,
                                         unsigned int       width);

/* Platform */
extern unsigned long long acpi_to_rune_get_timer(void);
extern int                acpi_to_rune_signal(unsigned int func, void* info);
extern int acpi_to_rune_enter_sleep(unsigned char state, unsigned int rega, unsigned int regb);

/* Print */
extern void acpi_to_rune_vprintf(const char* fmt, va_list args);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              Environmental and ACPI Table Interfaces
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsInitialize(void) { return acpi_to_rune_initialize(); }

ACPI_STATUS
AcpiOsTerminate(void) { return acpi_to_rune_terminate(); }

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer(void) {
    /*
     * Return the physical address of the RSDP (Root System Description Pointer)
     * Typically found via:
     * - EFI System Table on UEFI systems
     * - Searching EBDA (Extended BIOS Data Area) on legacy BIOS
     * - Searching 0xE0000-0xFFFFF on legacy BIOS
     */
    return acpi_to_rune_get_root_pointer();
}

ACPI_STATUS
AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* InitVal, ACPI_STRING* NewVal) {
    /*
     * Override predefined ACPI objects (_OS, _REV, etc.)
     * Return AE_OK with *NewVal = NULL to use default values
     */
    *NewVal = NULL;
    return AE_OK;
}

ACPI_STATUS
AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER** NewTable) {
    /*
     * Override ACPI tables (e.g., DSDT, SSDT)
     * Return AE_OK with *NewTable = NULL to use firmware tables
     */
    *NewTable = NULL;
    return AE_OK;
}

ACPI_STATUS
AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER*     ExistingTable,
                            ACPI_PHYSICAL_ADDRESS* NewAddress,
                            UINT32*                NewTableLength) {
    /*
     * Physical override variant - return physical address of replacement table
     * Return AE_OK with *NewAddress = 0 to use firmware tables
     */
    *NewAddress = 0;
    return AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Memory Management
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsCreateCache(char* CacheName, UINT16 ObjectSize, UINT16 MaxDepth, ACPI_CACHE_T** ReturnCache) {
    return AE_NOT_IMPLEMENTED;
}

ACPI_STATUS AcpiOsDeleteCache(ACPI_CACHE_T* Cache) { return AE_NOT_IMPLEMENTED; }

ACPI_STATUS AcpiOsPurgeCache(ACPI_CACHE_T* Cache) { return AE_NOT_IMPLEMENTED; }

void* AcpiOsAcquireObject(ACPI_CACHE_T* Cache) { return NULL; }

ACPI_STATUS AcpiOsReleaseObject(ACPI_CACHE_T* Cache, void* Object) {
    { return AE_NOT_IMPLEMENTED; }
}

void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Where, ACPI_SIZE Length) {
    /*
     * Map physical memory into virtual address space
     * Must handle both RAM and MMIO regions
     * Should use uncached mapping for MMIO
     */
    return NULL;
}

void AcpiOsUnmapMemory(void* LogicalAddress, ACPI_SIZE Size) {
    /* Unmap previously mapped memory */
}

ACPI_STATUS
AcpiOsGetPhysicalAddress(void* LogicalAddress, ACPI_PHYSICAL_ADDRESS* PhysicalAddress) {
    /* Convert virtual address to physical address */
    *PhysicalAddress = 0;
    return AE_OK;
}

void* AcpiOsAllocate(ACPI_SIZE Size) {
    /*
     * Allocate memory (kernel heap)
     * Must be usable in interrupt context
     */
    return NULL;
}

void AcpiOsFree(void* Memory) { /* Free memory allocated by AcpiOsAllocate */ }

BOOLEAN
AcpiOsReadable(void* Memory, ACPI_SIZE Length) {
    /* Check if memory range is readable (for debugging) */
    return TRUE;
}

BOOLEAN
AcpiOsWritable(void* Memory, ACPI_SIZE Length) {
    /* Check if memory range is writable (for debugging) */
    return TRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Multithreading and Scheduling
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_THREAD_ID
AcpiOsGetThreadId(void) {
    /*
     * Return unique thread/task identifier
     * Can be process ID, thread ID, or CPU ID if no threading
     */
    return 1;
}

ACPI_STATUS
AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context) {
    /*
     * Queue deferred work for execution
     * Type can be: OSL_GLOBAL_LOCK_HANDLER, OSL_NOTIFY_HANDLER,
     *              OSL_GPE_HANDLER, OSL_DEBUGGER_MAIN_THREAD,
     *              OSL_DEBUGGER_EXEC_THREAD, OSL_EC_POLL_HANDLER,
     *              OSL_EC_BURST_HANDLER
     * Should execute Function(Context) asynchronously
     */
    return AE_NOT_IMPLEMENTED;
}

void AcpiOsSleep(UINT64 Milliseconds) {
    /* Sleep for specified milliseconds (can be interrupted) */
}

void AcpiOsStall(UINT32 Microseconds) {
    /*
     * Busy-wait for specified microseconds
     * Must not sleep, used in critical sections
     */
}

void AcpiOsWaitEventsComplete() {
    /*
     * Wait for all queued asynchronous events to complete
     * Used during shutdown/suspend
     */
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Mutual Exclusion and Synchronization
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsCreateMutex(ACPI_MUTEX* OutHandle) {
    /* Create a mutex */
    *OutHandle = NULL;
    return AE_OK;
}

void AcpiOsDeleteMutex(ACPI_MUTEX Handle) { /* Delete a mutex */ }

ACPI_STATUS
AcpiOsAcquireMutex(ACPI_MUTEX Handle, UINT16 Timeout) {
    /*
     * Acquire mutex with timeout in milliseconds
     * ACPI_WAIT_FOREVER means wait indefinitely
     */
    return AE_OK;
}

void AcpiOsReleaseMutex(ACPI_MUTEX Handle) { /* Release mutex */ }

ACPI_STATUS
AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle) {
    /* Create a counting semaphore */
    *OutHandle = NULL;
    return AE_OK;
}

ACPI_STATUS
AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle) {
    /* Delete semaphore */
    return AE_OK;
}

ACPI_STATUS
AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout) {
    /*
     * Wait for semaphore units with timeout in milliseconds
     * ACPI_WAIT_FOREVER means wait indefinitely
     */
    return AE_OK;
}

ACPI_STATUS
AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units) {
    /* Signal semaphore (increment by Units) */
    return AE_OK;
}

ACPI_STATUS
AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle) {
    /*
     * Create a spinlock
     * Must be usable in interrupt context
     */
    *OutHandle = NULL;
    return AE_OK;
}

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) { /* Delete spinlock */ }

ACPI_CPU_FLAGS
AcpiOsAcquireLock(ACPI_SPINLOCK Handle) {
    /*
     * Acquire spinlock, disable interrupts
     * Return interrupt state flags
     */
    return 0;
}

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) {
    /* Release spinlock, restore interrupt state from Flags */
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Interrupt Handling
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsInstallInterruptHandler(UINT32           InterruptNumber,
                              ACPI_OSD_HANDLER ServiceRoutine,
                              void*            Context) {
    /*
     * Install interrupt handler
     * ServiceRoutine will be called as: ServiceRoutine(Context)
     * Should return ACPI_INTERRUPT_HANDLED or ACPI_INTERRUPT_NOT_HANDLED
     */
    return AE_NOT_IMPLEMENTED;
}

ACPI_STATUS
AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine) {
    /* Remove previously installed interrupt handler */
    return AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              Memory Access and Memory Mapped I/O
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width) {
    /*
     * Read from physical memory address
     * Width is 8, 16, 32, or 64 bits
     */
    *Value = 0;
    return AE_OK;
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width) {
    /*
     * Write to physical memory address
     * Width is 8, 16, 32, or 64 bits
     */
    return AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Port Input/Output
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width) {
    /*
     * Read from I/O port (x86 IN instruction)
     * Width is 8, 16, or 32 bits
     */
    *Value = 0;
    return AE_OK;
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width) {
    /*
     * Write to I/O port (x86 OUT instruction)
     * Width is 8, 16, or 32 bits
     */
    return AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              PCI Configuration Space Access
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsReadPciConfiguration(ACPI_PCI_ID PciId, UINT32 Register, UINT64* Value, UINT32 Width) {
    /*
     * Read PCI configuration space
     * PciId contains: Segment, Bus, Device, Function
     * Register is the config space offset
     * Width is 8, 16, 32, or 64 bits
     */
    *Value = 0;
    return AE_OK;
}

ACPI_STATUS
AcpiOsWritePciConfiguration(ACPI_PCI_ID PciId, UINT32 Register, UINT64 Value, UINT32 Width) {
    /*
     * Write PCI configuration space
     * PciId contains: Segment, Bus, Device, Function
     * Register is the config space offset
     * Width is 8, 16, 32, or 64 bits
     */
    return AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Formatted Output
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

void ACPI_INTERNAL_VAR_XFACE AcpiOsPrintf(const char* Format, ...) {
    /*
     * Printf-style output for ACPICA messages
     * Should go to kernel log/console
     */
}

void AcpiOsVprintf(const char* Format, va_list Args) { /* vprintf variant of AcpiOsPrintf */ }

void AcpiOsRedirectOutput(void* Destination) {
    /* Redirect debug output (usually not needed in kernel) */
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  System ACPI Table Access
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS AcpiOsGetTableByAddress(ACPI_PHYSICAL_ADDRESS Address, ACPI_TABLE_HEADER** OutTable) {
    return AE_NOT_IMPLEMENTED;
}

ACPI_STATUS AcpiOsGetTableByIndex(UINT32                  Index,
                                  ACPI_TABLE_HEADER**     OutTable,
                                  ACPI_PHYSICAL_ADDRESS** OutAddress) {
    return AE_NOT_IMPLEMENTED;
}

ACPI_STATUS AcpiOsGetTableByName(char*                   Signature,
                                 UINT32                  Instance,
                                 ACPI_TABLE_HEADER**     OutTable,
                                 ACPI_PHYSICAL_ADDRESS** OutAddress) {
    return AE_NOT_IMPLEMENTED;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Miscellaneous
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

UINT64 AcpiOsGetTimer() {
    /*
     * Return current time in 100-nanosecond units
     * Used for performance timing, doesn't need to be precise
     */
    return 0;
}

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info) {
    /*
     * Handle platform-specific signals
     * Function can be: ACPI_SIGNAL_FATAL, ACPI_SIGNAL_BREAKPOINT
     */
    return AE_OK;
}

ACPI_STATUS AcpiOsGetLine(char* Buffer, UINT32 BufferLength, UINT32* BytesRead) {
    /*
     * Read line from debug console (for interactive debugger)
     * Usually not implemented in production kernels
     */
    return AE_NOT_IMPLEMENTED;
}
