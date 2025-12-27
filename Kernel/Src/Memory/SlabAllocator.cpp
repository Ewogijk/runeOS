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

#include <Memory/SlabAllocator.h>

#include <KRE/Math.h>

namespace Rune::Memory {
    DEFINE_TYPED_ENUM(CacheType, U8, CACHE_TYPES, 0x0)

    DEFINE_ENUM(HeapStartFailureCode, HEAP_START_FAILURE_CODES, 0x0)

    ////////////////////////////////////////////////////////////////////////
    //
    //  Slab Implementation
    //
    ////////////////////////////////////////////////////////////////////////

    auto Slab::create_on_slab(size_t object_size, VirtualAddr page) -> Slab* {
        size_t objects = (get_page_size() - sizeof(Slab)) / object_size;
        objects        = min(objects, MAX_OBJECT_COUNT);

        size_t free_list_size = objects;
        size_t wastage        = get_page_size() - sizeof(Slab) - (objects * object_size);

        if (free_list_size > wastage) {
            objects        = (get_page_size() - sizeof(Slab) - free_list_size) / object_size;
            free_list_size = objects;
            // don't care about wastage update
        }

        auto* slab = memory_addr_to_pointer<Slab>(page + get_page_size() - sizeof(Slab));
        slab->next = nullptr;
        slab->prev = nullptr;

        slab->object_size     = object_size;
        slab->object_count    = objects;
        slab->allocated_count = 0;
        slab->page            = memory_addr_to_pointer<void>(page);
        slab->slab_size       = get_page_size();

        // Init free list
        auto* free_list =
            memory_addr_to_pointer<U8>(page + get_page_size() - sizeof(Slab) - free_list_size);
        slab->free_buf.free_object = 0;
        for (size_t i = 0; i < objects - 1; i++) free_list[i] = i + 1;
        free_list[objects - 1] = MAX_OBJECT_COUNT;

        memset(slab->page, 0, get_page_size() - sizeof(Slab) - free_list_size);

        return slab;
    }

    auto Slab::create_off_slab(ObjectCache*          slab_cache,
                               ObjectCache*          object_buf_node_cache,
                               ObjectBufNodeHashMap* object_buf_node_hashmap,
                               size_t                object_size,
                               VirtualAddr           page,
                               size_t                slab_size) -> Slab* {
        auto* slab = reinterpret_cast<Slab*>(slab_cache->allocate());

        slab->next = nullptr;
        slab->prev = nullptr;

        slab->object_size     = object_size;
        slab->object_count    = slab_size / object_size;
        slab->allocated_count = 0;

        slab->page      = memory_addr_to_pointer<void>(page);
        slab->slab_size = slab_size;

        memset(slab->page, 0, slab_size);

        auto* prev   = reinterpret_cast<ObjectBufNode*>(object_buf_node_cache->allocate());
        prev->object = memory_addr_to_pointer<void>(page);
        prev->next   = nullptr;
        prev->owner  = slab;
        slab->free_buf.regular_object = prev;
        object_buf_node_hashmap->insert(prev->object, prev);

        for (size_t i = 1; i < slab->object_count; i++) {
            auto* n   = reinterpret_cast<ObjectBufNode*>(object_buf_node_cache->allocate());
            n->object = memory_addr_to_pointer<void>(page + (i * object_size));
            n->next   = nullptr;
            n->owner  = slab;
            object_buf_node_hashmap->insert(n->object, n);

            prev->next = n;
            prev       = n;
        }

        return slab;
    }

    auto Slab::alloc_on_slab() -> void* {
        if (allocated_count == object_count) return nullptr;

        auto* obj       = memory_addr_to_pointer<void>(memory_pointer_to_addr(page)
                                                 + (free_buf.free_object * object_size));
        auto* free_list = memory_addr_to_pointer<U8>(memory_pointer_to_addr(this) - object_count);
        free_buf.free_object = free_list[free_buf.free_object];
        allocated_count++;
        return obj;
    }

    auto Slab::alloc_off_slab() -> void* {
        if (allocated_count == object_count) return nullptr;

        ObjectBufNode* removed  = free_buf.regular_object;
        free_buf.regular_object = free_buf.regular_object->next;
        removed->next           = nullptr;

        allocated_count++;
        return removed->object;
    }

    auto Slab::free_on_slab(void* obj) -> bool {
        if (allocated_count == 0) return false;

        size_t obj_idx = (memory_pointer_to_addr(obj) - memory_pointer_to_addr(page)) / object_size;
        auto*  free_list = memory_addr_to_pointer<U8>(memory_pointer_to_addr(this) - object_count);
        free_list[obj_idx]   = free_buf.free_object;
        free_buf.free_object = obj_idx;
        allocated_count--;
        return true;
    }

    auto Slab::free_off_slab(Memory::ObjectBufNode* obj_buf) -> bool {
        if (obj_buf == nullptr) return false;

        obj_buf->next           = free_buf.regular_object;
        free_buf.regular_object = obj_buf;
        allocated_count--;
        return true;
    }

    ////////////////////////////////////////////////////////////////////////
    //
    //  ObjectCache Implementation
    //
    ////////////////////////////////////////////////////////////////////////

    // Return the new head of the list
    auto insert_last(Slab* list, Slab* slab) -> Slab* {
        Slab* new_head = nullptr;
        if (list == nullptr) {
            // Empty list
            new_head   = slab;
            slab->next = slab->prev = slab;
        } else {
            new_head   = list;
            Slab* head = list;
            Slab* tail = list->prev;
            tail->next = slab; // NOLINT tail != null -> well tested
            slab->next = head;
            slab->prev = tail;
            head->prev = slab;
        }
        return new_head;
    }

    // Return the new head of the list
    auto remove(Slab* list, Slab* element) -> Slab* {
        // Assumption: element is always in list
        Slab* new_head = nullptr;
        new_head       = nullptr;
        if (element != element->next) {
            new_head            = list == element ? element->next : list;
            element->next->prev = element->prev;
            element->prev->next = element->next;
        } // else One element list
        element->next = nullptr;
        element->prev = nullptr;

        return new_head;
    }

    // Free all ObjectBufNodes and the slab
    void destroy_slab_list(Slab* list, ObjectCache* slab_cache) {
        Slab* head = list;
        Slab* slab = list;
        while (slab != nullptr) {
            Slab* next = slab->next;

            slab->next                    = nullptr;
            slab->prev                    = nullptr;
            slab->free_buf.regular_object = nullptr;
            slab->object_size             = 0;
            slab->object_count            = 0;
            slab->allocated_count         = 0;
            slab->page                    = nullptr;
            slab_cache->free(slab);

            slab = slab != next ? next : nullptr;
            if (slab == head) break;
        }
    }

    auto ObjectCache::grow() -> bool {
        VirtualAddr page = (_free_page_list != nullptr) ? _free_page_list->mem_addr : _limit;
        size_t      aligned_size =
            _align == 0 ? _object_size : _align * ((_object_size - 1) / _align + 1);
        size_t slab_size = aligned_size;
        if (!memory_is_aligned(slab_size, Memory::get_page_size()))
            slab_size = memory_align(aligned_size, Memory::get_page_size(), true);

        if ((page + slab_size) >= _managed.end()) return false;

        bool        all_fine   = true;
        VirtualAddr last_alloc = 0x0;
        for (VirtualAddr i = page; i < page + slab_size; i += Memory::get_page_size()) {
            if (!_vmm->allocate(i, _page_flags)) {
                all_fine   = false;
                last_alloc = i;
                break;
            }
        }
        if (!all_fine) {
            for (VirtualAddr i = page; i < last_alloc; i += Memory::get_page_size())
                if (!_vmm->free(i)) break;
            return false;
        }

        if (page < _limit) {
            MemoryNode* f   = _free_page_list;
            _free_page_list = _free_page_list->next;
            f->next         = nullptr;
            f->mem_addr     = 0;
            _memory_node_cache->free(f);
        } else {
            _limit += slab_size;
        }

        Slab* slab = _type == CacheType::ON_SLAB ? Slab::create_on_slab(aligned_size, page)
                                                 : Slab::create_off_slab(_slab_cache,
                                                                         _object_buf_node_cache,
                                                                         _object_buf_node_hash_map,
                                                                         aligned_size,
                                                                         page,
                                                                         slab_size);

        _empty_list = insert_last(_empty_list, slab);
        _slab_count++;
        return true;
    }

    ObjectCache::ObjectCache() : _managed(), _type(CacheType::NONE) {}

    auto ObjectCache::get_managed() const -> MemoryRegion { return _managed; }

    auto ObjectCache::get_type() const -> CacheType { return _type; }

    auto ObjectCache::init(VirtualMemoryManager* vmm,
                           ObjectCache*          memory_node_cache,
                           MemoryRegion          managed,
                           U16                   page_flags,
                           ObjectCache*          object_buf_node_cache,
                           ObjectBufNodeHashMap* object_buf_node_hash_map,
                           size_t                object_size,
                           size_t                align,
                           ObjectCache*          slab_cache) -> int8_t {
        if (object_size == 0) return -1;
        if (align > 0 && ((align & (align - 1)) != 0))
            return -2; // align is > 0 and not a power of 2

        _vmm               = vmm;
        _memory_node_cache = memory_node_cache;
        _managed           = managed;
        _limit             = managed.start;
        _page_flags        = page_flags;

        _object_buf_node_cache    = object_buf_node_cache;
        _object_buf_node_hash_map = object_buf_node_hash_map;
        _object_size              = object_size;
        _align                    = align;

        _slab_cache = slab_cache;

        _type = object_size < (Memory::get_page_size() / ON_OFF_SLAB_BOUNDARY_DIVIDER)
                    ? CacheType::ON_SLAB
                    : CacheType::OFF_SLAB;
        return 0;
    }

    auto ObjectCache::allocate() -> void* {
        Slab* alloc_slab = (_partial_list != nullptr) ? _partial_list : _empty_list;
        if ((alloc_slab == nullptr) || alloc_slab->allocated_count == alloc_slab->object_count) {
            if (!grow()) return nullptr;
            alloc_slab = (_partial_list != nullptr) ? _partial_list : _empty_list;
        }
        void* obj = _type == CacheType::ON_SLAB ? alloc_slab->alloc_on_slab()
                                                : alloc_slab->alloc_off_slab();

        if (alloc_slab->allocated_count == alloc_slab->object_count) {
            // Move to full slabs
            if (alloc_slab == _empty_list)
                _empty_list = remove(_empty_list, alloc_slab);
            else
                _partial_list = remove(_partial_list, alloc_slab);

            _full_list = insert_last(_full_list, alloc_slab);
        } else if (alloc_slab == _empty_list) {
            // Move to partial slabs
            _empty_list   = remove(_empty_list, alloc_slab);
            _partial_list = insert_last(_partial_list, alloc_slab);
        }
        return obj;
    }

    void ObjectCache::free(void* obj) {
        Slab*  slab             = nullptr;
        size_t old_alloc_count  = 0;
        bool   slab_has_changed = false;
        if (_type == CacheType::ON_SLAB) {
            auto page = memory_pointer_to_addr(obj)
                        >> 12 << 12; // NOLINT TODO add function to get page of address??
            slab = memory_addr_to_pointer<Slab>(page + Memory::get_page_size() - sizeof(Slab));
            if (slab->page != memory_addr_to_pointer<void>(page)) return;

            old_alloc_count  = slab->allocated_count;
            slab_has_changed = slab->free_on_slab(obj);
        } else {
            ObjectBufNode* node = _object_buf_node_hash_map->get(obj);
            if (node == nullptr) return;

            slab = node->owner;
            if (slab == nullptr) return;

            old_alloc_count  = slab->allocated_count;
            slab_has_changed = slab->free_off_slab(node);
        }

        if (!slab_has_changed) return;

        if (slab->allocated_count == 0) {
            // Move to empty list
            if (slab == _full_list)
                _full_list = remove(_full_list, slab);
            else
                _partial_list = remove(_partial_list, slab);

            _empty_list = insert_last(_empty_list, slab);
        } else if (old_alloc_count == slab->object_count) // slab->allocated_count > 0
        {
            // Slab was full and is now only partially full -> Move to partial list
            _full_list    = remove(_full_list, slab);
            _partial_list = insert_last(_partial_list, slab);
        }
    }

    auto ObjectCache::object_at(U8 idx) -> void* {
        if (_type == CacheType::OFF_SLAB || _slab_count == 0) return nullptr;

        size_t obj_count = 0;
        if (_empty_list != nullptr)
            obj_count = _empty_list->object_count;
        else if (_partial_list != nullptr)
            obj_count = _partial_list->object_count;
        else
            obj_count = _full_list->object_count;

        VirtualAddr page = (_managed.start + ((idx / obj_count) * Memory::get_page_size()));
        return memory_addr_to_pointer<void>(page + ((idx % obj_count) * _object_size));
    }

    void ObjectCache::destroy() {
        if (_type == CacheType::OFF_SLAB) {
            destroy_slab_list(_full_list, _slab_cache);
            destroy_slab_list(_partial_list, _slab_cache);
            destroy_slab_list(_empty_list, _slab_cache);
            _object_buf_node_hash_map->destroy(_object_buf_node_cache);
        }

        for (VirtualAddr addr = _managed.start; addr < _limit; addr += Memory::get_page_size())
            _vmm->free(addr);

        _vmm               = nullptr;
        _memory_node_cache = nullptr;
        _managed           = {};
        _limit             = 0;
        _page_flags        = 0;
        _free_page_list    = nullptr;

        _object_buf_node_cache    = nullptr;
        _object_buf_node_hash_map = nullptr;
        _object_size              = 0;
        _align                    = 0;

        _slab_cache   = nullptr;
        _full_list    = nullptr;
        _partial_list = nullptr;
        _empty_list   = nullptr;
        _slab_count   = 0;

        _type = CacheType::NONE;
    }

    ////////////////////////////////////////////////////////////////////////
    //
    //  ObjectBufNodeHashMap Implementation
    //
    ////////////////////////////////////////////////////////////////////////

    ObjectBufNodeHashMap::ObjectBufNodeHashMap() : _nodes(), _hash_node_cache(nullptr) {
        memset(reinterpret_cast<void*>(_nodes.data()), 0, sizeof(HashNode*) * BUCKET_COUNT);
    }

    void ObjectBufNodeHashMap::init(Memory::ObjectCache* hash_node_cache) {
        _hash_node_cache = hash_node_cache;
        memset(reinterpret_cast<void*>(_nodes.data()), 0, sizeof(HashNode*) * BUCKET_COUNT);
    }

    void ObjectBufNodeHashMap::insert(void* key, ObjectBufNode* value) {
        auto  hash  = memory_pointer_to_addr(key) % BUCKET_COUNT;
        auto* node  = reinterpret_cast<HashNode*>(_hash_node_cache->allocate());
        node->key   = key;
        node->value = value;
        node->next  = nullptr;

        HashNode* old_head = _nodes[hash];
        node->next         = old_head;
        _nodes[hash]       = node;
    }

    void ObjectBufNodeHashMap::remove(void* key) {
        auto      hash = memory_pointer_to_addr(key) % BUCKET_COUNT;
        HashNode* node = _nodes[hash];
        HashNode* last = nullptr;
        while (node != nullptr) {
            if (node->key == key) {
                if (last != nullptr)
                    last->next = node->next;
                else
                    _nodes[hash] = node->next;
                _hash_node_cache->free(node);
                return;
            }

            last = node;
            node = node->next;
        }
    }

    auto ObjectBufNodeHashMap::get(void* key) -> ObjectBufNode* {
        auto  hash = memory_pointer_to_addr(key) % BUCKET_COUNT;
        auto* node = _nodes[hash];
        while (node != nullptr) {
            if (node->key == key) return node->value;
            node = node->next;
        }
        return nullptr;
    }

    void ObjectBufNodeHashMap::destroy(ObjectCache* object_buf_cache) {
        for (auto& node : _nodes) {
            while (node != nullptr) {
                HashNode* next      = node->next;
                node->value->next   = nullptr;
                node->value->object = nullptr;
                node->value->owner  = nullptr;
                object_buf_cache->free(node->value);

                node->next  = nullptr;
                node->value = nullptr;
                node->key   = nullptr;
                _hash_node_cache->free(node);
                node = next;
            }
            node = nullptr;
        }

        _hash_node_cache = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////
    //
    //  SlabAllocator Implementation
    //
    ////////////////////////////////////////////////////////////////////////

    auto Log2Shit(size_t n) -> U8 {
        // NOLINTBEGIN
        switch (n) {
            case 16:    return 4;
            case 32:    return 5;
            case 64:    return 6;
            case 128:   return 7;
            case 256:   return 8;
            case 512:   return 9;
            case 1024:  return 10;
            case 2048:  return 11;
            case 4096:  return 12;
            case 8192:  return 13;
            case 16384: return 14;
            case 32768: return 15;
            case 65536: return 16;
            default:    return 0;
        }
        // NOLINTEND
    }

    void power_of_two_boundaries(size_t& lower, size_t& upper, size_t size) {
        // NOLINTBEGIN
        size--;
        size |= size >> 1;
        size |= size >> 2;
        size |= size >> 4;
        size |= size >> 8;
        size |= size >> 16;
        size |= size >> 32;
        size++;

        upper = size;
        lower = size >> 1;
        // NOLINTEND
    }

    auto SlabAllocator::init_cache(Memory::ObjectCache*          cache,
                                   size_t                        obj_size,
                                   size_t                        align,
                                   uint16_t                      page_flags,
                                   bool                          force_off_slab,
                                   Memory::ObjectBufNodeHashMap* object_buf_node_hashmap)
        -> int8_t {
        if (_limit >= _heap_memory.end()) return -1;

        if (force_off_slab
            && obj_size >= Memory::get_page_size() / ObjectCache::ON_OFF_SLAB_BOUNDARY_DIVIDER)
            return -2;

        MemoryRegion region = {.start = (_free_list != nullptr) ? _free_list->mem_addr : _limit,
                               .size  = CACHE_SIZE};
        int8_t       r_code = 0;
        if (obj_size < Memory::get_page_size() / ObjectCache::ON_OFF_SLAB_BOUNDARY_DIVIDER) {
            r_code = cache->init(_vmm,
                                 nullptr,
                                 region,
                                 page_flags,
                                 nullptr,
                                 nullptr,
                                 obj_size,
                                 align,
                                 nullptr);
        } else {
            r_code = cache->init(_vmm,
                                 &_memory_node_cache,
                                 region,
                                 page_flags,
                                 &_object_buf_node_cache,
                                 object_buf_node_hashmap,
                                 obj_size,
                                 align,
                                 &_slab_cache);
        }
        if (r_code < 0) return r_code;

        if (region.start == _limit) {
            _limit += CACHE_SIZE;
        } else {
            MemoryNode* f = _free_list;
            _free_list    = _free_list->next;
            f->next       = nullptr;
            f->mem_addr   = 0;
            _memory_node_cache.free(f);
        }

        return 0;
    }

    auto SlabAllocator::get_min_cache_size() const -> U32 { return 1 << MIN_SIZE_POWER; } // NOLINT

    auto SlabAllocator::get_max_cache_size() const -> U32 { // NOLINT
        return 1 << (STATIC_CACHE_COUNT + MIN_SIZE_POWER - 1);
    }

    auto SlabAllocator::start(MemoryMap* v_map, VirtualMemoryManager* vmm) -> HeapStartFailureCode {
        for (const auto& reg : *v_map) {
            if (reg.memory_type == MemoryRegionType::KERNEL_HEAP) {
                _heap_memory = reg;
                break;
            }
        }
        if (_heap_memory.memory_type == MemoryRegionType::NONE) {
            _start_failure_code = HeapStartFailureCode::HEAP_NOT_MAPPED;
            return _start_failure_code;
        }
        _vmm   = vmm;
        _limit = _heap_memory.start;

        // Init the bootstrap caches
        U16 page_flags = PageFlag::PRESENT | PageFlag::WRITE_ALLOWED;

        if (init_cache(&_object_cache_cache, sizeof(ObjectCache), 0, page_flags, true, nullptr)
            < 0) {
            _start_failure_code = HeapStartFailureCode::BC_OBJECT_CACHE_ERROR;
            return _start_failure_code;
        }

        if (init_cache(&_slab_cache, sizeof(Slab), 0, page_flags, true, nullptr) < 0) {
            _start_failure_code = HeapStartFailureCode::BC_SLAB_ERROR;
            return _start_failure_code;
        }

        if (init_cache(&_object_buf_node_cache, sizeof(ObjectBufNode), 0, page_flags, true, nullptr)
            < 0) {
            _start_failure_code = HeapStartFailureCode::BC_OBJECT_BUF_NODE_ERROR;
            return _start_failure_code;
        }

        if (init_cache(&_object_buf_node_hash_map_cache,
                       sizeof(ObjectBufNodeHashMap),
                       0,
                       page_flags,
                       true,
                       nullptr)
            < 0) {
            _start_failure_code = HeapStartFailureCode::BC_OBJECT_BUF_NODE_HASHMAP_ERROR;
            return _start_failure_code;
        }

        if (init_cache(&_hash_node_cache, sizeof(HashNode), 0, page_flags, true, nullptr) < 0) {
            _start_failure_code = HeapStartFailureCode::BC_HASHNODE_ERROR;
            return _start_failure_code;
        }

        if (init_cache(&_memory_node_cache, sizeof(MemoryNode), 0, page_flags, true, nullptr) < 0) {
            _start_failure_code = HeapStartFailureCode::BC_MEMORY_NODE_ERROR;
            return _start_failure_code;
        }

        // Init general purpose and dma caches
        U16    dma_page_flags = page_flags | PageFlag::CACHE_DISABLE | PageFlag::WRITE_THROUGH;
        size_t size           = MIN_OBJ_SIZE;
        for (size_t i = 0; i < STATIC_CACHE_COUNT; i++) {
            auto* gpc  = reinterpret_cast<ObjectCache*>(_object_cache_cache.allocate());
            auto* dmac = reinterpret_cast<ObjectCache*>(_object_cache_cache.allocate());
            auto* gpcMap =
                reinterpret_cast<ObjectBufNodeHashMap*>(_object_buf_node_hash_map_cache.allocate());
            auto* dmaMap =
                reinterpret_cast<ObjectBufNodeHashMap*>(_object_buf_node_hash_map_cache.allocate());

            if ((gpc == nullptr) || (dmac == nullptr) || (gpcMap == nullptr)
                || (dmaMap == nullptr)) {
                _start_failure_code = HeapStartFailureCode::ALLOC_GP_OR_DMA_CACHE_ERROR;
                return _start_failure_code;
            }

            gpcMap->init(&_hash_node_cache);
            dmaMap->init(&_hash_node_cache);
            if (init_cache(gpc, size, 0, page_flags, false, gpcMap) < 0) {
                _start_failure_code = HeapStartFailureCode::GP_CACHE_ERROR;
                return _start_failure_code;
            }

            if (init_cache(dmac, size, 0, dma_page_flags, false, dmaMap) < 0) {
                _start_failure_code = HeapStartFailureCode::DMA_CACHE_ERROR;
                return _start_failure_code;
            }

            _general_purpose_cache[i]   = gpc;
            _dma_cache[i]               = dmac;
            size                      <<= 1;
        }
        return HeapStartFailureCode::NONE;
    }

    SlabAllocator::SlabAllocator()
        : _general_purpose_cache(),
          _dma_cache(),
          _start_failure_code(HeapStartFailureCode::NONE) {}

    auto SlabAllocator::allocate(size_t size) -> void* {
        // Allocating small objects is very inefficient, but still we want to allocate them
        // -> Pad them to MIN_OBJ_SIZE TODO this cannot be more efficient?!?!
        size = max(size, MIN_OBJ_SIZE);

        size_t lower_po_2 = 0;
        size_t upper_po_2 = 0;
        power_of_two_boundaries(lower_po_2, upper_po_2, size);
        return _general_purpose_cache[Log2Shit(upper_po_2) - MIN_SIZE_POWER]->allocate();
    }

    auto SlabAllocator::allocate_dma(size_t size) -> void* {
        // Allocating small objects is very inefficient, but still we want to allocate them
        // -> Pad them to MIN_OBJ_SIZE TODO this cannot be more efficient?!?!
        size = max(size, MIN_OBJ_SIZE);

        size_t lower_po_2 = 0;
        size_t upper_po_2 = 0;
        power_of_two_boundaries(lower_po_2, upper_po_2, size);
        return _dma_cache[Log2Shit(upper_po_2) - MIN_SIZE_POWER]->allocate();
    }

    void SlabAllocator::free(void* obj) {
        size_t cache_idx =
            (memory_align(memory_pointer_to_addr(obj), CACHE_SIZE, false) - _heap_memory.start)
            / CACHE_SIZE;
        auto* c = reinterpret_cast<ObjectCache*>(
            _object_cache_cache.object_at(cache_idx - BOOTSTRAP_CACHE_COUNT));
        if ((c == nullptr) || c->get_type() == CacheType::NONE) return;
        c->free(obj);
    }

    auto SlabAllocator::create_new_cache(size_t object_size, size_t align, bool dma)
        -> ObjectCache* {
        auto* cache = reinterpret_cast<ObjectCache*>(_object_cache_cache.allocate());
        if (cache == nullptr) return nullptr;

        auto* obnhm =
            reinterpret_cast<ObjectBufNodeHashMap*>(_object_buf_node_hash_map_cache.allocate());
        if (obnhm == nullptr) {
            _object_cache_cache.free(cache);
            return nullptr;
        }
        obnhm->init(&_hash_node_cache);

        U16 pageFlags = PageFlag::PRESENT | PageFlag::WRITE_ALLOWED;
        if (dma) pageFlags = pageFlags | PageFlag::CACHE_DISABLE | PageFlag::WRITE_THROUGH;

        if (init_cache(cache, object_size, align, pageFlags, false, obnhm) < 0) {
            _object_cache_cache.free(cache);
            _object_buf_node_hash_map_cache.free(obnhm);
            return nullptr;
        }
        return cache;
    }

    void SlabAllocator::destroy_cache(Memory::ObjectCache* cache) {
        VirtualAddr m_start  = cache->get_managed().start;
        auto*       mem_node = reinterpret_cast<MemoryNode*>(_memory_node_cache.allocate());
        if (mem_node == nullptr) return; // TODO log error

        cache->destroy();
        _object_cache_cache.free(cache);

        if (m_start == _limit) {
            _limit -= CACHE_SIZE;
        } else {
            mem_node->next     = _free_list;
            mem_node->mem_addr = m_start;
            _free_list         = mem_node;
        }
    }

} // namespace Rune::Memory