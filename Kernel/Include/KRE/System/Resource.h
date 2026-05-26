
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

#include <KRE/Collections/Array.h>
#include <KRE/Collections/HashMap.h>

#include <KRE/Memory.h>
#include <KRE/Stream.h>
#include <KRE/String.h>
#include <KRE/TypeTraits.h>

namespace Rune {

    // ========================================================================================== //
    // Resource
    // ========================================================================================== //

    /// @brief A resource is a uniquely identifiable object with a human-readable name, e.g. a file.
    ///
    /// A handle is an unsigned integer that is unique for its specific type, more concrete no two
    /// files are allowed to have the same handle, but a file and say thread are allowed to have the
    /// same handle, because they are different types of resources.
    ///
    /// The handle 0 is the NONE handle and indicates a non-existent resource, this handle must not
    /// be assigned to any valid resource.
    ///
    /// For debugging purpose each handle has name, names are not required to be unique.
    ///
    /// @tparam ResourceType
    /// @tparam Handle
    template <Integer Handle>
    class Resource {
        Handle m_handle;
        String m_name;

      public:
        /// @brief Handle 0 indicates non-existent resources.
        static constexpr U8 HANDLE_NONE = 0;

        Resource(Handle handle, String name) : m_handle(handle), m_name(name) {}

        /// @brief
        /// @return The handle of this resource.
        [[nodiscard]] auto get_handle() const -> Handle { return m_handle; }

        /// @brief
        /// @return The name of this resource.
        [[nodiscard]] auto get_name() const -> String { return m_name; }

        /// @brief
        /// @return The unique name contains handle and name.
        [[nodiscard]] auto get_unique_name() const -> String {
            return String::format("{}-{}", m_handle, m_name);
        }
    };

    // ========================================================================================== //
    // HandleFactory
    // ========================================================================================== //

    /// @brief The handle counter provides unique handles for newly created resources.
    /// @tparam Handle
    template <Integer Handle>
    class HandleCounter {
        Handle _counter;

      public:
        explicit HandleCounter() : _counter(Resource<Handle>::HANDLE_NONE) {}

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

    /// @brief Type of handle.
    using Handle = U32;

    /// @brief The handle factory is responsible to provide unique handles and
    class HandleFactory {
        Handle             m_counter{0};
        LinkedList<Handle> m_free_list;

      public:
        explicit HandleFactory() = default;

        /// @brief
        /// @return
        auto borrow() -> Handle {
            if (!m_free_list.empty()) {
                return m_free_list.remove_back().value();
            }

            return ++m_counter;
        }

        /// @brief
        /// @param handle
        auto give_back(Handle handle) -> void {
            SILENCE_UNUSED(handle)
            m_free_list.add_back(handle);
        }
    };

    // ====================================================================================== //
    // Table Formatter
    // ====================================================================================== //

    /// @brief A table formatter that prints resource properties in tabular format to a stream.
    /// @tparam ResourceType Type of the resource.
    /// @tparam ColumnCount Number of table columns.
    template <typename ResourceType, size_t ColumnCount>
    class TableFormatter {
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

        /// @brief Increase width of column cold_idx if _column_widths[col_idx] < new_width.
        /// @param col_idx
        /// @param new_width
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

        /// @brief Print a single row of data.
        /// @param stream
        /// @param data A row of data.
        /// @param align Alignment for the string format function.
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

        /// @brief Print a horizontal divider as wide as the table.
        /// @param stream
        void print_divider(const SharedPointer<TextStream>& stream) {
            for (size_t i = 0; i < _table_width; i++) stream->write(DIVIDER_CHAR);
            stream->write('\n');
        }

      public:
        constexpr TableFormatter(
            Function<Array<String, ColumnCount>(const ResourceType&)> row_converter)
            : _column_headers(),
              _row_converter(move(row_converter)),
              _table_width(OUTER_PADDING + (INNER_PADDING * (ColumnCount - 1)) + OUTER_PADDING) {}

        constexpr TableFormatter(
            Array<String, ColumnCount>                                column_headers,
            Function<Array<String, ColumnCount>(const ResourceType&)> row_converter)
            : _column_headers(move(column_headers)),
              _row_converter(move(row_converter)),
              _table_width(OUTER_PADDING + (INNER_PADDING * (ColumnCount - 1)) + OUTER_PADDING) {
            for (size_t i = 0; i < ColumnCount; i++)
                adjust_column_width(i, _column_headers[i].size());
        }

        /// @brief Create a new table with the provided row constructor.
        /// @param row_converter
        /// @return A table instance.
        static auto
        make_table(Function<Array<String, ColumnCount>(const ResourceType&)> row_converter)
            -> TableFormatter<ResourceType, ColumnCount> {
            return TableFormatter<ResourceType, ColumnCount>(move(row_converter));
        }

        /// @brief Add all resource objects in the provided collection to the table.
        /// @tparam CollectionType Type of the collection.
        /// @param collection An iterable collection.
        /// @return This table instance.
        template <typename CollectionType>
        auto with_data(CollectionType collection) -> TableFormatter<ResourceType, ColumnCount> {
            for (const ResourceType& resource : collection) add_row(resource);
            return *this;
        }

        /// @brief Set the headers of the table.
        /// @param headers Array of headers.
        /// @return This table instance.
        auto with_headers(Array<String, ColumnCount> headers)
            -> TableFormatter<ResourceType, ColumnCount>& {
            _column_headers = move(headers);
            for (size_t i = 0; i < ColumnCount; i++)
                adjust_column_width(i, _column_headers[i].size());
            return *this;
        }

        /// @brief Add a new table row with the values of the given resource.
        ///
        /// @param resource Instance of a resource.
        /// @return This table instance.
        auto add_row(const ResourceType& resource) -> TableFormatter<ResourceType, ColumnCount>& {
            Array<String, ColumnCount> row_values = _row_converter(resource);
            // Check if the column widths need to be increased
            for (size_t i = 0; i < ColumnCount; i++) {
                size_t value_size = row_values[i].size();
                adjust_column_width(i, value_size);
            }
            _rows.add_back(row_values);
            return *this;
        }

        /// @brief Print the table to the provided text stream.
        ///
        /// @param stream
        /// @return This table instance.
        auto print(const SharedPointer<TextStream>& stream)
            -> TableFormatter<ResourceType, ColumnCount>& {
            if (!stream || !stream->is_write_supported()) return *this;

            // Write the column header names
            print_data_row(stream, _column_headers, '^');
            print_divider(stream);

            // Write the table entries
            for (size_t i = 0; i < _rows.size(); i++) print_data_row(stream, _rows[i], '<');
            return *this;
        }
    };

    // ========================================================================================== //
    // Resource Cache
    // ========================================================================================== //

    /// @brief A cache of userspace accessible resources e.g., files or mutexes.
    /// @tparam ResourceType Type of the stored resource.
    /// @tparam ColumnCount  Number of columns used when printing the table.
    template <typename ResourceType, size_t ColumnCount>
    class ResourceCache {
        HashMap<Handle, SharedPointer<ResourceType>>                             m_resources{};
        HandleFactory                                                            m_handle_factory;
        Array<String, ColumnCount>                                               m_column_headers;
        Function<Array<String, ColumnCount>(const SharedPointer<ResourceType>&)> m_row_converter;

      public:
        explicit ResourceCache(
            Array<String, ColumnCount>                                               column_headers,
            Function<Array<String, ColumnCount>(const SharedPointer<ResourceType>&)> row_converter)
            : m_column_headers(move(column_headers)),
              m_row_converter(move(row_converter)) {}

        /// @brief
        /// @return Return a raw-pointer copy of all resources in the table.
        [[nodiscard]] auto get_resources() const -> LinkedList<SharedPointer<ResourceType>> {
            LinkedList<SharedPointer<ResourceType>> copy;
            for (const auto& resource : m_resources) copy.add_back(*resource.value);
            return copy;
        }

        /// @brief Allocate a new resource on the heap and assign it a unique handle.
        /// @tparam Args Type of resource constructor arguments.
        /// @param name Name of the resource.
        /// @param args Resource constructor arguments.
        /// @return The created resource, or null if no handles are available.
        template <typename... Args>
        auto allocate(const String& name, Args&&... args) -> SharedPointer<ResourceType> {
            auto resource =
                make_shared<ResourceType>(m_handle_factory.borrow(), name, forward<Args>(args)...);
            m_resources.put(resource->get_handle(), resource);
            return resource;
        }

        /**
         * @brief Delete the resource with the given handle from the table.
         *
         * Existing shared pointers held by callers remain valid until released by those callers.
         *
         * @param handle Handle of the resource to delete.
         * @return True if the resource was deleted, false if no such resource exists.
         */
        auto free(Handle handle) -> bool {
            bool freed = m_resources.remove(handle);
            if (freed) m_handle_factory.give_back(handle);
            return freed;
        }

        /**
         * @brief Search for a resource instance by handle.
         * @param handle Handle of the requested resource.
         * @return The resource, or null if no resource with the handle exists.
         */
        [[nodiscard]] auto find(Handle handle) const -> SharedPointer<ResourceType> {
            auto it = m_resources.find(handle);
            return it == m_resources.end() ? SharedPointer<ResourceType>() : *it->value;
        }

        /**
         * @brief Print all resources in the table to the stream.
         * @param stream Stream to print to.
         */
        void print(const SharedPointer<TextStream>& stream) const {
            TableFormatter<SharedPointer<ResourceType>, ColumnCount>::make_table(m_row_converter)
                .with_headers(m_column_headers)
                .with_data(m_resources.values())
                .print(stream);
        }
    };
} // namespace Rune

#endif // RUNEOS_RESOURCE_H
