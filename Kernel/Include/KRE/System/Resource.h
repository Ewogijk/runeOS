
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

#include "KRE/Collections/Array.h"

#include <KRE/Memory.h>
#include <KRE/Stream.h>
#include <KRE/String.h>

namespace Rune {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      ID Counter
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief The handle counter provides a subsystem with unique handles for it's resources.
     *
     * <p>
     *  A handle must be a unsigned numeric type, so it can be incremented.
     * </p>
     * <p>
     *  The handle 0 is reserved and means the resource is invalid or in case of referencing means
     * that no resource is referenced. 0 is basically a null pointer in a sense.
     * </p>
     * @tparam Counter
     */
    template <typename Handle>
    class IDCounter {
        Handle _counter;

      public:
        explicit IDCounter() : _counter(0) {}

        /**
         * @brief Check if the handle counter has free resource handles.
         * @return True: The handle counter has free handless, False: All handles have been used.
         */
        [[nodiscard]] auto has_more() const -> bool { return _counter < _counter + 1; }

        /**
         * @brief Get the next unused handle and increment the counter.
         * @return An unused handle;
         */
        auto acquire() -> Handle { return ++_counter; }

        /**
         * The counter will not be decremented when it is zero, to prevent an underflow.
         *
         * @brief Decrement the previously incremented counter, thus making the last acquired handle
         * usable again.
         */
        void release_last_acquired() {
            if (_counter > 0) _counter--;
        }
    };

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                      Table
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * A table formatter that prints resource properties in tabular format to a stream.
     * @tparam ResourceType Type of the resource.
     * @tparam ColumnCount Number of table columns.
     */
    template <typename ResourceType, size_t ColumnCount>
    class Table {
        // Padding before and after a data row
        static constexpr size_t OUTER_PADDING = 1;
        // Padding in between table cells
        static constexpr size_t INNER_PADDING = 2;
        // Character to be used to draw a horizontal divider
        static constexpr char DIVIDER_CHAR = '-';

        Array<String, ColumnCount>                                _column_headers;
        Array<size_t, ColumnCount>                                _column_widths{};
        Function<Array<String, ColumnCount>(const ResourceType&)> _row_converter;
        LinkedList<Array<String, ColumnCount>>                    _rows;
        size_t                                                    _table_width{0};

        /**
         * Increase width of column cold_idx if _column_widths[col_idx] < new_width.
         * @param col_idx
         * @param new_width
         */
        void adjust_column_width(size_t col_idx, size_t new_width) {
            size_t col_width = _column_widths[col_idx];
            if (col_width < new_width) {
                _table_width            += new_width - col_width;
                _column_widths[col_idx]  = new_width;
            };
        }

        [[nodiscard]] auto make_str_template(char fill, char align, size_t width) const -> String {
            return String::format(":{}{}{}", fill, align, width);
        }

        /**
         * Print a single row of data.
         * @param stream
         * @param data A row of data.
         * @param align Alignment for the string format function.
         */
        void print_data_row(const SharedPointer<TextStream>& stream,
                            Array<String, ColumnCount>&      data,
                            char                             align) {

            for (size_t j = 0; j < OUTER_PADDING; j++) stream->write(" ");

            for (size_t i = 0; i < ColumnCount; i++) {
                stream->write_formatted(
                    String("{") + make_str_template(' ', align, _column_widths[i]) + "}",
                    data[i]);
                if (i != ColumnCount - 1) {
                    for (size_t j = 0; j < INNER_PADDING; j++) stream->write(" ");
                }
            }
            stream->write('\n');
        }

        /**
         * Print a horizontal divider as wide as the table.
         * @param stream
         */
        void print_divider(const SharedPointer<TextStream>& stream) {
            for (size_t i = 0; i < _table_width; i++) stream->write(DIVIDER_CHAR);
            stream->write('\n');
        }

      public:
        constexpr Table(Function<Array<String, ColumnCount>(const ResourceType&)> row_converter)
            : _column_headers(),
              _row_converter(move(row_converter)),
              _table_width(OUTER_PADDING + (INNER_PADDING * (ColumnCount - 1)) + OUTER_PADDING) {}

        constexpr Table(Array<String, ColumnCount>                                column_headers,
                        Function<Array<String, ColumnCount>(const ResourceType&)> row_converter)
            : _column_headers(move(column_headers)),
              _row_converter(move(row_converter)),
              _table_width(OUTER_PADDING + (INNER_PADDING * (ColumnCount - 1)) + OUTER_PADDING) {
            for (size_t i = 0; i < ColumnCount; i++)
                adjust_column_width(i, _column_headers[i].size());
        }

        /**
         * Create a new table with the provided row constructor.
         * @param row_converter
         * @return A table instance.
         */
        static auto
        make_table(Function<Array<String, ColumnCount>(const ResourceType&)> row_converter)
            -> Table<ResourceType, ColumnCount> {
            return Table<ResourceType, ColumnCount>(move(row_converter));
        }

        /**
         * Add all resource objects in the provided collection to the table.
         * @tparam CollectionType Type of the collection.
         * @param collection An iterable collection.
         * @return This table instance.
         */
        template <typename CollectionType>
        auto with_data(CollectionType collection) -> Table<ResourceType, ColumnCount> {
            for (const ResourceType& resource : collection) add_row(resource);
            return *this;
        }

        /**
         * Set the headers of the table.
         * @param headers Array of headers.
         * @return This table instance.
         */
        auto with_headers(Array<String, ColumnCount> headers) -> Table<ResourceType, ColumnCount>& {
            _column_headers = move(headers);
            for (size_t i = 0; i < ColumnCount; i++)
                adjust_column_width(i, _column_headers[i].size());
            return *this;
        }

        /**
         * Add a new table row with the values of the given resource.
         *
         * @param resource Instance of a resource.
         * @return This table instance.
         */
        auto add_row(const ResourceType& resource) -> Table<ResourceType, ColumnCount>& {
            Array<String, ColumnCount> row_values = _row_converter(resource);
            // Check if the column widths need to be increased
            for (size_t i = 0; i < ColumnCount; i++) {
                size_t value_size = row_values[i].size();
                adjust_column_width(i, value_size);
            }
            _rows.add_back(row_values);
            return *this;
        }

        /**
         * Print the table to the provided text stream.
         *
         * @param stream
         * @return This table instance.
         */
        auto print(const SharedPointer<TextStream>& stream) -> Table<ResourceType, ColumnCount>& {
            if (!stream || !stream->is_write_supported()) return *this;

            // Write the column header names
            print_data_row(stream, _column_headers, '^');
            print_divider(stream);

            // Write the table entries
            for (size_t i = 0; i < _rows.size(); i++) print_data_row(stream, *_rows[i], '<');
            return *this;
        }
    };
} // namespace Rune

#endif // RUNEOS_RESOURCE_H
