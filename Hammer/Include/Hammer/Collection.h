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

#ifndef RUNEOS_COLLECTION_H
#define RUNEOS_COLLECTION_H


#include <Hammer/Utility.h>
#include <Hammer/Memory.h>


namespace Rune {

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                          Hash Definition and Hash Implementations for basic types
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

    /**
     * @brief Provides hash support for a type.
     *
     * A hash implementation must satisfy the following requirements:
     * <ol>
     * <li><a href="https://en.cppreference.com/w/cpp/named_req/DefaultConstructible">DefaultConstructible</a></li>
     * <li><a href="https://en.cppreference.com/w/cpp/named_req/CopyAssignable>CopyAssignable</a></li>
     * <li>Implement the "size_t operator()(const int& key) const" function making the hash calculation.</li>
     * </ol>
     * @tparam K Hash type.
     */
    template<class K>
    struct Hash;


    template<>
    struct Hash<signed char> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const signed char& key) const {
            return key;
        }
    };


    template<>
    struct Hash<char> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const char& key) const {
            return key;
        }
    };


    template<>
    struct Hash<short> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const short& key) const {
            return key;
        }
    };


    template<>
    struct Hash<int> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const int& key) const {
            return key;
        }
    };


    template<>
    struct Hash<long> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const long& key) const {
            return key;
        }
    };


    template<>
    struct Hash<long long> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const long long& key) const {
            return key;
        }
    };


    template<>
    struct Hash<unsigned char> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const unsigned char& key) const {
            return key;
        }
    };


    template<>
    struct Hash<unsigned short> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const unsigned short& key) const {
            return key;
        }
    };


    template<>
    struct Hash<unsigned int> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const unsigned int& key) const {
            return key;
        }
    };


    template<>
    struct Hash<unsigned long> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const unsigned long& key) const {
            return key;
        }
    };


    template<>
    struct Hash<unsigned long long> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const unsigned long long& key) const {
            return key;
        }
    };


    template<>
    struct Hash<float> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const float& key) const {
            // Calc hash for floats up to 10 digit precision
            float            num   = key;
            static const int pow10 = 1000000000;
            long             whole = (long) num;
            float            frac  = (num - (float) whole) * (float) pow10;
            size_t           hash  = 7 * whole + (size_t) (7 * frac);
            return hash;
        }
    };


    template<>
    struct Hash<double> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const double& key) const {
            // Calc hash for floats up to 10 digit precision
            double           num   = key;
            static const int pow10 = 1000000000;
            long             whole = (long) num;
            double           frac  = (num - (double) whole) * (double) pow10;
            size_t           hash  = 7 * whole + (size_t) (7 * frac);
            return hash;
        }
    };


    template<>
    struct Hash<long double> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const long double& key) const {
            // Calc hash for floats up to 10 digit precision
            long double      num   = key;
            static const int pow10 = 1000000000;
            long             whole = (long) num;
            long double      frac  = (num - (long double) whole) * (long double) pow10;
            size_t           hash  = 7 * whole + (size_t) (7 * frac);
            return hash;
        }
    };


    template<>
    struct Hash<bool> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const bool& key) const {
            return (size_t) key;
        }
    };


    template<>
    struct Hash<const char*> {

        Hash& operator=(Hash&& o) = default;


        Hash& operator=(const Hash& o) = default;


        size_t operator()(const char* key) const {
            size_t size = 0;
            const char* c_pos = key;
            while (*c_pos) {
                c_pos++;
                size++;
            }

            size_t      hash = 2383;
            for (size_t i    = 0; i < size; i++)
                hash += 101 * key[i];
            return hash;
        }
    };


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          Hashmap
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * @brief A hash map entry in a bucket.
     * @tparam K
     * @tparam V
     */
    template<typename K, typename V>
    struct HashNode {
        HashNode<K, V>* next;
        K key;
        V value;


        explicit HashNode(K key, V value) : key(move(key)), value(move(value)) {

        }
    };


    /**
     * @brief A key-value pair returned by the HashMapIterator.
     * @tparam K
     * @tparam V
     */
    template<typename K, typename V>
    struct Pair {
        K* key;
        V* value;
    };


    /**
     * @brief An interator over a hashmap.
     * @tparam K
     * @tparam V
     */
    template<typename K, typename V>
    class HashMapIterator {
        HashNode<K, V>** _bucket;
        size_t _bucket_count;
        size_t _bucket_pos;

        HashNode<K, V>* _current;
        Pair<K, V> _pair;
    public:
        explicit HashMapIterator(HashNode<K, V>** bucket, size_t bucket_count, size_t bucket_pos, HashNode<K, V>* node)
                :
                _bucket(bucket),
                _bucket_count(bucket_count),
                _bucket_pos(bucket_pos),
                _current(node),
                _pair({ node ? &node->key : nullptr, node ? &node->value : nullptr }) {


        }


        [[nodiscard]] bool has_next() const {
            return _current;
        }


        const Pair<K, V>& operator*() const {
            return _pair;
        }


        const Pair<K, V>* operator->() const {
            return _current ? &_pair : nullptr;
        }


        // pre-increment
        HashMapIterator<K, V>& operator++() {
            _current        = _current->next;
            if (!_current) { // search for next bucket
                for (size_t i = _bucket_pos + 1; i < _bucket_count; i++) {
                    HashNode<K, V>* c = _bucket[i];
                    if (c) {
                        _current    = c;
                        _bucket_pos = i;
                        break;
                    }
                }
            }
            if (!_current)
                _bucket_pos = _bucket_count;
            _pair           = { _current ? &_current->key : nullptr, _current ? &_current->value : nullptr };
            return *this;
        }


        //post-increment
        HashMapIterator<K, V> operator++(int) {
            HashMapIterator<K, V> tmp = *this;
            ++(*this);
            return tmp;
        }


        bool operator==(const HashMapIterator<K, V>& o) const {
            return _current == o._current && _bucket_pos == _bucket_count;
        }


        bool operator!=(const HashMapIterator<K, V>& o) const {
            return _current != o._current || _bucket_pos != _bucket_count;
        }
    };


    /**
     * Simple hashmap implementation.
     *
     * @tparam K Key type.
     * @tparam V Value type.
     */
    template<typename K, typename V>
    class HashMap {
        double _load_factor;

        size_t _bucket_count;
        HashNode<K, V>** _bucket;
        size_t _size;

        Hash<K> _hash;


        size_t calc_hash(const K& key, size_t size) const {
            return _hash(key) % size;
        }


        // Create a bigger bucket array and rehash all entries.
        void rehash(size_t new_bucket_count) {
            auto** new_bucket = new HashNode<K, V>* [new_bucket_count];
            // Important: Initialize with nullptr -> The data comes from the heap and will contain random values
            //            This leads to undefined behavior
            for (size_t i = 0; i < new_bucket_count; i++)
                new_bucket[i] = nullptr;

            for (size_t i = 0; i < _bucket_count; i++) {
                HashNode<K, V>* node = _bucket[i];
                while (node) {
                    HashNode<K, V>* next = node->next;
                    int new_hash = calc_hash(node->key, new_bucket_count);

                    HashNode<K, V>* old_head = new_bucket[new_hash];
                    node->next = old_head;
                    new_bucket[new_hash] = node;
                    node = next;
                }
            }

            auto** old_bucket = _bucket;
            _bucket_count = new_bucket_count;
            _bucket       = new_bucket;
            delete old_bucket;
        }


        HashMapIterator<K, V> put0(HashNode<K, V>* node) {
            if ((double) (_size + 1) / _bucket_count > _load_factor)
                rehash(2 * _bucket_count);

            int hash = calc_hash(node->key, _bucket_count);
            HashNode<K, V>* old_head = _bucket[hash];
            node->next = old_head;
            _bucket[hash] = node;
            _size++;
            return HashMapIterator<K, V>(_bucket, _bucket_count, hash, node);
        }


        bool remove0(const K& key) {
            int hash = calc_hash(key, _bucket_count);
            HashNode<K, V>* node = _bucket[hash];
            HashNode<K, V>* last = nullptr;
            while (node) {
                if (node->key == key) {
                    if (last)
                        last->next = node->next;
                    else
                        _bucket[hash] = node->next;
                    delete node;
                    _size--;
                    return true;
                }

                last = node;
                node = node->next;
            }
            return false;
        }


        HashMapIterator<K, V> find0(const K& key) const {
            int hash = calc_hash(key, _bucket_count);
            HashNode<K, V>* node = _bucket[hash];
            while (node) {
                if (node->key == key)
                    return HashMapIterator<K, V>(_bucket, _bucket_count, hash, node);
                node = node->next;
            }
            return HashMapIterator<K, V>(_bucket, _bucket_count, _bucket_count, nullptr);
        }


        void free_nodes() {
            for (size_t i = 0; i < _bucket_count; i++) {
                HashNode<K, V>* node = _bucket[i];
                while (node) {
                    HashNode<K, V>* next = node->next;
                    delete node;
                    node = next;
                }
            }
            delete _bucket;
        }


        void copy(const HashMap<K, V>& o) {
            _load_factor  = o._load_factor;
            _bucket_count = o._bucket_count;
            _bucket       = new HashNode<K, V>* [_bucket_count];
            _size         = o._size;
            _hash         = o._hash;

            if (!o._bucket)
                // Other hash map has not run "perform_lazy_init" yet
                return;

            for (size_t i = 0; i < _bucket_count; i++) {
                HashNode<K, V>* o_curr = o._bucket[i];
                HashNode<K, V>* t_prev = nullptr;
                while (o_curr) {
                    auto* t_curr = new HashNode<K, V>(o_curr->key, o_curr->value);
                    if (t_prev != nullptr)
                        t_prev->next = t_curr;

                    if (o_curr == o._bucket[i])
                        _bucket[i] = t_curr;

                    t_prev = t_curr;
                    o_curr = o_curr->next;
                }
            }
        }


        /**
         * @brief Dynamically allocate the buckets.
         *
         * We will perform a lazy allocation because the heap may or may not be ready at the point when a hash map will
         * be created (e.g. when calling global constructors in the kernel).
         */
        void perform_lazy_init() {
            if (!_bucket) {
                _bucket = new HashNode<K, V>* [_bucket_count];
                memset(_bucket, 0, sizeof(_bucket) * _bucket_count);
            }
        }


    public:

        /**
         * Init a new hashmap with a bucket count of 4.
         *
         * @param hashFunc
         */
        explicit HashMap() :
                _load_factor(0.75),
                _bucket_count(4),
                _bucket(nullptr),
                _size(0),
                _hash(Hash<K>{ }) {

        }


        /**
         * Init a new hashmap with given bucket count.
         *
         * @param hashFunc
         */
        explicit HashMap(size_t bucket_count) :
                _load_factor(0.75),
                _bucket_count(bucket_count),
                _bucket(nullptr),
                _size(0),
                _hash(Hash<K>{ }) {

        }


        /**
         * Init a new hashmap with given bucket count and hash function.
         *
         * @param hashFunc
         */
        HashMap(const Hash<K>& hash, size_t bucket_count) :
                _load_factor(0.75),
                _bucket_count(bucket_count),
                _bucket(nullptr),
                _size(0),
                _hash(Move(hash)) {

        }


        ~HashMap() {
            free_nodes();
        }


        HashMap(const HashMap<K, V>& o) noexcept: _load_factor(0.0), _bucket_count(0), _size(0) {
            copy(o);
        }


        HashMap& operator=(const HashMap<K, V>& o) noexcept {
            if (this == &o)
                return *this;

            free_nodes();
            copy(o);

            return *this;
        }


        HashMap(HashMap<K, V>&& o) noexcept {
            _load_factor  = o._load_factor;
            _bucket_count = o._bucket_count;
            swap(_bucket, o._bucket);
            _size = o._size;
            _hash = o._hash;

            o._load_factor  = 0.0;
            o._bucket_count = 0;
            o._bucket       = nullptr;
            o._size         = 0;
        }


        HashMap& operator=(HashMap<K, V>&& o) noexcept {
            _load_factor  = o._load_factor;
            _bucket_count = o._bucket_count;
            swap(_bucket, o._bucket);
            _size = o._size;
            _hash = o._hash;

            o._load_factor  = 0.0;
            o._bucket_count = 0;
            o._bucket       = nullptr;
            o._size         = 0;
            return *this;
        }


        /**
         *
         * @return Number of entries in the hashmap.
         */
        [[nodiscard]] size_t size() const {
            return _size;
        }


        /**
         *
         * @return True if the hashmap contains at least one entry.
         */
        [[nodiscard]] bool is_empty() const {
            return _size == 0;
        }


        /**
         *
         * @return Number of buckets that contain entries with the same hash.
         */
        [[nodiscard]] size_t get_bucket_count() const {
            return _bucket_count;
        }


        /**
         * Add a new entry mapping the given key to the value.
         *
         * @param key
         * @param value
         *
         * @return Iterator pointing to the new entry.
         */
        HashMapIterator<K, V> put(const K& key, const V& value) {
            perform_lazy_init();
            return put0(new HashNode<K, V>(move(key), move(value)));
        }


        /**
         * Add a new entry mapping the given key to the value.
         *
         * @param key
         * @param value
         *
         * @return Iterator pointing to the new entry.
         */
        HashMapIterator<K, V> put(K&& key, V&& value) {
            perform_lazy_init();
            return put0(new HashNode<K, V>(move(key), move(value)));
        }


        /**
         * Remove the key and associated value from the hash map.
         *
         * @param key
         *
         * @return True if the key is no longer in the hash map.
         */
        bool remove(const K& key) {
            if (!_bucket)
                return false;
            return remove0(key);
        }


        /**
         * Remove the key and associated value from the hash map.
         *
         * @param key
         *
         * @return True if the key is no longer in the hash map.
         */
        bool remove(K&& key) {
            if (!_bucket)
                return false;
            return remove0(key);
        }


        /**
         * Try to find the mapping for the key.
         *
         * @param key
         *
         * @return If the key is found, an iterator pointing to the mapping else an iterator pointing to "end()".
         */
        HashMapIterator<K, V> find(const K& key) const {
            if (!_bucket)
                return end();
            return find0(key);
        }


        /**
         * Try to find the mapping for the key.
         *
         * @param key
         *
         * @return If the key is found, an iterator pointing to the mapping else an iterator pointing to "end()".
         */
        HashMapIterator<K, V> find(K&& key) const {
            if (!_bucket)
                return end();
            return find0(key);
        }


        HashMapIterator<K, V> begin() const {
            if (!_bucket)
                return end();

            for (size_t i = 0; i < _bucket_count; i++)
                if (_bucket[i])
                    return HashMapIterator<K, V>(_bucket, _bucket_count, i, _bucket[i]);
            return end();
        }


        HashMapIterator<K, V> end() const {
            return HashMapIterator<K, V>(_bucket, _bucket_count, _bucket_count, nullptr);
        }
    };


    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
    //                                          LinkedList
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//


    /**
     * @brief A linked list node with an element.
     * @tparam T
     */
    template<typename T>
    struct Node {
        T element;
        Node<T>* previous = nullptr;
        Node<T>* next     = nullptr;


        explicit Node(T element) : element(move(element)) {

        }
    };


    template<typename T>
    class LinkedListIterator {
        Node<T>* _current;
    public:
        explicit LinkedListIterator(Node<T>* ptr) : _current(ptr) {

        }


        [[nodiscard]] bool has_next() const {
            return _current;
        }


        T& operator*() const {
            return _current->element;
        }


        T* operator->() const {
            return _current ? &_current->element : nullptr;
        }


        // pre-increment
        LinkedListIterator<T>& operator++() {
            _current = _current->next;
            return *this;
        }


        //post-increment
        LinkedListIterator<T> operator++(int) {
            LinkedListIterator tmp = *this;
            ++(*this);
            return tmp;
        }


        bool operator==(const LinkedListIterator& o) const {
            return _current == o._current;
        }


        bool operator!=(const LinkedListIterator& o) const {
            return _current != o._current;
        }
    };


    /**
     * Linked list implementation.
     */
    template<typename T>
    class LinkedList {
        Node<T>* _head;
        Node<T>* _tail;
        size_t _size;


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
                    _head = node;
                } else {
                    node->previous = _tail;
                    _tail->next    = node;
                    _tail = node;
                }
            }
            _size++;
        }


        bool remove0(Node<T>* node) {
            if (node == nullptr)
                return false;

            if (node->previous != nullptr)
                node->previous->next = node->next;
            else
                _head = node->next;

            if (node->next != nullptr)
                node->next->previous = node->previous;
            else
                _tail = node->previous;

            if (node->next == nullptr && node->previous == nullptr)
                _head = _tail = nullptr;

            node->previous = nullptr;
            node->next     = nullptr;
            delete node;
            _size--;
            return true;
        }


        bool contains0(const T& element) {
            Node<T>* current = _head;
            while (current != nullptr) {
                if (current->Element == element)
                    return true;
                current = current->Next;
            }
            return false;
        }


        void free_nodes() {
            Node<T>* c = _head;
            while (c) {
                Node<T>* n = c->next;
                delete c;
                c = n;
            }
        }


        void copy(const LinkedList<T>& o) {
            if (o._size == 0) {
                _head = _tail = nullptr;
                _size = 0;
                return;
            }

            Node<T>* o_curr = o._head;
            Node<T>* t_prev = nullptr;
            while (o_curr) {
                Node<T>* t_curr = new Node(o_curr->element);

                if (o_curr->previous != nullptr) {
                    t_curr->previous = t_prev;
                    if (t_prev != nullptr)
                        t_prev->next = t_curr;
                }


                if (o_curr == o._head)
                    _head = t_curr;
                if (o_curr == o._tail)
                    _tail = t_curr;

                o_curr = o_curr->next;
                t_prev = t_curr;
            }
            _size           = o._size;
        }


    public:
        explicit LinkedList() : _head(nullptr), _tail(nullptr), _size(0) {

        }


        ~LinkedList() {
            free_nodes();
        }


        LinkedList(const LinkedList<T>& o) noexcept: _size(0) {
            copy(o);
        }


        LinkedList& operator=(const LinkedList<T>& o) noexcept {
            if (this == &o)
                return *this;

            free_nodes();
            copy(o);
            return *this;
        }


        LinkedList(LinkedList<T>&& o) noexcept {
            swap(_head, o._head);
            swap(_tail, o._tail);
            _size = o._size;

            o._head = nullptr;
            o._tail = nullptr;
            o._size = 0;
        }


        LinkedList& operator=(LinkedList<T>&& o) noexcept {
            swap(_head, o._head);
            swap(_tail, o._tail);
            _size = o._size;
            o._size = 0;
            return *this;
        }


        /**
         *
         * @return First element.
         */
        [[nodiscard]] T* head() const {
            return _head ? &_head->element : nullptr;
        }


        /**
         *
         * @return Last element.
         */
        [[nodiscard]] T* tail() const {
            return _tail ? &_tail->element : nullptr;
        }


        /**
         *
         * @return True if the list contains no elements.
         */
        [[nodiscard]] bool is_empty() const {
            return _head == nullptr;
        }


        /**
         *
         * @return Number of elements in the list.
         */
        [[nodiscard]] size_t size() const {
            return _size;
        }


        /**
         * Add the element to the end of the list.
         *
         * @param element
         */
        void add_back(const T& element) {
            add0(new Node<T>(move(element)), false);
        }


        /**
         * Add the element to the end of the list.
         *
         * @param element
         */
        void add_back(T&& element) {
            add0(new Node<T>(move(element)), false);
        }


        /**
         * Add the element to the front of the list.
         *
         * @param element
         */
        void add_front(const T& element) {
            add0(new Node<T>(move(element)), true);
        }


        /**
         * Add the element to the front of the list.
         *
         * @param element
         */
        void add_front(T&& element) {
            add0(new Node<T>(move(element)), true);
        }


        /**
         * Add all elements of the other list to the end of this list.
         *
         * @param other
         */
        void add_all(const LinkedList<T>& other) {
            for (T& element: other)
                add_back(element);
        }


        /**
         * Remove the first element.
         */
        bool remove_front() {
            return remove0(_head);
        }


        /**
         * Remove the last element.
         */
        bool remove_back() {
            return remove0(_tail);
        }


        /**
         * Remove the element from the list.
         *
         * @param element
         */
        bool remove(const T& element) {
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
        bool remove(T&& element) {
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
        bool remove_at(size_t index) {
            if (index >= _size)
                return false;
            size_t i = 0;
            Node<T>* to_remove = _head;
            while (i < index) {
                to_remove = to_remove->next;
                i++;
            }
            return remove0(to_remove);
        }


        /**
         *
         * @return True: The element is in the list, False: Is not in the list.
         */
        bool contains(const T& element) {
            return contains0(element);
        }


        /**
         *
         * @return True: The element is in the list, False: Is not in the list.
         */
        bool contains(T&& element) {
            return contains0(element);
        }


        /**
         * Remove all elements from the list.
         */
        void clear() {
            free_nodes();
            _head = _tail = nullptr;
            _size = 0;
        }


        /**
         *
         * @param idx Idx of an element.
         *
         * @return The element at index or nullptr if the list is empty or the index is out of bounds.
         */
        T* operator[](size_t idx) const {
            if (idx >= _size)
                return nullptr;

            size_t i = 0;
            Node<T>* curr = _head;
            while (curr && i < idx) {
                curr = curr->next;
                i++;
            }
            return curr ? &(curr->element) : nullptr;
        }


        LinkedListIterator<T> begin() const {
            return LinkedListIterator<T>(_head);
        }


        LinkedListIterator<T> end() const {
            return LinkedListIterator<T>(nullptr);
        }
    };
}

#endif //RUNEOS_COLLECTION_H
