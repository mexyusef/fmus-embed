#include <fmus/core/memory.h>
#include <fmus/core/logging.h>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <mutex>

namespace fmus {
namespace core {

// StandardAllocator implementation
void* StandardAllocator::allocate(size_t size, size_t alignment) {
    if (size == 0) {
        return nullptr;
    }

#if defined(_MSC_VER)
    return _aligned_malloc(size, alignment);
#else
    void* ptr = nullptr;
    int result = posix_memalign(&ptr, alignment, size);
    return (result == 0) ? ptr : nullptr;
#endif
}

void StandardAllocator::deallocate(void* ptr, size_t size, size_t alignment) {
    (void)size;      // Unused parameter
    (void)alignment; // Unused parameter

    if (ptr) {
#if defined(_MSC_VER)
        _aligned_free(ptr);
#else
        free(ptr);
#endif
    }
}

const char* StandardAllocator::getName() const {
    return "StandardAllocator";
}

// PoolAllocator implementation
PoolAllocator::PoolAllocator(size_t blockSize, size_t blockCount, size_t alignment)
    : m_blockSize(std::max(blockSize, sizeof(void*))),  // Block must be at least size of pointer
      m_blockCount(blockCount),
      m_alignment(alignment),
      m_poolMemory(nullptr),
      m_freeList(nullptr),
      m_freeBlockCount(0) {

    // Calculate the actual block size with alignment consideration
    size_t alignedBlockSize = ((m_blockSize + alignment - 1) / alignment) * alignment;

    // Allocate the pool memory
    size_t totalSize = alignedBlockSize * blockCount;
    m_poolMemory = new uint8_t[totalSize];

    // Initialize the free list
    reset();
}

PoolAllocator::~PoolAllocator() {
    delete[] m_poolMemory;
}

void* PoolAllocator::allocate(size_t size, size_t alignment) {
    // Check if the requested size fits in a block
    if (size > m_blockSize || alignment > m_alignment) {
        FMUS_LOG_ERROR("PoolAllocator: Requested size or alignment exceeds block parameters");
        return nullptr;
    }

    // Check if there are free blocks
    if (!m_freeList) {
        FMUS_LOG_WARNING("PoolAllocator: No free blocks available");
        return nullptr;
    }

    // Get the first free block
    void* block = m_freeList;

    // Update the free list to point to the next free block
    m_freeList = *reinterpret_cast<void**>(block);

    // Decrement the free block count
    --m_freeBlockCount;

    return block;
}

void PoolAllocator::deallocate(void* ptr, size_t size, size_t alignment) {
    (void)size;      // Unused parameter
    (void)alignment; // Unused parameter

    if (!ptr) {
        return;
    }

    // Check if the pointer is within the pool
    uint8_t* ptrByte = static_cast<uint8_t*>(ptr);
    if (ptrByte < m_poolMemory || ptrByte >= (m_poolMemory + m_blockSize * m_blockCount)) {
        FMUS_LOG_ERROR("PoolAllocator: Attempted to deallocate pointer outside of pool");
        return;
    }

    // Add the block to the free list
    *reinterpret_cast<void**>(ptr) = m_freeList;
    m_freeList = ptr;

    // Increment the free block count
    ++m_freeBlockCount;
}

const char* PoolAllocator::getName() const {
    return "PoolAllocator";
}

void PoolAllocator::reset() {
    // Calculate the aligned block size
    size_t alignedBlockSize = ((m_blockSize + m_alignment - 1) / m_alignment) * m_alignment;

    // Initialize all blocks as a linked list
    m_freeList = m_poolMemory;
    m_freeBlockCount = m_blockCount;

    // Link all blocks together
    for (size_t i = 0; i < m_blockCount - 1; ++i) {
        void* currentBlock = m_poolMemory + i * alignedBlockSize;
        void* nextBlock = m_poolMemory + (i + 1) * alignedBlockSize;
        *reinterpret_cast<void**>(currentBlock) = nextBlock;
    }

    // Set the last block's next pointer to null
    *reinterpret_cast<void**>(m_poolMemory + (m_blockCount - 1) * alignedBlockSize) = nullptr;
}

size_t PoolAllocator::getFreeBlockCount() const {
    return m_freeBlockCount;
}

size_t PoolAllocator::getTotalBlockCount() const {
    return m_blockCount;
}

// MemoryManager implementation
static std::mutex g_memoryManagerMutex;

MemoryManager::MemoryManager()
    : m_defaultAllocator(std::make_shared<StandardAllocator>()) {
    // Register the default allocator
    registerAllocator("standard", m_defaultAllocator);
}

MemoryManager& MemoryManager::instance() {
    static MemoryManager instance;
    return instance;
}

void MemoryManager::setDefaultAllocator(std::shared_ptr<IAllocator> allocator) {
    if (!allocator) {
        return; // Don't allow null allocators
    }

    std::lock_guard<std::mutex> lock(g_memoryManagerMutex);
    m_defaultAllocator = allocator;
}

std::shared_ptr<IAllocator> MemoryManager::getDefaultAllocator() const {
    std::lock_guard<std::mutex> lock(g_memoryManagerMutex);
    return m_defaultAllocator;
}

void MemoryManager::registerAllocator(const std::string& name, std::shared_ptr<IAllocator> allocator) {
    if (!allocator || name.empty()) {
        return; // Don't allow null allocators or empty names
    }

    std::lock_guard<std::mutex> lock(g_memoryManagerMutex);
    m_allocators[name] = allocator;
}

std::shared_ptr<IAllocator> MemoryManager::getAllocator(const std::string& name) const {
    std::lock_guard<std::mutex> lock(g_memoryManagerMutex);

    auto it = m_allocators.find(name);
    if (it != m_allocators.end()) {
        return it->second;
    }

    return nullptr;
}

void* MemoryManager::allocate(size_t size, size_t alignment) {
    std::shared_ptr<IAllocator> allocator;

    {
        std::lock_guard<std::mutex> lock(g_memoryManagerMutex);
        allocator = m_defaultAllocator;
    }

    return allocator->allocate(size, alignment);
}

void MemoryManager::deallocate(void* ptr, size_t size, size_t alignment) {
    std::shared_ptr<IAllocator> allocator;

    {
        std::lock_guard<std::mutex> lock(g_memoryManagerMutex);
        allocator = m_defaultAllocator;
    }

    allocator->deallocate(ptr, size, alignment);
}

} // namespace core
} // namespace fmus
