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

#include "X64Core.h"

#include "Interrupt/IDT.h"


namespace Rune::CPU {
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          CPUID Features
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    String get_vendor() {
        CPUIDResponse cpuid_response;
        make_cpuid_request(0x0, &cpuid_response);
        char buf[13];
        buf[0]  = (char) ((cpuid_response.rbx >> 0) & 0xFF);
        buf[1]  = (char) ((cpuid_response.rbx >> 8) & 0xFF);
        buf[2]  = (char) ((cpuid_response.rbx >> 16) & 0xFF);
        buf[3]  = (char) ((cpuid_response.rbx >> 24) & 0xFF);
        buf[4]  = (char) ((cpuid_response.rdx >> 0) & 0xFF);
        buf[5]  = (char) ((cpuid_response.rdx >> 8) & 0xFF);
        buf[6]  = (char) ((cpuid_response.rdx >> 16) & 0xFF);
        buf[7]  = (char) ((cpuid_response.rdx >> 24) & 0xFF);
        buf[8]  = (char) ((cpuid_response.rcx >> 0) & 0xFF);
        buf[9]  = (char) ((cpuid_response.rcx >> 8) & 0xFF);
        buf[10] = (char) ((cpuid_response.rcx >> 16) & 0xFF);
        buf[11] = (char) ((cpuid_response.rcx >> 24) & 0xFF);
        buf[12] = 0;
        return { buf };
    }


    U8 get_physical_address_width() {
        CPUIDResponse cpuid_response;
        make_cpuid_request(0x80000008, &cpuid_response);
        return cpuid_response.rax & 0xFF;
    }


    IMPLEMENT_TYPED_ENUM(ModelSpecificRegister, U32, MODEL_SPECIFIC_REGISTERS, 0x0)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
//                                          X64Core Class
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    // when implementing SMP I need to figure out how to properly store multiple GDT (on per core)
    // storage as class member does not work, probably due to padding bytes added by compiler (did not debug that detailed)
    SegmentDescriptor     SD[7];
    GlobalDescriptorTable GDT;
    TaskStateSegment64    TSS;


    X64Core::X64Core(U8 core_id) : _core_id(core_id), _kgs_base(0), _gs_base(0) {

    }


    bool X64Core::init() {
        if (!cpuid_supported())
            return false;

        // NOTE: comment not valid anymore but too lazy to remove
        // We set the GDT members here instead of the constructor, because we need to init
        // the bootstrap core even before global constructors, this means we cannot set the
        // values in the constructor because it is never called.
        // This is relevant because we need an X64Core instance for the boostrap core, but we
        // cannot dynamically allocate it, therefore we need to declare it globally.
        // When we discover more cores later on, we will dynamically allocate them and could
        // init the values in the constructor, but this is also works, so we just use it for
        // all cores.
        GDT.limit = sizeof(SD) - 1;
        GDT.entry = SD;

        enable_sse();    // Enable floating point instructions
        init_gdt(&GDT, &TSS);
        load_gdtr(&GDT, GDTOffset::KERNEL_CODE, GDTOffset::KERNEL_DATA);
        load_task_state_register(GDTOffset::TSS);

        // KernelGSBase holds a pointer to the kernel stack of the running thread
        // GSBase holds a pointer to the user stack of the running thread
        // These are needed during system calls as the CPU does not switch stacks automatically, so we need to keep
        // track of them ourselves, these MSRs are intended to do exactly that
        write_msr(ModelSpecificRegister::KERNEL_GS_BASE, (uintptr_t) &_kgs_base);
        write_msr(ModelSpecificRegister::GS_Base, (uintptr_t) &_gs_base);

        // Initial values are set for debugging purposes
        _kgs_base = 1;
        _gs_base  = 2;

        // If GS points initially to 2, the user mode GS placeholder, then call "swapgs"
        if (read_gs() == 2)
            CPU::swapgs();
        return true;
    }


    U8 X64Core::get_id() {
        return _core_id;
    }


    TechSpec X64Core::get_tech_spec() {
        return {
                get_vendor(),
                "",
                ""
        };
    }


    ArchSpec X64Core::get_arch_details() {
        return {
                get_physical_address_width()
        };
    }


    PrivilegeLevel X64Core::get_current_privilege_level() {
        U8 ring = read_cs() & 0x3;   // Bits 0-1 encode the current privilege level
        if (ring == 3)
            return PrivilegeLevel::USER;
        else if (ring == 0)
            return PrivilegeLevel::KERNEL;
        else
            return PrivilegeLevel::NONE;
    }


    LinkedList<InterruptVector> X64Core::get_interrupt_vector_table() {
        LinkedList<InterruptVector> ivt;

        for (int i = 0; i < 256; i++) {
            GateDescriptor gd = idt_get()->entry[i];

            auto handler_addr = ((LibK::VirtualAddr) gd.offset_high) << 32
                                | ((LibK::VirtualAddr) gd.offset_mid) << 16
                                | gd.offset_low;

            PrivilegeLevel pl = PrivilegeLevel::NONE;

            if (gd.flags.dpl == 0)
                pl = PrivilegeLevel::KERNEL;
            else if (gd.flags.dpl == 3)
                pl = PrivilegeLevel::USER;

            ivt.add_back(
                    {
                            (U8) i,
                            handler_addr,
                            pl,
                            gd.flags.p > 0

                    }
            );
        }
        return ivt;
    }


    void X64Core::dump_core_state(const SharedPointer<LibK::TextStream>& stream) {
        x86CoreState state{ };
        read_state(&state);
        dump_core_state(stream, state);
    }


    void X64Core::switch_to_thread(Thread* c_thread, Thread* n_thread) {
        // The kernel stack is essentially a temporary buffer whenever kernel code is run
        // Kernel code is only ever run after an exception, IRQ or syscall and after this is handled
        // the kernel stack will be emptied, that is we set the stack pointer (nearly) to the bottom
        // on top of the null frame
        auto kernel_sp_bottom = ((LibK::MemorySize) (uintptr_t) n_thread->kernel_stack_bottom) + Thread::KERNEL_STACK_SIZE - 8;
        TSS.rsp_0 = kernel_sp_bottom;
        _kgs_base = kernel_sp_bottom;
        _gs_base  = n_thread->user_stack_top;

        context_switch_ass(
                &c_thread->kernel_stack_top,       // Passed as pointer so the assembly can update the value in the thread struct
                c_thread->base_page_table_address,
                n_thread->kernel_stack_top,
                n_thread->base_page_table_address
        );
    }


    void X64Core::execute_in_kernel_mode(Thread* t, Register thread_exit) {
        // threadExit will be pushed onto the stack so t->Main returns to it
        // -> adjust the kernel stack in the thread struct manually
        t->kernel_stack_top -= 8;
        exec_kernel_mode(t->argc, (uintptr_t) t->argv, (uintptr_t) t->main, thread_exit);
    }


    void X64Core::execute_in_user_mode(Thread* t) {
        // Update cached stack pointers
        TSS.rsp_0 = t->kernel_stack_top;
        _kgs_base = t->kernel_stack_top;
        _gs_base  = t->user_stack_top;
        exec_user_mode(t->argc, (uintptr_t) t->argv, (uintptr_t) t->main);
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      x64 core specific API
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    void X64Core::dump_core_state(const SharedPointer<LibK::TextStream>& stream, const x86CoreState& state) {
        stream->write_formatted(
                "-------------------------------------------- CPU{} Core Dump --------------------------------------------\n",
                _core_id
        );
        stream->write_formatted(
                "rax={:0=#16x}, rbx={:0=#16x}, rcx={:0=#16x}, rdx={:0=#16x}\n",
                state.RAX,
                state.RBX,
                state.RCX,
                state.RDX
        );
        stream->write_formatted(
                "rsi={:0=#16x}, rdi={:0=#16x}, rbp={:0=#16x}, rsp={:0=#16x}\n",
                state.RSI,
                state.RDI,
                state.RBP,
                state.RSP
        );
        stream->write_formatted(
                "r8 ={:0=#16x}, r9 ={:0=#16x}, r10={:0=#16x}, r11={:0=#16x}\n",
                state.R8,
                state.R9,
                state.R10,
                state.R11
        );
        stream->write_formatted(
                "r12={:0=#16x}, r13={:0=#16x}, r14={:0=#16x}, r15={:0=#16x}\n",
                state.R12,
                state.R13,
                state.R14,
                state.R15
        );
        stream->write_formatted(

                "rip={:0=#16x}, rflags={:0=#16x}\n",
                state.RIP,
                state.RFlags
        );
        stream->write_formatted(
                "cr0={:0=#16x}, cr2={:0=#16x}, cr3={:0=#16x}, cr4={:0=#16x}\n",
                state.CR0,
                state.CR2,
                state.CR3,
                state.CR4
        );
        stream->write_formatted(
                "cs={:0=#4x}, ds={:0=#4x}, ss={:0=#4x}, es={:0=#4x}, fs={:0=#4x}, gs={:0=#4x}\n",
                state.CS,
                state.DS,
                state.SS,
                state.ES,
                state.FS,
                state.GS
        );


        InterruptDescriptorTable* idt = idt_get();
        stream->write_formatted("");
        stream->write_formatted("GDT={:0=#16x}, Limit={:0=#4x}\n", (uintptr_t) &GDT.entry, GDT.limit);
        stream->write_formatted("IDT={:0=#16x}, Limit={:0=#4x}\n", (uintptr_t) &idt->entry, idt->limit);
        stream->write_formatted(


                "TSS={:0=#16x}, RSP0={:0=#16x}\n",
                (uintptr_t) &TSS,
                TSS.rsp_0
        );
        stream->write_formatted("");

        stream->write_formatted("------------------ Model Specific Registers -----------------\n");
        stream->write_formatted("EFER        ={:0=#16x}\n", read_msr(ModelSpecificRegister::EFER));
        stream->write_formatted("STAR        ={:0=#16x}\n", read_msr(ModelSpecificRegister::STAR));
        stream->write_formatted("LSTAR       ={:0=#16x}\n", read_msr(ModelSpecificRegister::LSTAR));
        stream->write_formatted("FMASK       ={:0=#16x}\n", read_msr(ModelSpecificRegister::FMASK));
        stream->write_formatted(
                "KernelGSBase={:0=#16x} ({:0=#16x})\n",
                read_msr(ModelSpecificRegister::KERNEL_GS_BASE),
                _kgs_base
        );
        stream->write_formatted(
                "GSBase      ={:0=#16x} ({:0=#16x})\n",
                read_msr(ModelSpecificRegister::GS_Base),
                _gs_base
        );
        stream->write_formatted("GS          ={:0=#16x}\n", read_gs());
        stream->write_formatted("");

        stream->write_formatted("------------------ Global Descriptor Table -----------------\n");
        stream->write_formatted("  Sel           Base         Limit  A RW DC E S DPL P L DB G\n");
        for (int i = 0; i < 5; i++) {
            SegmentDescriptor sd    = GDT.entry[i];
            U32               limit = (U32) sd.limit_flags.limit_high << 16 | (U32) sd.limit_low;
            stream->write_formatted(
                    " {:0=#2x}    {:0=#16x} {:0=#5x} {} {}  {}  {} {}  {}  {} {} {}  {}\n",
                    i * sizeof(SegmentDescriptor),
                    0,
                    limit,
                    sd.access_byte.accessed,
                    sd.access_byte.read_write,
                    sd.access_byte.direction_conforming,
                    sd.access_byte.executable,
                    sd.access_byte.s,
                    sd.access_byte.descriptor_privilege_level,
                    sd.access_byte.present,
                    sd.limit_flags.long_mode,
                    sd.limit_flags.db,
                    sd.limit_flags.granularity
            );
        }
    }
}