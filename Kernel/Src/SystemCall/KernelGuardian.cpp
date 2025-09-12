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

#include <SystemCall/KernelGuardian.h>

namespace Rune::SystemCall {
    KernelGuardian::KernelGuardian() : _kernel_memory_start(0) {}

    void KernelGuardian::set_kernel_memory_start(VirtualAddr kernel_memory_start) {
        _kernel_memory_start = kernel_memory_start;
    }

    bool KernelGuardian::verify_user_buffer(void* user_buf, size_t user_buf_size) const {
        return user_buf && (uintptr_t) user_buf + user_buf_size < _kernel_memory_start;
    }

    bool KernelGuardian::copy_byte_buffer_user_to_kernel(void* user_buf, size_t user_buf_size, void* kernel_buf) const {
        if (!user_buf || !kernel_buf) return false;

        // Check that the user memory buffer does not intersect with kernel memory
        // If we would copy without the check undefined behavior could occur (we could overwrite something important)
        if ((uintptr_t) user_buf >= _kernel_memory_start
            || (uintptr_t) user_buf + user_buf_size >= _kernel_memory_start)
            return false;

        // Copy the content of the user memory buffer over to the kernel memory buffer
        // We assume userBufSize==kernelBufSize, it is the callers responsibility to ensure this
        memcpy(kernel_buf, user_buf, user_buf_size);
        return true;
    }

    bool KernelGuardian::copy_byte_buffer_kernel_to_user(void* kernel_buf, void* user_buf, size_t user_buf_size) const {
        if (!user_buf || !kernel_buf) return false;

        // Check that the user memory buffer does not intersect with kernel memory
        // If we would copy without the check undefined behavior could occur (we could overwrite something important)
        if ((uintptr_t) user_buf >= _kernel_memory_start
            || (uintptr_t) user_buf + user_buf_size >= _kernel_memory_start)
            return false;

        // Copy the content of the kernel memory buffer over to the user memory buffer
        // We assume userBufSize==kernelBufSize, it is the callers responsibility to ensure this
        memcpy(user_buf, kernel_buf, user_buf_size);
        return true;
    }

    bool
    KernelGuardian::copy_string_user_to_kernel(const char* user_str, const int max_size, const char* kernel_str) const {
        if (!user_str || !kernel_str) return false;

        // Check that the user string does not intersect with kernel memory
        // If we would copy without the check undefined behavior could occur (we could overwrite something important)
        if (reinterpret_cast<uintptr_t>(user_str) >= _kernel_memory_start
            || reinterpret_cast<uintptr_t>(user_str) + max_size >= _kernel_memory_start)
            return false;

        int         size  = 0;
        const char* c_pos = user_str;
        while (*c_pos && size < Ember::STRING_SIZE_LIMIT) {
            c_pos++;
            size++;
            if (max_size >= 0 && size >= max_size) break;
        }

        if (size >= Ember::STRING_SIZE_LIMIT)
            // The user memory string is not null terminated.
            return false;

        if (max_size >= 0 && size > max_size)
            // The user memory string is bigger than expected
            return false;

        memcpy((void*) kernel_str, (void*) user_str, size + 1); // size + 1 -> Include the null terminator
        const_cast<char*>(kernel_str)[size] = '\0';
        return true;
    }
} // namespace Rune::SystemCall
