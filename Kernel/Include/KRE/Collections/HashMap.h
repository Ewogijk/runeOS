//
// Created by ewogijk on 9/23/25.
//

#ifndef RUNEOS_DICTIONARY_H
#define RUNEOS_DICTIONARY_H

#include <KRE/CppLanguageSupport.h>
#include <KRE/Utility.h>

namespace Rune {

    /**
     * @brief A hash map entry in a bucket.
     * @tparam K
     * @tparam V
     */
    template <typename K, typename V>
    struct HashNode {
        HashNode<K, V>* next;
        K               key;
        V               value;

        explicit HashNode(K key, V value) : key(move(key)), value(move(value)) {}
    };

    /**
     * @brief A key-value pair returned by the HashMapIterator.
     * @tparam K
     * @tparam V
     */
    template <typename K, typename V>
    struct Pair {
        K* key;
        V* value;
    };

    /**
     * @brief An interator over a hashmap.
     * @tparam K
     * @tparam V
     */
    template <typename K, typename V>
    class HashMapIterator {
        HashNode<K, V>** _bucket;
        size_t           _bucket_count{};
        size_t           _bucket_pos{};

        HashNode<K, V>* _current;
        Pair<K, V>      _pair;

      public:
        explicit HashMapIterator(HashNode<K, V>** bucket,
                                 size_t           bucket_count,
                                 size_t           bucket_pos,
                                 HashNode<K, V>*  node)
            : _bucket(bucket),
              _bucket_count(bucket_count),
              _bucket_pos(bucket_pos),
              _current(node),
              _pair({node ? &node->key : nullptr, node ? &node->value : nullptr}) {}

        [[nodiscard]]
        auto has_next() const -> bool {
            return _current;
        }

        auto operator*() const -> const Pair<K, V>& { return _pair; }

        auto operator->() const -> const Pair<K, V>* { return _current ? &_pair : nullptr; }

        // pre-increment
        auto operator++() -> HashMapIterator<K, V>& {
            _current = _current->next;
            if (_current == nullptr) { // search for next bucket
                for (size_t i = _bucket_pos + 1; i < _bucket_count; i++) {
                    HashNode<K, V>* curr = _bucket[i];
                    if (curr) {
                        _current    = curr;
                        _bucket_pos = i;
                        break;
                    }
                }
            }
            if (_current == nullptr) {
                _bucket_pos = _bucket_count;
            }
            _pair = {_current ? &_current->key : nullptr, _current ? &_current->value : nullptr};
            return *this;
        }

        // post-increment
        auto operator++(int) -> HashMapIterator<K, V> {
            HashMapIterator<K, V> tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator==(const HashMapIterator<K, V>& other) const -> bool {
            return _current == other._current && _bucket_pos == _bucket_count;
        }

        auto operator!=(const HashMapIterator<K, V>& other) const -> bool {
            return _current != other._current || _bucket_pos != _bucket_count;
        }
    };

    /**
     * @brief An interator over the values of a hashmap.
     * @tparam K
     * @tparam V
     */
    template <typename K, typename V>
    class HashMapValueIterator {
        HashMapIterator<K, V> _iter;

      public:
        explicit HashMapValueIterator(const HashMapIterator<K, V>& iter) : _iter(iter) {}

        [[nodiscard]]
        auto has_next() const -> bool {
            return _iter.has_next();
        }

        auto operator*() const -> const V& { return *(*_iter).value; }

        auto operator->() const -> const V* { return _iter.has_next() ? _iter->value : nullptr; }

        // pre-increment
        auto operator++() -> HashMapValueIterator<K, V>& {
            ++_iter;
            return *this;
        }

        // post-increment
        auto operator++(int) -> HashMapValueIterator<K, V> {
            HashMapIterator<K, V> tmp = *this;
            ++(*this);
            return tmp;
        }

        auto operator==(const HashMapValueIterator<K, V>& other) const -> bool {
            return _iter == other._iter;
        }

        auto operator!=(const HashMapValueIterator<K, V>& other) const -> bool {
            return _iter != other._iter;
        }
    };

    /**
     * A view over the values of a hashmap.
     * @tparam K
     * @tparam V
     */
    template <typename K, typename V>
    class HashMapValueView {
        HashNode<K, V>** _bucket;
        size_t           _bucket_count;

      public:
        explicit HashMapValueView(HashNode<K, V>** bucket, size_t bucket_count)
            : _bucket(bucket),
              _bucket_count(bucket_count) {}

        auto begin() const -> HashMapValueIterator<K, V> {
            if (_bucket == nullptr) {
                return end();
            }

            for (size_t i = 0; i < _bucket_count; i++) {
                if (_bucket[i]) {
                    return HashMapValueIterator<K, V>(
                        HashMapIterator<K, V>(_bucket, _bucket_count, i, _bucket[i]));
                }
            }
            return end();
        }

        auto end() const -> HashMapValueIterator<K, V> {
            return HashMapValueIterator<K, V>(
                HashMapIterator<K, V>(_bucket, _bucket_count, _bucket_count, nullptr));
        }
    };

    /**
     * Simple hashmap implementation.
     *
     * @tparam K Key type.
     * @tparam V Value type.
     */
    template <typename K, typename V>
    class HashMap {
        static constexpr double DEFAULT_LOAD_FACTOR  = 0.75;
        static constexpr size_t DEFAULT_BUCKET_COUNT = 4;
        double                  _load_factor;

        size_t           _bucket_count;
        HashNode<K, V>** _bucket;
        size_t           _size;

        Hash<K> _hash;

        auto calc_hash(const K& key, size_t size) const -> size_t { return _hash(key) % size; }

        // Create a bigger bucket array and rehash all entries.
        void rehash(size_t new_bucket_count) {
            auto** new_bucket = new HashNode<K, V>*[new_bucket_count];
            // Important: Initialize with nullptr -> The data comes from the heap and will contain
            // random values
            //            This leads to undefined behavior
            for (size_t i = 0; i < new_bucket_count; i++) {
                new_bucket[i] = nullptr;
            }

            for (size_t i = 0; i < _bucket_count; i++) {
                HashNode<K, V>* node = _bucket[i];
                while (node) {
                    HashNode<K, V>* next     = node->next;
                    int             new_hash = calc_hash(node->key, new_bucket_count);

                    HashNode<K, V>* old_head = new_bucket[new_hash];
                    node->next               = old_head;
                    new_bucket[new_hash]     = node;
                    node                     = next;
                }
            }

            auto** old_bucket = _bucket;
            _bucket_count     = new_bucket_count;
            _bucket           = new_bucket;
            delete old_bucket;
        }

        auto put0(HashNode<K, V>* node) -> HashMapIterator<K, V> {
            if ((double) (_size + 1) / _bucket_count > _load_factor) {
                rehash(2 * _bucket_count);
            }

            int             hash     = calc_hash(node->key, _bucket_count);
            HashNode<K, V>* old_head = _bucket[hash];
            node->next               = old_head;
            _bucket[hash]            = node;
            _size++;
            return HashMapIterator<K, V>(_bucket, _bucket_count, hash, node);
        }

        auto remove0(const K& key) -> bool {
            int             hash = calc_hash(key, _bucket_count);
            HashNode<K, V>* node = _bucket[hash];
            HashNode<K, V>* last = nullptr;
            while (node) {
                if (node->key == key) {
                    if (last) {
                        last->next = node->next;
                    } else {
                        _bucket[hash] = node->next;
                    }
                    delete node;
                    _size--;
                    return true;
                }

                last = node;
                node = node->next;
            }
            return false;
        }

        auto find0(const K& key) const -> HashMapIterator<K, V> {
            int             hash = calc_hash(key, _bucket_count);
            HashNode<K, V>* node = _bucket[hash];
            while (node) {
                if (node->key == key) {
                    return HashMapIterator<K, V>(_bucket, _bucket_count, hash, node);
                }
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
                _bucket[i] = nullptr;
            }
            delete _bucket;
        }

        void copy(const HashMap<K, V>& other) {
            _load_factor  = other._load_factor;
            _bucket_count = other._bucket_count;
            _size         = other._size;
            _hash         = other._hash;
            _bucket       = new HashNode<K, V>*[_bucket_count];
            for (size_t i = 0; i < _bucket_count; i++) _bucket[i] = nullptr;

            if (other._bucket == nullptr) {
                // Other hash map has not run "perform_lazy_init" yet
                return;
            }

            for (size_t i = 0; i < _bucket_count; i++) {
                HashNode<K, V>* o_curr = other._bucket[i];
                HashNode<K, V>* t_prev = nullptr;
                while (o_curr) {
                    auto* t_curr = new HashNode<K, V>(o_curr->key, o_curr->value);
                    if (t_prev != nullptr) {
                        t_prev->next = t_curr;
                    }
                    if (o_curr == other._bucket[i]) {
                        _bucket[i] = t_curr;
                    }
                    t_prev = t_curr;
                    o_curr = o_curr->next;
                }
            }
        }

        /**
         * @brief Dynamically allocate the buckets.
         *
         * We will perform a lazy allocation because the heap may or may not be ready at the point
         * when a hash map will be created (e.g. when calling global constructors in the kernel).
         */
        void perform_lazy_init() {
            if (_bucket == nullptr) {
                _bucket = new HashNode<K, V>*[_bucket_count];
                memset(_bucket, 0, sizeof(_bucket) * _bucket_count);
            }
        }

      public:
        /**
         * Init a new hashmap with a bucket count of 4.
         *
         * @param hashFunc
         */
        explicit HashMap()
            : _load_factor(DEFAULT_LOAD_FACTOR),
              _bucket_count(DEFAULT_BUCKET_COUNT),
              _bucket(nullptr),
              _size(0),
              _hash(Hash<K>{}) {}

        /**
         * Init a new hashmap with given bucket count.
         *
         * @param hashFunc
         */
        explicit HashMap(size_t bucket_count)
            : _load_factor(DEFAULT_LOAD_FACTOR),
              _bucket_count(bucket_count),
              _bucket(nullptr),
              _size(0),
              _hash(Hash<K>{}) {}

        /**
         * Init a new hashmap with given bucket count and hash function.
         *
         * @param hashFunc
         */
        HashMap(const Hash<K>& hash, size_t bucket_count)
            : _load_factor(DEFAULT_LOAD_FACTOR),
              _bucket_count(bucket_count),
              _bucket(nullptr),
              _size(0),
              _hash(Move(hash)) {}

        ~HashMap() { free_nodes(); }

        HashMap(const HashMap<K, V>& other) noexcept
            : _load_factor(0.0),
              _bucket_count(0),
              _size(0) {
            copy(other);
        }

        auto operator=(const HashMap<K, V>& other) noexcept -> HashMap& {
            if (this == &other) {
                return *this;
            }

            free_nodes();
            copy(other);

            return *this;
        }

        HashMap(HashMap<K, V>&& other) noexcept
            : _bucket_count(other._bucket_count),
              _load_factor(other._load_factor),
              _size(other._size),
              _hash(other._hash) {
            swap(_bucket, other._bucket);
            other._load_factor  = 0.0;
            other._bucket_count = 0;
            other._bucket       = nullptr;
            other._size         = 0;
        }

        auto operator=(HashMap<K, V>&& other) noexcept -> HashMap& {
            _load_factor  = other._load_factor;
            _bucket_count = other._bucket_count;
            swap(_bucket, other._bucket);
            _size = other._size;
            _hash = other._hash;

            other._load_factor  = 0.0;
            other._bucket_count = 0;
            other._bucket       = nullptr;
            other._size         = 0;
            return *this;
        }

        /**
         *
         * @return Number of entries in the hashmap.
         */
        [[nodiscard]] auto size() const -> size_t { return _size; }

        /**
         *
         * @return True if the hashmap contains at least one entry.
         */
        [[nodiscard]] auto is_empty() const -> bool { return _size == 0; }

        /**
         *
         * @return Number of buckets that contain entries with the same hash.
         */
        [[nodiscard]] auto get_bucket_count() const -> size_t { return _bucket_count; }

        /**
         *
         * @return A view of the values in the hashmap.
         */
        auto values() const -> HashMapValueView<K, V> {
            return HashMapValueView<K, V>(_bucket, _bucket_count);
        }

        /**
         * Add a new entry mapping the given key to the value.
         *
         * @param key
         * @param value
         *
         * @return Iterator pointing to the new entry.
         */
        auto put(const K& key, const V& value) -> HashMapIterator<K, V> {
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
        auto put(K&& key, V&& value) -> HashMapIterator<K, V> {
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
        auto remove(const K& key) -> bool {
            if (_bucket == nullptr) {
                return false;
            }
            return remove0(key);
        }

        /**
         * Remove the key and associated value from the hash map.
         *
         * @param key
         *
         * @return True if the key is no longer in the hash map.
         */
        auto remove(K&& key) -> bool {
            if (_bucket == nullptr) {
                return false;
            }
            return remove0(key);
        }

        /**
         * Try to find the mapping for the key.
         *
         * @param key
         *
         * @return If the key is found, an iterator pointing to the mapping else an iterator
         * pointing to "end()".
         */
        auto find(const K& key) const -> HashMapIterator<K, V> {
            if (_bucket == nullptr) {
                return end();
            }
            return find0(key);
        }

        /**
         * Try to find the mapping for the key.
         *
         * @param key
         *
         * @return If the key is found, an iterator pointing to the mapping else an iterator
         * pointing to "end()".
         */
        auto find(K&& key) const -> HashMapIterator<K, V> {
            if (_bucket == nullptr) {
                return end();
            }
            return find0(key);
        }

        auto operator[](const K& key) -> V& {
            perform_lazy_init();
            auto maybe_value = find(key);
            if (maybe_value != end()) return *maybe_value->value;
            maybe_value = put0(new HashNode<K, V>(move(key), V()));
            return *maybe_value->value;
        }

        auto begin() const -> HashMapIterator<K, V> {
            if (_bucket == nullptr) {
                return end();
            }

            for (size_t i = 0; i < _bucket_count; i++) {
                if (_bucket[i]) {
                    return HashMapIterator<K, V>(_bucket, _bucket_count, i, _bucket[i]);
                }
            }
            return end();
        }

        auto end() const -> HashMapIterator<K, V> {
            return HashMapIterator<K, V>(_bucket, _bucket_count, _bucket_count, nullptr);
        }
    };

} // namespace Rune
#endif // RUNEOS_DICTIONARY_H
