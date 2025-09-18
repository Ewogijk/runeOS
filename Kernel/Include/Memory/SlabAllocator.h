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

#ifndef RUNEOS_SLABALLOCATOR_H
#define RUNEOS_SLABALLOCATOR_H

#include <Memory/Paging.h>
#include <Memory/VirtualMemoryManager.h>

namespace Rune::Memory {
    /**
     * Describes if slab meta information is stored on or off a slab.
     */
#define CACHE_TYPES(X)                                                                             \
    X(CacheType, ON_SLAB, 0x1)                                                                     \
    X(CacheType, OFF_SLAB, 0x2)

    DECLARE_TYPED_ENUM(CacheType, U8, CACHE_TYPES, 0x0) // NOLINT

/**
 * Reasons why the heap start can fail.
 */
#define HEAP_START_FAILURE_CODES(X)                                                                \
    X(HeapStartFailureCode, HEAP_NOT_MAPPED, 0x1)                                                  \
    X(HeapStartFailureCode, BC_OBJECT_CACHE_ERROR, 0x2)                                            \
    X(HeapStartFailureCode, BC_SLAB_ERROR, 0x3)                                                    \
    X(HeapStartFailureCode, BC_OBJECT_BUF_NODE_ERROR, 0x4)                                         \
    X(HeapStartFailureCode, BC_OBJECT_BUF_NODE_HASHMAP_ERROR, 0x5)                                 \
    X(HeapStartFailureCode, BC_HASHNODE_ERROR, 0x6)                                                \
    X(HeapStartFailureCode, BC_MEMORY_NODE_ERROR, 0x7)                                             \
    X(HeapStartFailureCode, ALLOC_GP_OR_DMA_CACHE_ERROR, 0x8)                                      \
    X(HeapStartFailureCode, GP_CACHE_ERROR, 0x9)                                                   \
    X(HeapStartFailureCode, DMA_CACHE_ERROR, 0xA)

    DECLARE_ENUM(HeapStartFailureCode, HEAP_START_FAILURE_CODES, 0x0) // NOLINT

    /**
     * A specialized hash map for ObjectBufNodes.
     */
    class ObjectBufNodeHashMap;

    /**
     * A slab stores allocated objects.
     */
    struct Slab;

    /**
     * Singly linkedlist of allocated objects.
     */
    struct ObjectBufNode {
        ObjectBufNode* next;
        void*          object;
        Slab*          owner;
    };

    /**
     * I forgot what this is!
     */
    union ObjectBuf {
        ObjectBufNode* regular_object;
        U8             free_object;
    };

    /**
     * A dynamically growing cache of objects of the size.
     */
    class ObjectCache;

    /**
     * See above.
     */
    struct Slab {
        static constexpr U8 MAX_OBJECT_COUNT =
            255; // And marker of the end of the free list (Object at index 255)

        Slab* next;
        Slab* prev;

        ObjectBuf free_buf;

        size_t object_size;
        size_t object_count;
        size_t allocated_count;

        void*  page;
        size_t slab_size;

        /**
         * Create an OnSlab slab that stores slab data on the slab itself.
         *
         * @param object_size
         * @param page
         * @return
         */
        static Slab* create_on_slab(size_t object_size, VirtualAddr page);

        /**
         * Creates an OffSlab cache that stores slab data externally.
         *
         * @param slab_cache
         * @param object_buf_node_cache
         * @param object_buf_node_hashmap
         * @param object_size
         * @param page
         * @param slab_size
         * @return
         */
        static Slab* create_off_slab(ObjectCache*          slab_cache,
                                     ObjectCache*          object_buf_node_cache,
                                     ObjectBufNodeHashMap* object_buf_node_hashmap,
                                     size_t                object_size,
                                     VirtualAddr           page,
                                     size_t                slab_size);

        /**
         * Make an OnSlab allocation.
         *
         * @return Pointer to the allocated object.
         */
        void* alloc_on_slab();

        /**
         * Make an OffSlab allocation.
         *
         * @return Pointer to the allocated object.
         */
        void* alloc_off_slab();

        /**
         * Make an OnSlab free.
         *
         * @return True: The object is freed, False: Not.
         */
        bool free_on_slab(void* obj);

        /**
         * Make an OffSlab free.
         *
         * @return True: The object is freed, False: Not.
         */
        bool free_off_slab(ObjectBufNode* obj_buf);
    };

    /**
     * Stores freed slab pages that lie between two used slab pages, these are prioritized for slab
     * allocation when an object cache should grow again.
     */
    struct MemoryNode {
        MemoryNode* next;
        VirtualAddr mem_addr;
    };

    /**
     * See above.
     */
    class ObjectCache {
        // Memory management
        VirtualMemoryManager* _vmm;
        ObjectCache*          _memory_node_cache;
        MemoryRegion          _managed;
        VirtualAddr           _limit;
        U16                   _page_flags;
        MemoryNode*           _free_page_list;

        // Object management
        ObjectCache*          _object_buf_node_cache;
        ObjectBufNodeHashMap* _object_buf_node_hash_map;
        size_t                _object_size;
        size_t                _align;

        // Slab management
        ObjectCache* _slab_cache;
        Slab*        _full_list;
        Slab*        _partial_list;
        Slab*        _empty_list;
        size_t       _slab_count;

        // Debug information
        CacheType _type;

        bool grow();

      public:
        ObjectCache();

        [[nodiscard]]
        MemoryRegion get_managed() const;

        [[nodiscard]]
        CacheType get_type() const;

        int8_t init(VirtualMemoryManager* vmm,
                    ObjectCache*          memory_node_cache,
                    MemoryRegion          managed,
                    U16                   page_flags,
                    ObjectCache*          object_buf_node_cache,
                    ObjectBufNodeHashMap* object_buf_node_hash_map,
                    size_t                object_size,
                    size_t                align,
                    ObjectCache*          slab_cache);

        /**
         * allocate an object in the cache, the object cache will grow if needed.
         *
         * @return A pointer to the allocated object.
         */
        void* allocate();

        /**
         * free an object in the cache, it is assumed that the given object is allocated in the
         * object cache. If not undefined behavior may occur.
         *
         * @param obj Pointer to the object to free.
         */
        void free(void* obj);

        /**
         * A object cache is essentially a dynamic array, so we can access objects by index.
         *
         * @param idx Index of an object.
         *
         * @return A pointer to the object.
         */
        void* object_at(U8 idx);

        /**
         * free all claimed memory of the object cache.
         */
        void destroy();
    };

    /**
     * Entry in the hashmap.
     */
    struct HashNode {
        HashNode*      next;
        void*          key;
        ObjectBufNode* value;
    };

    /**
     * See above.
     */
    class ObjectBufNodeHashMap {
        // Prime number to minimize hash collisions
        static constexpr U8 BUCKET_COUNT = 37;

        HashNode*    _nodes[BUCKET_COUNT];
        ObjectCache* _hash_node_cache;

      public:
        explicit ObjectBufNodeHashMap();

        void init(ObjectCache* hash_node_cache);

        void insert(void* key, ObjectBufNode* value);

        void remove(void* key);

        ObjectBufNode* get(void* key);

        void destroy(ObjectCache* object_buf_cache);
    };

    /**
     * The slab allocator manages object caches. It contains general purpose and DMA caches that can
     * allocate non-aligned objects of the size of a power of 2 from 16 byte - 64 KiB. Objects
     * caches of custom size and specific alignment can be requested.
     */
    class SlabAllocator {
        static constexpr U8         MIN_SIZE_POWER        = 4;
        static constexpr U8         STATIC_CACHE_COUNT    = 13;
        static constexpr U8         MIN_OBJ_SIZE          = 16;
        static constexpr MemorySize CACHE_SIZE            = 2 * 1048576; // 2 MiB
        static constexpr U8         BOOTSTRAP_CACHE_COUNT = 6;

        ObjectCache _object_cache_cache;
        ObjectCache _slab_cache;
        ObjectCache _object_buf_node_cache;
        ObjectCache _object_buf_node_hash_map_cache;
        ObjectCache _hash_node_cache;
        ObjectCache _memory_node_cache;

        ObjectCache* _general_purpose_cache[STATIC_CACHE_COUNT];
        ObjectCache* _dma_cache[STATIC_CACHE_COUNT];

        VirtualMemoryManager* _vmm;
        MemoryRegion          _heap_memory;
        VirtualAddr           _limit;
        MemoryNode*           _free_list;

        HeapStartFailureCode _start_failure_code;

        int8_t init_cache(ObjectCache*          cache,
                          size_t                obj_size,
                          size_t                align,
                          uint16_t              page_flags,
                          bool                  force_off_slab,
                          ObjectBufNodeHashMap* object_buf_node_hash_map);

      public:
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //
        // Constructors&Destructors
        //
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        SlabAllocator();

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //
        // Properties
        //
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        /**
         *
         * @return Size of the smallest general purpose/DMA cache.
         */
        [[nodiscard]]
        U32 get_min_cache_size() const;

        /**
         *
         * @return Size of the biggest general purpose/DMA cache.
         */
        [[nodiscard]]
        U32 get_max_cache_size() const;

        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//
        //
        // Slab Allocator Functions
        //
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++//

        [[nodiscard]]
        HeapStartFailureCode start(MemoryMap* v_map, VirtualMemoryManager* vmm);

        /**
         * allocate an object in a general purpose cache. The object size will rounded up to the
         * next power of 2 if needed. When the object is smaller than 16 bytes, it will be padded to
         * 16 bytes.
         *
         * @param size Object size.
         *
         * @return Pointer to the allocated object.
         */
        void* allocate(size_t size);

        /**
         * allocate an object in a DMA cache. The object size will rounded up to the next power of 2
         * if needed. When the object is smaller than 16 bytes, it will be padded to 16 bytes.
         *
         * @param size Object size.
         *
         * @return Pointer to the allocated object.
         */
        void* allocate_dma(size_t size);

        /**
         * free the object. The cache of the object is determined based on the pointer address and
         * it is assumed that the object is dynamically allocated. If not undefined behavior may
         * occur.
         *
         * @param obj Pointer to the object.
         */
        void free(void* obj);

        /**
         * Create a new object with requested configuration.
         *
         * @param object_size Size of the objects in the cache.
         * @param align       Memory alignment of the objects.
         * @param dma         Defines CPU caching strategies for paging. True: Use write-through and
         * dont cache pages, False: Use write-back and cache pages.
         *
         * @return A pointer to the object cache.
         */
        ObjectCache* create_new_cache(size_t object_size, size_t align, bool dma);

        /**
         * free all claimed memory of the object cache.
         *
         * @param cache Pointer to an object cache.
         */
        void destroy_cache(ObjectCache* cache);
    };
} // namespace Rune::Memory

#endif // RUNEOS_SLABALLOCATOR_H
