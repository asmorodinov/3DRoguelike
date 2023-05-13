#pragma once

#include <cstddef>
#include <memory>
#include <new>

#include "AdapterAllocator.h"

#include "../Utility/LogDuration.h"

#include "../../External/memory-allocators/includes/PoolAllocator.h"

template <std::size_t Count, std::size_t ChunkSize1, std::size_t ChunkSize2>
class StaticPoolAllocator {
 public:
    static void* allocate(std::size_t size) {
        return GetAllocator(size)->Allocate(size);
    }

    static void deallocate(std::size_t size, void* data) {
        GetAllocator(size)->Free(data);
    }

    static PoolAllocator* GetAllocator(std::size_t size) {
        static std::unique_ptr<PoolAllocator> allocator1;
        static std::unique_ptr<PoolAllocator> allocator2;

        if (!allocator1) {
            LOG_DURATION("init pool allocators")

            allocator1 = std::make_unique<PoolAllocator>(Count * ChunkSize1, ChunkSize1);
            allocator1->Init();
            allocator2 = std::make_unique<PoolAllocator>(Count * ChunkSize2, ChunkSize2);
            allocator2->Init();
        }

        if (size == ChunkSize1) {
            return allocator1.get();
        } else {
            return allocator2.get();
        }
    }
};

template <typename T>
using StdPoolAllocator = AdapterAllocator<T, StaticPoolAllocator<1u << 23, 72, 40>>;
