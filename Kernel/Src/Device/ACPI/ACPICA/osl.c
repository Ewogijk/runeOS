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

#include <Device/ACPI/ACPICA/acpi.h>

// ============================================================================================== //
// OSL-to-rune Bridge
// The abstraction layer between ACPICA and runeOS, it is needed because ACPICA is a C library
// while runeOS is implemented in C++. This means ACPICA is not allowed to reference runeOS,
// because gcc is not able to compile C++ code.
// ============================================================================================== //

/* Environmental and ACPI Table Interfaces */
extern ACPI_STATUS           acpi_to_rune_initialize(void);
extern ACPI_STATUS           acpi_to_rune_terminate(void);
extern ACPI_PHYSICAL_ADDRESS acpi_to_rune_get_root_pointer(void);
extern ACPI_STATUS acpi_to_rune_predefined_override(const ACPI_PREDEFINED_NAMES* predefined_object,
                                                    ACPI_STRING*                 new_value);
extern ACPI_STATUS acpi_to_rune_table_override(ACPI_TABLE_HEADER*  existing_table,
                                               ACPI_TABLE_HEADER** new_table);
extern ACPI_STATUS acpi_to_rune_physical_table_override(ACPI_TABLE_HEADER*     existing_table,
                                                        ACPI_PHYSICAL_ADDRESS* new_address,
                                                        UINT32*                new_table_length);

/* Memory Management */
extern ACPI_STATUS acpi_to_rune_create_cache(char*          cache_name,
                                             UINT16         object_size,
                                             UINT16         cache_depth,
                                             ACPI_CACHE_T** return_cache);
extern ACPI_STATUS acpi_to_rune_delete_cache(ACPI_CACHE_T* cache);
extern ACPI_STATUS acpi_to_rune_purge_cache(ACPI_CACHE_T* cache);
extern void*       acpi_to_rune_acquire_object(ACPI_CACHE_T* cache);
extern ACPI_STATUS acpi_to_rune_release_object(ACPI_CACHE_T* cache, void* object);
extern void*       acpi_to_rune_map_memory(ACPI_PHYSICAL_ADDRESS phys, ACPI_SIZE length);
extern void        acpi_to_rune_unmap_memory(void* virt, ACPI_SIZE length);
extern ACPI_STATUS acpi_to_rune_get_physical_address(void* virt, ACPI_PHYSICAL_ADDRESS* phys_out);
extern void*       acpi_to_rune_allocate(ACPI_SIZE size);
extern void        acpi_to_rune_free(void* ptr);
extern BOOLEAN     acpi_to_rune_readable(void* memory, ACPI_SIZE length);
extern BOOLEAN     acpi_to_rune_writable(void* memory, ACPI_SIZE length);

/* Multithreading and Scheduling Services */
extern ACPI_THREAD_ID acpi_to_rune_get_thread_id();
extern ACPI_STATUS
            acpi_to_rune_execute(ACPI_EXECUTE_TYPE type, ACPI_OSD_EXEC_CALLBACK func, void* ctx);
extern void acpi_to_rune_sleep(UINT64 ms);
extern void acpi_to_rune_stall(UINT64 us);
extern void acpi_to_rune_wait_events_complete();

/* Mutual Exclusion and Synchronization */
extern ACPI_STATUS acpi_to_rune_mutex_create(ACPI_MUTEX* out_handle);
extern void        acpi_to_rune_mutex_delete(ACPI_MUTEX mutex);
extern ACPI_STATUS acpi_to_rune_mutex_acquire(ACPI_MUTEX mutex, UINT16 timeout_ms);
extern void        acpi_to_rune_mutex_release(ACPI_MUTEX mutex);

extern ACPI_STATUS
acpi_to_rune_semaphore_create(UINT32 max_units, UINT32 initial_units, ACPI_SEMAPHORE* out_handle);
extern ACPI_STATUS acpi_to_rune_semaphore_delete(ACPI_SEMAPHORE sem);
extern ACPI_STATUS acpi_to_rune_semaphore_wait(ACPI_SEMAPHORE sem, UINT32 units, UINT16 timeout_ms);
extern ACPI_STATUS acpi_to_rune_semaphore_signal(ACPI_SEMAPHORE sem, UINT32 units);

extern ACPI_STATUS    acpi_to_rune_spinlock_create(ACPI_SPINLOCK* out_handle);
extern void           acpi_to_rune_spinlock_delete(ACPI_SPINLOCK lock);
extern ACPI_CPU_FLAGS acpi_to_rune_spinlock_acquire(ACPI_SPINLOCK lock);
extern void           acpi_to_rune_spinlock_release(ACPI_SPINLOCK lock, ACPI_CPU_FLAGS flags);

/* Interrupt Handling */
extern ACPI_STATUS
acpi_to_rune_install_interrupt_handler(UINT32 irq, ACPI_OSD_HANDLER handler, void* ctx);
extern ACPI_STATUS acpi_to_rune_remove_interrupt_handler(UINT32 irq, ACPI_OSD_HANDLER handler);

/* Memory Access and Memory Mapped I/O */
extern ACPI_STATUS acpi_to_rune_read_memory(ACPI_PHYSICAL_ADDRESS addr, UINT64* val, UINT32 width);
extern ACPI_STATUS acpi_to_rune_write_memory(ACPI_PHYSICAL_ADDRESS addr, UINT64 val, UINT32 width);

/* Port Input/Output */
extern ACPI_STATUS acpi_to_rune_read_port(ACPI_IO_ADDRESS port, UINT32* val, UINT32 width);
extern ACPI_STATUS acpi_to_rune_write_port(ACPI_IO_ADDRESS port, UINT32 val, UINT32 width);

/* PCI Configuration Space Access */
extern ACPI_STATUS
acpi_to_rune_read_pci_config(ACPI_PCI_ID pci_id, UINT32 reg, UINT64* val, UINT32 width);
extern ACPI_STATUS
acpi_to_rune_write_pci_config(ACPI_PCI_ID pci_id, UINT32 reg, UINT64 val, UINT32 width);

/* Formatted Output */
extern void acpi_to_rune_vprintf(const char* format, va_list args);
extern void acpi_to_rune_redirect_output(void* destination);

/* System ACPI Table Access */

extern ACPI_STATUS acpi_to_rune_get_table_by_address(ACPI_PHYSICAL_ADDRESS address,
                                                     ACPI_TABLE_HEADER**   out_table);
extern ACPI_STATUS acpi_to_rune_get_table_by_index(UINT32                  table_index,
                                                   ACPI_TABLE_HEADER**     out_table,
                                                   ACPI_PHYSICAL_ADDRESS** out_address);
extern ACPI_STATUS acpi_to_rune_get_table_by_name(char*                   signature,
                                                  UINT32                  instance,
                                                  ACPI_TABLE_HEADER**     out_table,
                                                  ACPI_PHYSICAL_ADDRESS** out_address);

/* Miscellaneous */
extern UINT64      acpi_to_rune_get_timer();
extern ACPI_STATUS acpi_to_rune_signal(UINT32 func, void* info);
extern ACPI_STATUS acpi_to_rune_get_line(char* buffer, UINT32 buffer_length, UINT32* bytes_read);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              Environmental and ACPI Table Interfaces
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsInitialize(void) { return acpi_to_rune_initialize(); }

ACPI_STATUS
AcpiOsTerminate(void) { return acpi_to_rune_terminate(); }

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer(void) { return acpi_to_rune_get_root_pointer(); }

ACPI_STATUS
AcpiOsPredefinedOverride(const ACPI_PREDEFINED_NAMES* InitVal, ACPI_STRING* NewVal) {
    *NewVal = NULL;
    return AE_OK;
}

ACPI_STATUS
AcpiOsTableOverride(ACPI_TABLE_HEADER* ExistingTable, ACPI_TABLE_HEADER** NewTable) {
    *NewTable = NULL;
    return AE_OK;
}

ACPI_STATUS
AcpiOsPhysicalTableOverride(ACPI_TABLE_HEADER*     ExistingTable,
                            ACPI_PHYSICAL_ADDRESS* NewAddress,
                            UINT32*                NewTableLength) {
    *NewAddress = 0;
    return AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Memory Management
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsCreateCache(char* CacheName, UINT16 ObjectSize, UINT16 MaxDepth, ACPI_CACHE_T** ReturnCache) {
    return acpi_to_rune_create_cache(CacheName, ObjectSize, MaxDepth, ReturnCache);
}

ACPI_STATUS AcpiOsDeleteCache(ACPI_CACHE_T* Cache) { return acpi_to_rune_delete_cache(Cache); }

ACPI_STATUS AcpiOsPurgeCache(ACPI_CACHE_T* Cache) { return acpi_to_rune_purge_cache(Cache); }

void* AcpiOsAcquireObject(ACPI_CACHE_T* Cache) { return acpi_to_rune_acquire_object(Cache); }

ACPI_STATUS AcpiOsReleaseObject(ACPI_CACHE_T* Cache, void* Object) {
    return acpi_to_rune_release_object(Cache, Object);
}

void* AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Where, ACPI_SIZE Length) {
    return acpi_to_rune_map_memory(Where, Length);
}

void AcpiOsUnmapMemory(void* LogicalAddress, ACPI_SIZE Size) {
    acpi_to_rune_unmap_memory(LogicalAddress, Size);
}

ACPI_STATUS
AcpiOsGetPhysicalAddress(void* LogicalAddress, ACPI_PHYSICAL_ADDRESS* PhysicalAddress) {
    return acpi_to_rune_get_physical_address(LogicalAddress, PhysicalAddress);
}

void* AcpiOsAllocate(ACPI_SIZE Size) {
    return acpi_to_rune_allocate(Size);
}

void AcpiOsFree(void* Memory) { acpi_to_rune_free(Memory); }

BOOLEAN
AcpiOsReadable(void* Memory, ACPI_SIZE Length) { return acpi_to_rune_readable(Memory, Length); }

BOOLEAN
AcpiOsWritable(void* Memory, ACPI_SIZE Length) { return acpi_to_rune_writable(Memory, Length); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Multithreading and Scheduling
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_THREAD_ID
AcpiOsGetThreadId(void) { return acpi_to_rune_get_thread_id(); }

ACPI_STATUS
AcpiOsExecute(ACPI_EXECUTE_TYPE Type, ACPI_OSD_EXEC_CALLBACK Function, void* Context) {
    return acpi_to_rune_execute(Type, Function, Context);
}

void AcpiOsSleep(UINT64 Milliseconds) { acpi_to_rune_sleep(Milliseconds); }

void AcpiOsStall(UINT32 Microseconds) { acpi_to_rune_stall(Microseconds); }

void AcpiOsWaitEventsComplete() { acpi_to_rune_wait_events_complete(); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Mutual Exclusion and Synchronization
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsCreateMutex(ACPI_MUTEX* OutHandle) { return acpi_to_rune_mutex_create(OutHandle); }

void AcpiOsDeleteMutex(ACPI_MUTEX Handle) { acpi_to_rune_mutex_delete(Handle); }

ACPI_STATUS
AcpiOsAcquireMutex(ACPI_MUTEX Handle, UINT16 Timeout) {
    return acpi_to_rune_mutex_acquire(Handle, Timeout);
}

void AcpiOsReleaseMutex(ACPI_MUTEX Handle) { acpi_to_rune_mutex_release(Handle); }

ACPI_STATUS
AcpiOsCreateSemaphore(UINT32 MaxUnits, UINT32 InitialUnits, ACPI_SEMAPHORE* OutHandle) {
    return acpi_to_rune_semaphore_create(MaxUnits, InitialUnits, OutHandle);
}

ACPI_STATUS
AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle) { return acpi_to_rune_semaphore_delete(Handle); }

ACPI_STATUS
AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units, UINT16 Timeout) {
    return acpi_to_rune_semaphore_wait(Handle, Units, Timeout);
}

ACPI_STATUS
AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle, UINT32 Units) {
    return acpi_to_rune_semaphore_signal(Handle, Units);
}

ACPI_STATUS
AcpiOsCreateLock(ACPI_SPINLOCK* OutHandle) { return acpi_to_rune_spinlock_create(OutHandle); }

void AcpiOsDeleteLock(ACPI_SPINLOCK Handle) { acpi_to_rune_spinlock_delete(Handle); }

ACPI_CPU_FLAGS
AcpiOsAcquireLock(ACPI_SPINLOCK Handle) { return acpi_to_rune_spinlock_acquire(Handle); }

void AcpiOsReleaseLock(ACPI_SPINLOCK Handle, ACPI_CPU_FLAGS Flags) {
    acpi_to_rune_spinlock_release(Handle, Flags);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Interrupt Handling
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsInstallInterruptHandler(UINT32           InterruptNumber,
                              ACPI_OSD_HANDLER ServiceRoutine,
                              void*            Context) {
    return acpi_to_rune_install_interrupt_handler(InterruptNumber, ServiceRoutine, Context);
}

ACPI_STATUS
AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber, ACPI_OSD_HANDLER ServiceRoutine) {
    return acpi_to_rune_remove_interrupt_handler(InterruptNumber, ServiceRoutine);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              Memory Access and Memory Mapped I/O
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64* Value, UINT32 Width) {
    return acpi_to_rune_read_memory(Address, Value, Width);
}

ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width) {
    return acpi_to_rune_write_memory(Address, Value, Width);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Port Input/Output
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address, UINT32* Value, UINT32 Width) {
    return acpi_to_rune_read_port(Address, Value, Width);
}

ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address, UINT32 Value, UINT32 Width) {
    return acpi_to_rune_write_port(Address, Value, Width);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                              PCI Configuration Space Access
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS
AcpiOsReadPciConfiguration(ACPI_PCI_ID PciId, UINT32 Register, UINT64* Value, UINT32 Width) {
    return acpi_to_rune_read_pci_config(PciId, Register, Value, Width);
}

ACPI_STATUS
AcpiOsWritePciConfiguration(ACPI_PCI_ID PciId, UINT32 Register, UINT64 Value, UINT32 Width) {
    return acpi_to_rune_write_pci_config(PciId, Register, Value, Width);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Formatted Output
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

void ACPI_INTERNAL_VAR_XFACE AcpiOsPrintf(const char* Format, ...) {
    va_list args;
    va_start(args, Format);
    acpi_to_rune_vprintf(Format, args);
    va_end(args);
}

void AcpiOsVprintf(const char* Format, va_list Args) { acpi_to_rune_vprintf(Format, Args); }

void AcpiOsRedirectOutput(void* Destination) { acpi_to_rune_redirect_output(Destination); }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  System ACPI Table Access
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

ACPI_STATUS AcpiOsGetTableByAddress(ACPI_PHYSICAL_ADDRESS Address, ACPI_TABLE_HEADER** OutTable) {
    return acpi_to_rune_get_table_by_address(Address, OutTable);
}

ACPI_STATUS AcpiOsGetTableByIndex(UINT32                  Index,
                                  ACPI_TABLE_HEADER**     OutTable,
                                  ACPI_PHYSICAL_ADDRESS** OutAddress) {
    return acpi_to_rune_get_table_by_index(Index, OutTable, OutAddress);
}

ACPI_STATUS AcpiOsGetTableByName(char*                   Signature,
                                 UINT32                  Instance,
                                 ACPI_TABLE_HEADER**     OutTable,
                                 ACPI_PHYSICAL_ADDRESS** OutAddress) {
    return acpi_to_rune_get_table_by_name(Signature, Instance, OutTable, OutAddress);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                  Miscellaneous
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

UINT64 AcpiOsGetTimer() { return acpi_to_rune_get_timer(); }

ACPI_STATUS AcpiOsSignal(UINT32 Function, void* Info) {
    return acpi_to_rune_signal(Function, Info);
}

ACPI_STATUS AcpiOsGetLine(char* Buffer, UINT32 BufferLength, UINT32* BytesRead) {
    return acpi_to_rune_get_line(Buffer, BufferLength, BytesRead);
}
