//
// Created by ewogijk on 9/23/25.
//

#ifndef RUNEOS_LINKEDLIST_H
#define RUNEOS_LINKEDLIST_H

#include <KRE/CRL/CRL.h>
#include <KRE/Utility.h>

namespace Rune {

    // ========================================================================================== //
    // LinkedList
    // ========================================================================================== //

    /// @brief A linked list node with an element.
    /// @tparam T
    template <typename T>
    struct Node {
        T        m_element;
        Node<T>* m_previous = nullptr;
        Node<T>* m_next     = nullptr;

        explicit Node(T element) : m_element(move(element)) {}
    };

    /// @brief An iterator over a linked list.
    /// @tparam T
    template <typename T>
    class LinkedListIterator {
        Node<T>* m_current;

      public:
        explicit LinkedListIterator(Node<T>* ptr) : m_current(ptr) {}

        [[nodiscard]]
        auto has_next() const -> bool {
            return m_current;
        }

        auto operator*() const -> T& { return m_current->m_element; }

        auto operator->() const -> T* { return m_current ? &m_current->m_element : nullptr; }

        // pre-increment
        auto operator++() -> LinkedListIterator<T>& {
            m_current = m_current->m_next;
            return *this;
        }

        // post-increment
        auto operator++(int) -> LinkedListIterator<T> {
            LinkedListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator==(const LinkedListIterator& other) const -> bool {
            return m_current == other.m_current;
        }

        auto operator!=(const LinkedListIterator& other) const -> bool {
            return m_current != other.m_current;
        }
    };

    /// @brief A linked list implementation.
    /// @tparam T
    template <typename T>
    class LinkedList {
        Node<T>* m_first;
        Node<T>* m_last;
        size_t   m_size{0};

        // Add to front or back, add at index not needed atm
        void add0(Node<T>* node, bool front) {
            if (m_first == nullptr) {
                node->m_next     = nullptr;
                node->m_previous = nullptr;
                m_first = m_last = node;
            } else {
                if (front) {
                    node->m_next        = m_first;
                    m_first->m_previous = node;
                    m_first             = node;
                } else {
                    node->m_previous = m_last;
                    m_last->m_next   = node;
                    m_last           = node;
                }
            }
            m_size++;
        }

        auto remove0(Node<T>* node) -> Optional<T> {
            if (node == nullptr) {
                return Optional<T>();
            }

            if (node->m_previous != nullptr) {
                node->m_previous->m_next = node->m_next;
            } else {
                m_first = node->m_next;
            }

            if (node->m_next != nullptr) {
                node->m_next->m_previous = node->m_previous;
            } else {
                m_last = node->m_previous;
            }

            if (node->m_next == nullptr && node->m_previous == nullptr) {
                m_first = m_last = nullptr;
            }

            T removed_ele    = move(node->m_element);
            node->m_previous = nullptr;
            node->m_next     = nullptr;
            delete node;
            m_size--;
            return Optional<T>(forward<T>(removed_ele));
        }

        auto index_of0(const T& element) -> int {
            int idx = 0;
            for (auto& node : *this) {
                if (node == element) {
                    return idx;
                }
                idx++;
            }
            return -1;
        }

        auto index_of0(const T& element) const -> int {
            int idx = 0;
            for (auto& node : *this) {
                if (node == element) {
                    return idx;
                }
                idx++;
            }
            return -1;
        }

        void free_nodes() {
            Node<T>* curr = m_first;
            while (curr) {
                Node<T>* next = curr->m_next;
                delete curr;
                curr = next;
            }
        }

        void copy(const LinkedList<T>& other) {
            if (other.m_size == 0) {
                m_first = m_last = nullptr;
                m_size           = 0;
                return;
            }

            Node<T>* o_curr = other.m_first;
            Node<T>* t_prev = nullptr;
            while (o_curr) {
                auto* t_curr = new Node<T>(o_curr->m_element);
                if (o_curr->m_previous != nullptr) {
                    t_curr->m_previous = t_prev;
                    if (t_prev != nullptr) {
                        t_prev->m_next = t_curr;
                    }
                }

                if (o_curr == other.m_first) {
                    m_first = t_curr;
                }
                if (o_curr == other.m_last) {
                    m_last = t_curr;
                }

                o_curr = o_curr->m_next;
                t_prev = t_curr;
            }
            m_size = other.m_size;
        }

      public:
        explicit LinkedList() : m_first(nullptr), m_last(nullptr) {}

        LinkedList(std::initializer_list<T> values) : m_first(nullptr), m_last(nullptr) {
            for (const auto& value : values) add_back(value);
        }

        ~LinkedList() { free_nodes(); }

        LinkedList(const LinkedList<T>& other) noexcept { copy(other); }

        auto operator=(const LinkedList<T>& other) noexcept -> LinkedList& {
            if (this == &other) {
                return *this;
            }

            free_nodes();
            copy(other);
            return *this;
        }

        LinkedList(LinkedList<T>&& other) noexcept : m_size(other.m_size) {
            swap(m_first, other.m_first);
            swap(m_last, other.m_last);

            other.m_first = nullptr;
            other.m_last  = nullptr;
            other.m_size  = 0;
        }

        auto operator=(LinkedList<T>&& other) noexcept -> LinkedList& {
            swap(m_first, other.m_first);
            swap(m_last, other.m_last);
            m_size       = other.m_size;
            other.m_size = 0;
            return *this;
        }

        /// @brief
        /// @return First element.
        [[nodiscard]] auto first() const -> T& pre(m_size > 0) {
            contract_assert(m_size > 0);
            return m_first->m_element;
        }

        /// @brief
        /// @return Last element.
        [[nodiscard]] auto last() const -> T& pre(m_size > 0) { return m_last->m_element; }

        /// @brief
        /// @return True if the list contains no elements.
        [[nodiscard]] auto empty() const -> bool { return m_first == nullptr; }

        /// @brief
        /// @return Number of elements in the list.
        [[nodiscard]] auto size() const -> size_t { return m_size; }

        /// @brief Add the element to the end of the list.
        /// @param element
        void add_back(const T& element) { add0(new Node<T>(move(element)), false); }

        /// @brief  Add the element to the end of the list.
        /// @param element
        void add_back(T&& element) { add0(new Node<T>(move(element)), false); }

        /// @brief Add the element to the front of the list.
        /// @param element
        void add_front(const T& element) { add0(new Node<T>(move(element)), true); }

        /// @brief Add the element to the front of the list.
        /// @param element
        void add_front(T&& element) { add0(new Node<T>(move(element)), true); }

        /// @brief Add all elements of the other list to the end of this list.
        /// @param other
        void add_all(const LinkedList<T>& other) {
            for (T& element : other) {
                add_back(element);
            }
        }

        /// @brief Remove the first element.
        /// @return An optional with the removed element.
        auto remove_front() -> Optional<T> { return remove0(m_first); }

        /// @brief Remove the last element.
        /// @return An optional with the removed element.
        auto remove_back() -> Optional<T> { return remove0(m_last); }

        /// @brief Remove the element from the list.
        /// @param element
        /// @return An optional with the removed element.
        auto remove(const T& element) -> Optional<T> {
            Node<T>* to_remove = m_first;
            while (to_remove != nullptr && to_remove->m_element != element) {
                to_remove = to_remove->m_next;
            }
            return remove0(to_remove);
        }

        /// @brief Remove the element from the list.
        /// @param element
        /// @return An optional with the removed element.
        auto remove(T&& element) -> Optional<T> {
            Node<T>* to_remove = m_first;
            while (to_remove != nullptr && to_remove->m_element != element) {
                to_remove = to_remove->m_next;
            }
            return remove0(to_remove);
        }

        /// @brief
        /// @param index Remove the element at the requested index.
        /// @return An optional with the removed element.
        auto remove_at(size_t index) -> Optional<T> pre(0 < index && index < m_size) {
            size_t   idx       = 0;
            Node<T>* to_remove = m_first;
            while (idx < index) {
                to_remove = to_remove->m_next;
                idx++;
            }
            return remove0(to_remove);
        }

        /// @brief Get the index of the element in the list.
        /// @param element
        /// @return The index of the element, or -1 if not found.
        auto index_of(const T& element) const -> int {
            return index_of0(element);
        }

        /// @brief Get the index of the element in the list.
        /// @param element
        /// @return The index of the element, or -1 if not found.
        auto index_of(const T& element) -> int {
            return index_of0(element);
        }

        /// @brief Get the index of the element in the list.
        /// @param element
        /// @return The index of the element, or -1 if not found.
        auto index_of(T&& element) const -> int {
            return index_of0(move(element));
        }

        /// @brief Get the index of the element in the list.
        /// @param element
        /// @return The index of the element, or -1 if not found.
        auto index_of(T&& element) -> int {
            return index_of0(move(element));
        }

        /// @brief
        /// @param element
        /// @return True: The element is in the list, False: Is not in the list.
        auto contains(const T& element) const -> bool { return index_of(element) >= 0; }

        /// @brief
        /// @param element
        /// @return True: The element is in the list, False: Is not in the list.
        auto contains(const T& element) -> bool { return index_of(element) >= 0; }

        /// @brief
        /// @param element
        /// @return True: The element is in the list, False: Is not in the list.
        auto contains(T&& element) const -> bool { return index_of(move(element)) >= 0; }

        /// @brief
        /// @param element
        /// @return True: The element is in the list, False: Is not in the list.
        auto contains(T&& element) -> bool { return index_of(move(element)) >= 0; }

        /// @brief Remove all elements from the list.
        void clear() {
            free_nodes();
            m_first = m_last = nullptr;
            m_size           = 0;
        }

        /// @brief
        /// @param idx Idx of an element.
        /// @return The element at index or nullptr if the list is empty or the index is out of
        ///         bounds.
        auto operator[](size_t idx) const -> T& pre(idx < m_size) {
            size_t   c_idx = 0;
            Node<T>* curr  = m_first;
            while (curr && c_idx < idx) {
                curr = curr->m_next;
                c_idx++;
            }
            return curr->m_element;
        }

        [[nodiscard]] auto begin() const -> LinkedListIterator<T> {
            return LinkedListIterator<T>(m_first);
        }

        [[nodiscard]] auto end() const -> LinkedListIterator<T> {
            return LinkedListIterator<T>(nullptr);
        }
    };
} // namespace Rune

#endif // RUNEOS_LINKEDLIST_H
