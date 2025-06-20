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

#ifndef RUNEOS_APPMANAGEMENT_H
#define RUNEOS_APPMANAGEMENT_H


#include <Pickaxe/SystemCall.h>


namespace Rune::Pickaxe {
    /**
     * @brief A virtual key on the virtual keyboard which defines the keyboard as a 2D matrix of keys. Each key is
     *          defined by it's keycode which is an 16 bit unsigned integer defined as followed:
     *
     * <p>
     *        15      14      13       9 8   4 2   0 <br>
     *    | None | Released | Reserved | Col | Row |
     * </p>
     *
     * <p>
     *  Row     : Row of the key.<br>
     *  Col     : Column of the key.<br>
     *  Released: 1 - Key is released, 0 - Key is pressed<br>
     *  None    : 1 - This key represents a "null" key, 0 - This key represents an actual key press.
     * </p>
     *
     * <p>
     *  Note: This is a duplication of the struct in Device/Keyboard/Keyboard.h.
     * </p>
     */
    class VirtualKey {
        U16 _key_code;

    public:
        static const VirtualKey NONE;
        static constexpr U8     MAX_ROWS = 8;
        static constexpr U8     MAX_COLS = 32;


        /**
         * Create virtual key code in the given row and col that has been released or pressed.
         *
         * @param row       Row.
         * @param col       Column.
         * @param released  True if the key has been released, false if pressed.
         * @return A virtual key code.
         */
        static VirtualKey build(U8 row, U8 col, bool released);


        VirtualKey();


        explicit VirtualKey(U16 code);


        /**
         *
         * @return The integer representation of the key code.
         */
        [[nodiscard]] U16 get_key_code() const;


        /**
         *
         * @return Row position of the pressed key.
         */
        [[nodiscard]] U8 get_row() const;


        /**
         *
         * @return Column position of the pressed key.
         */
        [[nodiscard]] U8 get_col() const;


        /**
         *
         * @return True if the key is currently pressed.
         */
        [[nodiscard]] bool is_pressed() const;


        /**
         *
         * @return True if the key has been released.
         */
        [[nodiscard]] bool is_released() const;


        /**
         *
         * @return True if this virtual key code represents a "null" key.
         */
        [[nodiscard]] bool is_none() const;


        friend bool operator==(const VirtualKey& one, const VirtualKey& two);


        friend bool operator!=(const VirtualKey& one, const VirtualKey& two);
    };


    /**
     * @brief Wait for keyboard key to be pressed or released.
     * @return A virtual key encoding the exact position of the pressed/released key. In case of an error the
     *          VirtualKey::NONE is returned.
     */
    VirtualKey read_std_in();


    /**
     * @brief Write the message to the "stdout" stream which will append the message after the position of the cursor.
     *
     * Note that the length of passed c string is limited to 128 characters (including the null terminator).
     *
     * @param msg    A null terminated c string.
     * @param length Size of the c string.
     */
    void write_std_out(const char* msg, size_t length);


    /**
     * @brief Write the message to the "stderr" stream which will append the message after the position of the cursor.
     *
     * Note that the length of passed c string is limited to 128 characters (including the null terminator).
     *
     * @param msg    A null terminated c string.
     * @param length Size of the c string.
     */
    void write_std_err(const char* msg, size_t length);


    /**
     * @brief Start an application with the given arguments and working directory.
     *
     * The application is started in with it's own virtual memory space and own set of standard streams.
     *
     * <p>
     *  Note that the length of passed c string is limited to 128 characters (including the null terminator) and the
     *  number of command line arguments is limited to 32 arguments (including the null terminator).
     * </p>
     *
     * <p>
     *  The standard stream targets define the source of the stdin and destinations of the stdout/stderr streams.
     *  One of the following targets can be chosen:
     *  <ul>
     *      <li>void - Connect the standard stream to the void stream.</li>
     *      <li>inherit - Connect the stdin/stdout/stderr to the std streams of the calling app.</li>
     *      <li>file:"path"- Only stdout/stderr: Redirect to the specified file, if it does not exist it will be
     *              created.</li>
     *      <li>pipe:"handle": Connect the standard stream to the requested pipe.</li>
     *  </ul>
     * </p>
     *
     * @param app_path          A path to the executable, either absolute or relative to the working directory of the
     *                          calling app.
     * @param argv              The command line arguments, a null terminated array of c strings.
     * @param working_directory The working directory of the app that will be started, either absolute or relative to the
     *                          working directory of the calling app.
     * @param stdin_target  Stream target for stdin.
     * @param stdout_target Stream target for stdout.
     * @param stderr_target Stream target for stderr.
     * @return >= 0: The app has been started, the returned value is the assigned ID.
     *  -1: One or more system call arguments are malformed.
     *  -2: One or more command line arguments exceed the c string limit of 128 characters (including null terminator).
     *  -3: The number of app arguments exceeds the app argument limit of 32 (including null terminator).
     *  -4: The path to the application does not exist.
     *  -5: The working directory does not exist.
     *  -6: The app could not be started.
     */
    int app_start(
            const char* app_path,
            const char* argv[],
            const char* working_directory,
            const char* stdin_target,
            const char* stdout_target,
            const char* stderr_target
    );


    /**
     * The function intentionally has C linkage, so it can be called directly from assembly.
     *
     * @brief Free all resources used by the application and exit it. There is no coming back!
     * @param exitCode Exit code returned by the main thread. >=0 indicates successful app exit and <0 indicates an
     *                  error.
     */
    extern "C" void app_exit(int exit_code);


    /**
     * @brief Wait until the application with the requested handle has exited.
     * @param app_handle    Handle of an application.
     * @return INT_MAX: No application with the requested handle was running,
     *          Else: The exit code of the application.
     */
    int app_join(int app_handle);


    /**
     * @brief Get the current working directory of the active app and copy it to the char buffer.
     * @param wd_out  Pointer to a char buffer.
     * @param wd_size Size of the buffer.
     * @return 0: The working directory has been copied to the given buffer.
     *          -1: The char buffer is too small for the working directory.
     *          -2: An argument was bad, e.g. null pointer.
     */
    S64 app_get_working_directory(const char* wd_out, int wd_size);


    /**
     * @brief Change the working directory of the active app to the requested working directory.
     * @param new_wd The new working directory.
     * @return 0: The working directory of the active app has been changed.
     *          -1: The kernel guardian found one or more bad arguments.
     *          -2: The path contains illegal characters.
     *          -3: The path does not exist.
     *          -4: The node is not a directory.
     *          -5: An IO error happened.
     */
    S64 app_change_working_directory(const char* new_wd);
}

#endif //RUNEOS_APPMANAGEMENT_H
