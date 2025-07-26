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

#ifndef RUNEOS_KERNELGUARDIAN_H
#define RUNEOS_KERNELGUARDIAN_H


#include <Ember/Definitions.h>

#include <LibK/KMemory.h>


namespace Rune::SystemCall {

    class KernelGuardian {
        LibK::VirtualAddr _kernel_memory_start;

    public:
        /**
         * @brief Maximum number of characters (including the null terminator) a user land string can have.
         */
        static constexpr U8 USER_STRING_LIMIT = 128;


        void set_kernel_memory_start(LibK::VirtualAddr kernel_memory_start);


        /**
         * @brief Verify that the user buffer is not null and check that it does not intersect with kernel memory.
         *
         * @param user_buf      Pointer to a byte buffer in user mode memory.
         * @param user_buf_size Size of the user mode buffer.
         * @return True: The user mode buffer has passed the checks, False: The user mode buffer is bad.
         */
        bool verify_user_buffer(void* user_buf, size_t user_buf_size) const;


        /**
         * @brief Verify the user and kernel memory buffer and then copy the content of the user memory buffer to the
         *          kernel memory buffer.
         *
         * <p>
         *  Buffer verification includes the usual null check but it is also checked that the user memory buffer does
         *  not intersect with kernel memory.
         * </p>
         * <p>
         *  Furthermore the content of the user memory buffer is copied to the kernel memory buffer, this is important
         *  so that other user mode threads cannot modify the user memory buffer while the system call is running and
         *  therefore manipulate the kernel in some way or simply crash it.
         * </p>
         * <p>
         *  It is assumed that userBufSize==kernelBufSize, the caller must ensure that this is the case.
         * </p>
         * @param user_buf      Pointer to a buffer in user mode memory.
         * @param user_buf_size Size of the user memory buffer.
         * @param kernel_buf    Pointer to buffer in kernel mode memory.
         * @return True: The user memory is copied to the kernel buffer, False: The user or kernel buffer are null
         *          pointers or does intersect with kernel memory, no memory was copied to the kernel buffer.
         */
        bool copy_byte_buffer_user_to_kernel(void* user_buf, size_t user_buf_size, void* kernel_buf) const;


        /**
         * @brief Verify the user and kernel memory buffer and then copy the content of the kernel memory buffer to the
         *          user memory buffer. It is assumed that the user and kernel buffer are the same size.
         *
         * <p>
         *  Buffer verification includes the usual null check but it is also checked that the user memory buffer does
         *  not intersect with kernel memory.
         * </p>
         * <p>
         *  It is assumed that userBufSize==kernelBufSize, the caller must ensure that this is the case.
         * </p>
         * @param kernel_buf    Pointer to some a buffer in kernel mode memory.
         * @param user_buf      Pointer to some buffer in user mode memory.
         * @param user_buf_size Size of the user memory buffer.
         * @return True: The kernel memory is copied to the user buffer, False: The user buffer is a null pointer or
         *          does intersect with kernel memory, no memory was copied to the kernel buffer.
         */
        bool copy_byte_buffer_kernel_to_user(void* kernel_buf, void* user_buf, size_t user_buf_size) const;


        /**
         * @brief Verify that the c string is null terminated and copy it into the kernelStr buffer.
         *
         * <p>
         *  If (expSize >= 0) it is additionally checked that the c string has the expSize.
         * </p>
         * <p>
         *  It is assumed that the kernelStr buffer has enough space to fit the whole string
         *  (including the null terminator). That is it has a size of at least (UserStringLimit + 1) or if
         *  (expSize >= 0) at least (expSize + 1).
         * </p>
         * @param user_str   A c string buffer in user mode memory.
         * @param exp_size   The expected size of the user mode string, set to -1 to disable the size check.
         * @param kernel_str A c string buffer in kernel mode memory.
         * @return True: The user mode string is null terminated and has a valid size, it got copied to the kernelStr.
         *          False: The user mode string is not null terminated or has an invalid size, it was not copied to
         *                 kernelStr.
         */
        bool copy_string_user_to_kernel(const char* user_str, int exp_size, const char* kernel_str) const;
    };
}

#endif //RUNEOS_KERNELGUARDIAN_H
