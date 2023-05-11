#pragma once

#include <cstddef>
#include <memory>
#include <new>

#include "AdapterAllocator.h"

#include "../Utility/LogDuration.h"

#include "../../External/memory-allocators/includes/PoolAllocator.h"

template <std::size_t Count, std::size_t ChunkSize>
class StaticPoolAllocator {
 public:
    static void* allocate(std::size_t size) {
        return GetAllocator()->Allocate(size);
    }

    static void deallocate(std::size_t size, void* data) {
        GetAllocator()->Free(data);
    }

    static PoolAllocator* GetAllocator() {
        static std::unique_ptr<PoolAllocator> allocator;

        if (!allocator) {
            LOG_DURATION("init pool allocator")

            allocator = std::make_unique<PoolAllocator>(Count * ChunkSize, ChunkSize);
            allocator->Init();
        }

        return allocator.get();
    }
};

template <typename T>
using StdPoolAllocator = AdapterAllocator<T, StaticPoolAllocator<1u << 24, sizeof(T) + 2 * sizeof(void*)>>;
