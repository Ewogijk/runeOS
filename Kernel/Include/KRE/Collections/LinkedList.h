//
// Created by ewogijk on 9/23/25.
//

#ifndef RUNEOS_LINKEDLIST_H
#define RUNEOS_LINKEDLIST_H

#include <KRE/CppRuntimeSupport.h>
#include <KRE/Utility.h>

namespace Rune {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          LinkedList
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief A linked list node with an element.
     * @tparam T
     */
    template <typename T>
    struct Node {
        T        element;
        Node<T>* previous = nullptr;
        Node<T>* next     = nullptr;

        explicit Node(T element) : element(move(element)) {}
    };

    template <typename T>
    class LinkedListIterator {
        Node<T>* _current;

      public:
        explicit LinkedListIterator(Node<T>* ptr) : _current(ptr) {}

        [[nodiscard]]
        auto has_next() const -> bool {
            return _current;
        }

        auto operator*() const -> T& { return _current->element; }

        auto operator->() const -> T* { return _current ? &_current->element : nullptr; }

        // pre-increment
        auto operator++() -> LinkedListIterator<T>& {
            _current = _current->next;
            return *this;
        }

        // post-increment
        auto operator++(int) -> LinkedListIterator<T> {
            LinkedListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator==(const LinkedListIterator& other) const -> bool {
            return _current == other._current;
        }

        auto operator!=(const LinkedListIterator& other) const -> bool {
            return _current != other._current;
        }
    };

    /**
     * Linked list implementation.
     */
    template <typename T>
    class LinkedList {
        Node<T>* _head;
        Node<T>* _tail;
        size_t   _size{0};

        // Add to front or back, add at index not needed atm
        void add0(Node<T>* node, bool front) {
            if (_head == nullptr) {
                node->next     = nullptr;
                node->previous = nullptr;
                _head = _tail = node;
            } else {
                if (front) {
                    node->next      = _head;
                    _head->previous = node;
                    _head           = node;
                } else {
                    node->previous = _tail;
                    _tail->next    = node;
                    _tail          = node;
                }
            }
            _size++;
        }

        auto remove0(Node<T>* node) -> bool {
            if (node == nullptr) {
                return false;
            }

            if (node->previous != nullptr) {
                node->previous->next = node->next;
            } else {
                _head = node->next;
            }

            if (node->next != nullptr) {
                node->next->previous = node->previous;
            } else {
                _tail = node->previous;
            }

            if (node->next == nullptr && node->previous == nullptr) {
                _head = _tail = nullptr;
            }

            node->previous = nullptr;
            node->next     = nullptr;
            delete node;
            _size--;
            return true;
        }

        auto contains0(const T& element) -> bool {
            Node<T>* current = _head;
            while (current != nullptr) {
                if (current->Element == element) {
                    return true;
                }
                current = current->Next;
            }
            return false;
        }

        void free_nodes() {
            Node<T>* curr = _head;
            while (curr) {
                Node<T>* next = curr->next;
                delete curr;
                curr = next;
            }
        }

        void copy(const LinkedList<T>& other) {
            if (other._size == 0) {
                _head = _tail = nullptr;
                _size         = 0;
                return;
            }

            Node<T>* o_curr = other._head;
            Node<T>* t_prev = nullptr;
            while (o_curr) {
                auto* t_curr = new Node<T>(o_curr->element);
                if (o_curr->previous != nullptr) {
                    t_curr->previous = t_prev;
                    if (t_prev != nullptr) {
                        t_prev->next = t_curr;
                    }
                }

                if (o_curr == other._head) {
                    _head = t_curr;
                }
                if (o_curr == other._tail) {
                    _tail = t_curr;
                }

                o_curr = o_curr->next;
                t_prev = t_curr;
            }
            _size = other._size;
        }

      public:
        explicit LinkedList() : _head(nullptr), _tail(nullptr) {}

        LinkedList(std::initializer_list<T> values) : _head(nullptr), _tail(nullptr) {
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

        LinkedList(LinkedList<T>&& other) noexcept : _size(other._size) {
            swap(_head, other._head);
            swap(_tail, other._tail);

            other._head = nullptr;
            other._tail = nullptr;
            other._size = 0;
        }

        auto operator=(LinkedList<T>&& other) noexcept -> LinkedList& {
            swap(_head, other._head);
            swap(_tail, other._tail);
            _size       = other._size;
            other._size = 0;
            return *this;
        }

        /**
         *
         * @return First element.
         */
        [[nodiscard]]
        auto head() const -> T* {
            return _head ? &_head->element : nullptr;
        }

        /**
         *
         * @return Last element.
         */
        [[nodiscard]]
        auto tail() const -> T* {
            return _tail ? &_tail->element : nullptr;
        }

        /**
         *
         * @return True if the list contains no elements.
         */
        [[nodiscard]]
        auto is_empty() const -> bool {
            return _head == nullptr;
        }

        /**
         *
         * @return Number of elements in the list.
         */
        [[nodiscard]]
        auto size() const -> size_t {
            return _size;
        }

        /**
         * Add the element to the end of the list.
         *
         * @param element
         */
        void add_back(const T& element) { add0(new Node<T>(move(element)), false); }

        /**
         * Add the element to the end of the list.
         *
         * @param element
         */
        void add_back(T&& element) { add0(new Node<T>(move(element)), false); }

        /**
         * Add the element to the front of the list.
         *
         * @param element
         */
        void add_front(const T& element) { add0(new Node<T>(move(element)), true); }

        /**
         * Add the element to the front of the list.
         *
         * @param element
         */
        void add_front(T&& element) { add0(new Node<T>(move(element)), true); }

        /**
         * Add all elements of the other list to the end of this list.
         *
         * @param other
         */
        void add_all(const LinkedList<T>& other) {
            for (T& element : other) {
                add_back(element);
            }
        }

        /**
         * Remove the first element.
         */
        auto remove_front() -> bool { return remove0(_head); }

        /**
         * Remove the last element.
         */
        auto remove_back() -> bool { return remove0(_tail); }

        /**
         * Remove the element from the list.
         *
         * @param element
         */
        auto remove(const T& element) -> bool {
            Node<T>* to_remove = _head;
            while (to_remove != nullptr && to_remove->element != element) {
                to_remove = to_remove->next;
            }
            return remove0(to_remove);
        }

        /**
         * Remove the element from the list.
         *
         * @param element
         */
        auto remove(T&& element) -> bool {
            Node<T>* to_remove = _head;
            while (to_remove != nullptr && to_remove->element != element) {
                to_remove = to_remove->next;
            }
            return remove0(to_remove);
        }

        /**
         * @brief Remove the element at requested index.
         * @param index
         * @return
         */
        auto remove_at(size_t index) -> bool {
            if (index >= _size) {
                return false;
            }
            size_t   idx       = 0;
            Node<T>* to_remove = _head;
            while (idx < index) {
                to_remove = to_remove->next;
                idx++;
            }
            return remove0(to_remove);
        }

        /**
         *
         * @return True: The element is in the list, False: Is not in the list.
         */
        auto contains(const T& element) -> bool { return contains0(element); }

        /**
         *
         * @return True: The element is in the list, False: Is not in the list.
         */
        auto contains(T&& element) -> bool { return contains0(move(element)); }

        /**
         * Remove all elements from the list.
         */
        void clear() {
            free_nodes();
            _head = _tail = nullptr;
            _size         = 0;
        }

        /**
         *
         * @param idx Idx of an element.
         *
         * @return The element at index or nullptr if the list is empty or the index is out of
         * bounds.
         */
        auto operator[](size_t idx) const -> T* {
            if (idx >= _size) {
                return nullptr;
            }

            size_t   c_idx = 0;
            Node<T>* curr  = _head;
            while (curr && c_idx < idx) {
                curr = curr->next;
                c_idx++;
            }
            return curr ? &(curr->element) : nullptr;
        }

        auto begin() const -> LinkedListIterator<T> { return LinkedListIterator<T>(_head); }

        auto end() const -> LinkedListIterator<T> { return LinkedListIterator<T>(nullptr); }
    };
} // namespace Rune

#endif // RUNEOS_LINKEDLIST_H
