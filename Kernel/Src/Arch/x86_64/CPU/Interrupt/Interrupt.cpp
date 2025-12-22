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

#include "../X64Core.h"
#include "IDT.h"
#include "ISR_Stubs.h"

#include <KRE/Collections/Array.h>

#include <CPU/Interrupt/Exception.h>
#include <CPU/Interrupt/IRQ.h>
#include <CPU/Interrupt/Interrupt.h>

namespace Rune::CPU {
    constexpr U8 EXCEPTION_COUNT = 32;
    constexpr U8 IRQ_COUNT       = 224;
    constexpr U8 IRQ_NOT_PENDING = 255;

    constexpr U8 PAGE_FAULT_VECTOR = 14;

    /// Mapping of the first 32 interrupt codes (0..31) to exception names.
    const Array<const char*, EXCEPTION_COUNT> EXCEPTIONS = {"Divide by zero error",
                                                            "Debug",
                                                            "Non-maskable Interrupt",
                                                            "Breakpoint",
                                                            "Overflow",
                                                            "Bound Range Exceeded",
                                                            "Invalid Opcode",
                                                            "Device Not Available",
                                                            "Double Fault",
                                                            "Coprocessor Segment Overrun",
                                                            "Invalid TSS",
                                                            "Segment Not Present",
                                                            "Stack-Segment Fault",
                                                            "General Protection Fault",
                                                            "Page Fault",
                                                            "",
                                                            "x87 Floating-Point Exception",
                                                            "Alignment Check",
                                                            "Machine Check",
                                                            "SIMD Floating-Point Exception",
                                                            "Virtualization Exception",
                                                            "Control Protection Exception ",
                                                            "",
                                                            "",
                                                            "",
                                                            "",
                                                            "",
                                                            "",
                                                            "Hypervisor Injection Exception",
                                                            "VMM Communication Exception",
                                                            "Security Exception",
                                                            ""};

    const Array<U8, 4> EXCEPTION_TYPE_TO_ID = {
        255, // NONE
        0,   // DIVISION_BY_ZERO
        8,   // DOUBLE_FAULT
        14   // PAGE_FAULT
    };

    DEFINE_ENUM(ExceptionType, EXCEPTION_TYPES, 0x0)

    struct x86InterruptContext {
        x86CoreState core_state;
        Register     i_vector;
        Register     i_error_code;
        Register     i_RIP;
        Register     i_CS;
        Register     i_RFLAGS;
        Register     i_RSP;
        Register     i_SS;
    };

    /**
     * @brief Container for IRQ table entry and the handler.
     */
    struct IRQContainer {
        IRQTableEntry entry;
        IRQHandler    handler = [] { return IRQState::PENDING; };

        friend auto operator==(const IRQContainer& a, const IRQContainer& b) -> bool {
            return a.entry.device_handle == b.entry.device_handle;
        }

        friend auto operator!=(const IRQContainer& a, const IRQContainer& b) -> bool {
            return a.entry.device_handle != b.entry.device_handle;
        }
    };

    // The panic stream serves as output for debugging information when an exception has no
    // installed handler
    // NOLINTBEGIN cannot be const, as arrays are modified. Should these be put in a struct?
    SharedPointer<TextStream>                  PANIC_STREAM;
    Array<ExceptionHandler*, EXCEPTION_COUNT>  EXCEPTION_HANDLER_TABLE; // ISR 0-31
    Array<LinkedList<IRQContainer>, IRQ_COUNT> IRQ_HANDLER_TABLE;       // ISR 32-255
    Array<U64, EXCEPTION_COUNT + IRQ_COUNT>    RAISED_COUNT;  // Number of times an ISR was raised
    Array<U64, IRQ_COUNT>                      PENDING_COUNT; // Number of times an IRQ was left
    // pending

    PICDriver* PIC;
    U8         CURRENT_IRQ     = IRQ_NOT_PENDING;
    bool       MANUAL_EOI_SENT = false;
    // NOLINTEND

    CLINK void interrupt_dispatch(x86InterruptContext* x64_i_ctx) {
        U8 vector = x64_i_ctx->i_vector;
        // NOLINTBEGIN vector is CPU provided and irq_line is provided by the PIC -> indexes are
        // fine
        RAISED_COUNT[vector]++;
        if (vector < EXCEPTION_COUNT) {
            // Handle exception
            const char* exception_name = EXCEPTIONS[vector];
            if (!EXCEPTION_HANDLER_TABLE[vector]) {
                if (PANIC_STREAM && PANIC_STREAM->is_write_supported()) {
                    // Dump the state of the current core
                    PANIC_STREAM->set_background_color(Pixie::VSCODE_RED);
                    PANIC_STREAM->set_foreground_color(Pixie::VSCODE_WHITE);
                    PANIC_STREAM->write(
                        "-------------------------------------------- Interrupt Context "
                        "--------------------------------------------\n");
                    PANIC_STREAM->write_formatted(
                        "Unhandled exception {}: {}, Error code: {:0=#4x}\n",
                        x64_i_ctx->i_vector,
                        exception_name,
                        x64_i_ctx->i_error_code);
                    PANIC_STREAM->write_formatted(
                        "ip={:0=#4x}:{:0=#16x}, sp={:0=#4x}:{:0=#16x}, rflags={:0=#16x}\n\n",
                        x64_i_ctx->i_CS,
                        x64_i_ctx->i_RIP,
                        x64_i_ctx->i_SS,
                        x64_i_ctx->i_RSP,
                        x64_i_ctx->i_RFLAGS);
                    ((X64Core*) current_core())
                        ->dump_core_state(PANIC_STREAM, x64_i_ctx->core_state);
                    PANIC_STREAM->reset_style();
                }
                while (true) __asm__("hlt");
            }
            InterruptContext i_ctx = {x64_i_ctx->i_error_code, x64_i_ctx->i_vector};

            (*EXCEPTION_HANDLER_TABLE[vector])(forward<InterruptContext*>(&i_ctx),
                                               forward<const char*>(exception_name));
        } else {
            // Handle IRQ
            U8 irq_line = vector - PIC->get_irq_line_offset();
            CURRENT_IRQ = irq_line;

            if (!IRQ_HANDLER_TABLE[irq_line].is_empty()) {
                IRQState irq_state = IRQState::PENDING;
                for (auto irq_container : IRQ_HANDLER_TABLE[irq_line]) {
                    irq_state = irq_container.handler();
                    if (irq_state == IRQState::HANDLED) {
                        irq_container.entry.handled++;
                        break;
                    }
                }
                if (irq_state == IRQState::PENDING) PENDING_COUNT[irq_line]++;
            }
            // NOLINTEND
            if (!MANUAL_EOI_SENT) PIC->send_end_of_interrupt(irq_line);

            CURRENT_IRQ     = IRQ_NOT_PENDING;
            MANUAL_EOI_SENT = false;
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Interrupt API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto exception_get_table() -> LinkedList<ExceptionTableEntry> {
        LinkedList<ExceptionTableEntry> table;
        for (size_t i = 0; i < EXCEPTION_COUNT; i++)
            // NOLINTBEGIN iterating -> i is always in bounds
            table.add_back({.vector  = (U8) i,
                            .name    = EXCEPTIONS[i],
                            .raised  = RAISED_COUNT[i],
                            .handled = EXCEPTION_HANDLER_TABLE[i] != nullptr});
        // NOLINTEND
        return table;
    }

    void interrupt_load_vector_table() {
        idt_load();
        init_interrupt_service_routines();
        // Enable CPU exceptions
        for (U8 i = 0; i < EXCEPTION_COUNT; i++) idt_get()->entry[i].flags.p = true;
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Exception API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    void exception_install_panic_stream(SharedPointer<TextStream> panic_stream) {
        PANIC_STREAM = move(panic_stream);
    }

    auto exception_is_enabled(ExceptionType type) -> bool {
        return idt_get()->entry[EXCEPTION_TYPE_TO_ID[type.to_value()]].flags.p;
    }

    void exception_set_enabled(ExceptionType type, bool enabled) {
        idt_get()->entry[EXCEPTION_TYPE_TO_ID[type.to_value()]].flags.p = enabled;
    }

    auto exception_install_handler(ExceptionType type, ExceptionHandler* exception_handler)
        -> bool {
        if (exception_handler == nullptr) return false;

        switch (type) {
            case ExceptionType::PAGE_FAULT:
                if (EXCEPTION_HANDLER_TABLE[PAGE_FAULT_VECTOR] == nullptr)
                    EXCEPTION_HANDLER_TABLE[PAGE_FAULT_VECTOR] = exception_handler;
                return true;
            default: return false;
        }
    }

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          IRQ API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    auto irq_init(const LinkedList<PICDriver*>& pic_drivers) -> int {
        int pic_idx = -1;
        for (size_t i = 0; i < pic_drivers.size(); i++) {
            auto* driver = *pic_drivers[i];
            if (driver->start()) {
                PIC     = driver;
                pic_idx = (int) i;
                break;
            }
        }
        if (pic_idx < 0) return -1;

        interrupt_enable();
        return pic_idx;
    }

    auto irq_get_line_limit() -> U8 { return IRQ_COUNT; }

    auto irq_get_table_for(U8 irq_line) -> IRQTable {
        if (irq_line >= IRQ_COUNT || (PIC == nullptr))
            return {.irq_line     = 0,
                    .raised       = 0,
                    .left_pending = 0,
                    .entry        = LinkedList<IRQTableEntry>()};
        IRQTable table;
        table.irq_line = irq_line;
        // NOLINTBEGIN done bounds check on irq_line
        table.raised = RAISED_COUNT[irq_line + PIC->get_irq_line_offset()]; // Need offset into IDT
        // NOLINTEND
        for (auto& c : IRQ_HANDLER_TABLE[irq_line]) table.entry.add_back(c.entry);
        return table;
    }

    auto
    irq_install_handler(U8 irq_line, U16 dev_handle, const String& dev_name, IRQHandler handler)
        -> bool {
        if (irq_line >= IRQ_COUNT || (PIC == nullptr)) return false;

        interrupt_disable();
        for (auto& c : IRQ_HANDLER_TABLE[irq_line]) {
            if (c.entry.device_handle == dev_handle) {
                interrupt_enable();
                return false; // An IRQ handler for the device is already installed
            }
        }
        // NOLINTBEGIN done bounds check on irq_line
        IRQ_HANDLER_TABLE[irq_line].add_back({
            .entry   = {.device_handle = dev_handle, .device_name = dev_name, .handled = 0},
            .handler = move(handler)
        });
        if (IRQ_HANDLER_TABLE[irq_line].size() == 1) {
            U8 vector = PIC->get_irq_line_offset() + irq_line;
            idt_get()->entry[vector].flags.p =
                true;                  // Enable interrupt when first handler is installed
            PIC->clear_mask(irq_line); // Enable IRQ on PIC
        }
        // NOLINTEND
        interrupt_enable();
        return true;
    }

    auto irq_uninstall_handler(U8 irq_line, U16 dev_handle) -> bool {
        if (irq_line >= IRQ_COUNT || (PIC == nullptr)) return false;

        interrupt_disable();
        IRQContainer to_remove;
        for (auto& c : IRQ_HANDLER_TABLE[irq_line]) {
            if (c.entry.device_handle == dev_handle) {
                to_remove = c;
            }
        }
        if (to_remove.entry.device_handle != dev_handle) {
            interrupt_disable();
            return false; // No IRQ handler installed for device
        }
        // NOLINTBEGIN done bounds check on irq_line
        IRQ_HANDLER_TABLE[irq_line].remove(to_remove);
        if (IRQ_HANDLER_TABLE[irq_line].is_empty()) {
            U8 vector = PIC->get_irq_line_offset() + irq_line;
            PIC->mask(irq_line); // Disable IRQ on PIC
            idt_get()->entry[vector].flags.p =
                false; // Disable interrupt when last handler is uninstalled
        }
        // NOLINTEND
        interrupt_enable();
        return true;
    }

    auto irq_send_eoi() -> bool {
        if ((PIC == nullptr) || CURRENT_IRQ >= IRQ_COUNT) return false;

        PIC->send_end_of_interrupt(CURRENT_IRQ);
        MANUAL_EOI_SENT = true;
        return true;
    }
} // namespace Rune::CPU