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

#ifndef RUNEOS_ALGORITHM_H
#define RUNEOS_ALGORITHM_H


#include <KernelRuntime/CppLanguageSupport.h>
#include <KernelRuntime/Utility.h>


namespace Rune {

    template<typename T>
    int partition(T array[], int l, int h) {
        T        pivot = array[h];
        int      i     = l - 1;
        for (int j     = l; j <= h - 1; j++) {
            if (array[j] <= pivot) {
                i++;
                swap(array[i], array[j]);
            }
        }
        swap(array[i + 1], array[h]);
        return i + 1;
    }


    template<typename T>
    void quick_sort(T array[], int l, int h) {
        if (l < h) {
            const int pivot = partition(array, l, h);
            quick_sort(array, l, pivot - 1);
            quick_sort(array, pivot + 1, h);
        }
    }


    /**
     * Sort the given array of objects inplace that must overload the "<=" operator.
     *
     * @tparam T        Type of the array objects.
     * @param array     Array to be sorted.
     * @param arr_size   Size of the array.
     */
    template<typename T>
    void sort(T array[], const size_t arr_size) {
        quick_sort(array, 0, arr_size - 1);
    }


    /**
     * Delete the requested number of elements starting from the given idx from the array.
     *
     * @tparam T
     * @param arr
     * @param idx   Starting point of deletion.
     * @param count Number of elements to delete.
     */
    template<typename T>
    void array_delete(T arr[], size_t idx, size_t& count) {
        memmove(arr + idx, arr + idx + 1, sizeof(T) * (count - idx - 1));
        count--;
    }


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Bit Manipulation
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     *
     * @tparam T     Number type.
     * @param num
     * @param offset Bit offset.
     * @return True: The bit at offset is set, False: The bit is not set.
     */
    template<typename T>
    bool check_bit(T num, size_t offset) {
        return num >> offset & 1;
    }


    /**
     * Set the bit at offset and leave all other bits as they are.
     *
     * @tparam T     Number type.
     * @param num
     * @param offset Bit offset.
     * @return The number with the bit at offset set.
     */
    template<typename T>
    T set_bit(T num, const size_t offset) {
        return num | 1 << offset;
    }


    /**
     * Clear the bit at offset and leave all other bits as they are.
     *
     * @tparam T     Number type.
     * @param num
     * @param offset Bit offset.
     * @return The number with the bit at offset cleared.
     */
    template<typename T>
    T clear_bit(T num, const size_t offset) {
        return num & ~(1 << offset);
    }
}

#endif //RUNEOS_ALGORITHM_H
