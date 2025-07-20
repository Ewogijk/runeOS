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

#ifndef RUNEOS_X64CORE_H
#define RUNEOS_X64CORE_H


#include <CPU/CPU.h>

#include "GDT.h"


namespace Rune::CPU {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          CPU state
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * @brief The content of most CPU registers.
     */
    struct x86CoreState {
        // General purpose registers
        Register RAX;
        Register RBX;
        Register RCX;
        Register RDX;
        Register RSI;
        Register RDI;
        Register R8;
        Register R9;
        Register R10;
        Register R11;
        Register R12;
        Register R13;
        Register R14;
        Register R15;

        // Special registers
        Register RSP;
        Register RBP;
        Register RIP;
        Register RFlags;
        Register CR0;
        Register CR2;
        Register CR3;
        Register CR4;

        // Segment selectors
        Register CS;
        Register DS;
        Register SS;
        Register ES;
        Register FS;
        Register GS;
    };


    /**
     * @brief Load the current CPU state into the state object.
     */
    CLINK void read_state(x86CoreState* state);

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          CPUID functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    struct CPUIDResponse {
        Register rax = 0;
        Register rbx = 0;
        Register rcx = 0;
        Register rdx = 0;
    };


    /**
     * @brief True: CPU ID features are supported. False: Not.
     */
    CLINK bool cpuid_supported();


    /**
     * @brief Make a CPU ID request and store the result in "resp"
     */
    CLINK void make_cpuid_request(U64 request, CPUIDResponse* resp);


    /**
     * Read the 12 byte ASCII CPU vendor into the char `buf` which will be null terminated after the call to this
     * function.
     *
     * @param buf A char buffer of size 13.
     */
    String get_vendor();


    /**
     *
     * @return The size of a physical address in bits.
     */
    U8 get_physical_address_width();


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Model specific registers
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


#define MODEL_SPECIFIC_REGISTERS(X)                         \
    X(ModelSpecificRegister, STAR, 0xC0000081)              \
    X(ModelSpecificRegister, LSTAR, 0xC0000082)             \
    X(ModelSpecificRegister, FMASK, 0xC0000084)             \
    X(ModelSpecificRegister, EFER, 0xC0000080)              \
    X(ModelSpecificRegister, FS_Base, 0xC0000100)           \
    X(ModelSpecificRegister, GS_Base, 0xC0000101)           \
    X(ModelSpecificRegister, KERNEL_GS_BASE, 0xC0000102)    \


    /**
     * ID's of model specific registers.
     */
    DECLARE_TYPED_ENUM(ModelSpecificRegister, U32, MODEL_SPECIFIC_REGISTERS, 0x0) //NOLINT


    /**
     * @brief Write the value to the MSR with the given ID.
     */
    CLINK void write_msr(Register msr_id, Register value);


    /**
     * @brief Read the current value from the MSR with the given ID:
     */
    CLINK Register read_msr(Register msr_id);


    /**
     * @brief Read the value of the pointer that GS is currently pointed at.
     */
    CLINK Register read_gs();

    /**
     * @brief Call the "swapgs" instruction. If GS was currently pointing to KernelGSBase is will be pointing to GSBase
     *          after this call and vice versa.
     */
    CLINK void swapgs();


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Threading functions
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * Make a context switch from the current thread to the next thread.
     *
     * @param cStack Stack of the current thread.
     * @param cVAS   Virtual address space of the current thread.
     * @param nStack Stack of the next thread.
     * @param nVAS   Virtual address space of the next thread.
     */
    CLINK void context_switch_ass(
            LibK::VirtualAddr* c_stack,
            LibK::PhysicalAddr c_vas,
            LibK::VirtualAddr n_stack,
            LibK::PhysicalAddr n_vas
    );


    /**
     * @brief Call the thread main function with argc and argv as parameters in kernel mode.
     */
    CLINK void exec_kernel_mode(Register argc, Register argv, Register func_ptr, Register thread_exit);


    /**
    * @brief Call the thread main function with argc and argv as parameters in user mode.
    *
    * Note that the user stack must be setup as execution stack else undefined behavior will occur.
    */
    CLINK void exec_user_mode(Register argc, Register argv, Register func_ptr);


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Other assembly stuff
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * @brief Enable floating point arithmetic.
     */
    CLINK void enable_sse();


    /**
     * @brief Get the content of the CS register.
     */
    CLINK Register read_cs();


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          x64Core Class
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    class X64Core : public Core {
        U8       _core_id;
        Register _kgs_base; // Current thread's kernel stack pointer
        Register _gs_base;  // Current thread's user stack pointer

    public:

        explicit X64Core(U8 core_id);


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                          Core API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


        bool init() override;


        U8 get_id() override;


        TechSpec get_tech_spec() override;


        ArchSpec get_arch_details() override;


        PrivilegeLevel get_current_privilege_level() override;


        LinkedList<InterruptVector> get_interrupt_vector_table() override;


        void dump_core_state(const SharedPointer<LibK::TextStream>& stream) override;


        void switch_to_thread(Thread* c_thread, Thread* n_thread) override;


        void execute_in_kernel_mode(Thread* t, Register thread_exit) override;


        void execute_in_user_mode(Thread* t) override;


        void update_thread_local_storage(void* tls_ptr) override;


        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //                                      x64 core specific API
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        void dump_core_state(const SharedPointer<LibK::TextStream>& stream, const x86CoreState& state);
    };

}

#endif //RUNEOS_X64CORE_H
