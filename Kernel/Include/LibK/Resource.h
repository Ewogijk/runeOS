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

#ifndef RUNEOS_RESOURCE_H
#define RUNEOS_RESOURCE_H


#include <Ember/Ember.h>
#include <Hammer/String.h>
#include <Hammer/Memory.h>

#include <LibK/Stream.h>


namespace Rune::LibK {

    /**
     * @brief The handle counter provides a subsystem with unique handles for it's resources.
     *
     * <p>
     *  A handle must be a unsigned numeric type, so it can be incremented.
     * </p>
     * <p>
     *  The handle 0 is reserved and means the resource is invalid or in case of referencing means that no resource is
     *  referenced. 0 is basically a null pointer in a sense.
     * </p>
     * @tparam Counter
     */
    template<typename Handle>
    class HandleCounter {
        Handle _counter;
    public:
        explicit HandleCounter() : _counter(0) {

        }


        /**
         * @brief Check if the handle counter has free resource handles.
         * @return True: The handle counter has free handless, False: All handles have been used.
         */
        [[nodiscard]] bool has_more_handles() const {
            return _counter < _counter + 1;
        }


        /**
         * @brief Get the next unused handle and increment the counter.
         * @return An unused handle;
         */
        Handle acquire_handle() {
            return ++_counter;
        }


        /**
         * The counter will not be decrement when it is zero, to prevent an underflow.
         *
         * @brief Decrement the previously incremented counter, thus making the last acquired handle usable again.
         */
        void release_last_acquired() {
            if (_counter > 0)
                _counter--;
        }
    };


    /**
     * @brief Defines the header and width of a column. Furthermore the ValueYeeter is a function that returns the
     *          string representation of the value that should be displayed in the column for a given resource.
     */
    template<class Resource>
    struct Column {
        String                 header = "";
        size_t                      width  = 0;
        Function<String(Resource*)> value_yeeter;


        /**
         * @brief Make a column named "Handle-Name" and width 56 that displays the Handle and Name properties of a
         *          resource.
         *
         * A resource must fulfill the following requirements:
         * <ol>
         *  <li>It must define the "U16 Handle" public property.</li>
         *  <li>It must define the "Lib::String Name" public property.</li>
         * </ol>
         * @param col_width Width of the column in characters.
         * @return The column
         */
        static Column make_handle_column_table(size_t col_width) {
            return {
                    "Handle-Name",
                    col_width,
                    [](Resource* app) {
                        return String::format("{}-{}", app->handle, app->name);
                    }
            };
        }
    };


    /**
     * @brief The table formatter formats information about system resources in a tabular format. The columns are
     *          defined by the subsystems.
     */
    template<class Resource>
    class TableFormatter {
        String                       _name;
        LinkedList<Column<Resource>> _table_columns;
        size_t                       _table_width;


        [[nodiscard]] String make_str_template(char fill, char align, size_t width) const {
            return String::format(":{}{}{}", fill, align, width);
        }


    public:
        TableFormatter() : _name(""), _table_columns(), _table_width(0) {

        }


        /**
         * @brief Configure the table formatter with a table header and column definitions.
         * @param name         Name of the table, will be printed as part of a header.
         * @param tableColumns Table column definitions.
         */
        void configure(const String& name, const LinkedList<Column<Resource>>& table_columns) {

            if (_table_columns.size() == 0) {
                _name         = name;
                _table_columns = move(table_columns);

                _table_width = 2 * (_table_columns.size() - 1); // Number of spaces in between columns
                for (auto& c: _table_columns)
                    _table_width += c.width;
            }
        }


        /**
         * @brief Write the formatted table with information about each resource returned by the iterator to the given
         *          stream.
         * @param stream
         * @param iterator
         */
        void dump(const SharedPointer<TextStream>& stream, const Function<Resource*()>& iterator) const {
            if (!stream->is_write_supported())
                return;

            // Write a divider with the resource table name
            stream->write_formatted(
                    String("{") + make_str_template('-', '^', _table_width) + "}\n",
                    String::format(" {} Table ", _name)
            );

            // Write the column header names
            for (size_t i = 0; i < _table_columns.size(); i++) {
                auto* tc = _table_columns[i];
                stream->write_formatted(String("{") + make_str_template(' ', '<', tc->width) + "}", tc->header);
                if (i < _table_columns.size() - 1)
                    stream->write("  ");
            }
            stream->write('\n');

            // Write the horizontal column divider between headers and table entries
            for (size_t i = 0; i < _table_width; i++)
                stream->write('-');
            stream->write('\n');

            // Write the table entries
            Resource* curr = iterator();
            while (curr) {
                for (size_t i = 0; i < _table_columns.size(); i++) {
                    auto* tc = _table_columns[i];
                    stream->write_formatted(
                            String("{") + make_str_template(' ', '<', tc->width) + "}",
                            tc->value_yeeter(curr)
                    );
                    if (i < _table_columns.size() - 1)
                        stream->write("  ");
                }
                stream->write('\n');
                curr = iterator();
            }
        }
    };
}

#endif //RUNEOS_RESOURCE_H
