#pragma once

#include "../fmus_config.h"
#include "result.h"
#include <memory>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <new>
#include <string>
#include <unordered_map>

namespace fmus {
namespace core {

/**
 * @brief Abstract memory allocator interface
 */
class FMUS_EMBED_API IAllocator {
public:
    virtual ~IAllocator() = default;

    /**
     * @brief Allocate memory
     *
     * @param size The number of bytes to allocate
     * @param alignment The alignment requirement (power of 2)
     * @return void* Pointer to the allocated memory, or nullptr if allocation failed
     */
    virtual void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) = 0;

    /**
     * @brief Deallocate memory
     *
     * @param ptr Pointer to the memory to deallocate
     * @param size The size of the allocation
     * @param alignment The alignment used for the allocation
     */
    virtual void deallocate(void* ptr, size_t size, size_t alignment = alignof(std::max_align_t)) = 0;

    /**
     * @brief Get the name of the allocator
     *
     * @return const char* The allocator name
     */
    virtual const char* getName() const = 0;
};

/**
 * @brief Standard allocator that uses the C++ new/delete operators
 */
class FMUS_EMBED_API StandardAllocator : public IAllocator {
public:
    /**
     * @brief Allocate memory using the standard C++ allocator
     *
     * @param size The number of bytes to allocate
     * @param alignment The alignment requirement (power of 2)
     * @return void* Pointer to the allocated memory, or nullptr if allocation failed
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override;

    /**
     * @brief Deallocate memory using the standard C++ allocator
     *
     * @param ptr Pointer to the memory to deallocate
     * @param size The size of the allocation
     * @param alignment The alignment used for the allocation
     */
    void deallocate(void* ptr, size_t size, size_t alignment = alignof(std::max_align_t)) override;

    /**
     * @brief Get the name of the allocator
     *
     * @return const char* The allocator name
     */
    const char* getName() const override;
};

/**
 * @brief Pool allocator for efficient allocation of fixed-size objects
 */
class FMUS_EMBED_API PoolAllocator : public IAllocator {
public:
    /**
     * @brief Construct a new Pool Allocator
     *
     * @param blockSize The size of each block in the pool
     * @param blockCount The number of blocks in the pool
     * @param alignment The alignment of each block (power of 2)
     */
    PoolAllocator(size_t blockSize, size_t blockCount, size_t alignment = alignof(std::max_align_t));

    /**
     * @brief Destructor to free the pool memory
     */
    ~PoolAllocator();

    /**
     * @brief Allocate memory from the pool
     *
     * @param size The number of bytes to allocate (must be <= blockSize)
     * @param alignment The alignment requirement (power of 2)
     * @return void* Pointer to the allocated memory, or nullptr if the pool is full
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override;

    /**
     * @brief Return memory to the pool
     *
     * @param ptr Pointer to the memory to deallocate
     * @param size The size of the allocation
     * @param alignment The alignment used for the allocation
     */
    void deallocate(void* ptr, size_t size, size_t alignment = alignof(std::max_align_t)) override;

    /**
     * @brief Get the name of the allocator
     *
     * @return const char* The allocator name
     */
    const char* getName() const override;

    /**
     * @brief Reset the pool, making all blocks available again
     */
    void reset();

    /**
     * @brief Get the number of free blocks in the pool
     *
     * @return size_t The number of free blocks
     */
    size_t getFreeBlockCount() const;

    /**
     * @brief Get the total number of blocks in the pool
     *
     * @return size_t The total number of blocks
     */
    size_t getTotalBlockCount() const;

private:
    size_t m_blockSize;     ///< The size of each block
    size_t m_blockCount;    ///< The total number of blocks
    size_t m_alignment;     ///< The alignment of each block
    uint8_t* m_poolMemory;  ///< The pool memory
    void* m_freeList;       ///< Linked list of free blocks
    size_t m_freeBlockCount; ///< The number of free blocks
};

/**
 * @brief Memory management system
 */
class FMUS_EMBED_API MemoryManager {
public:
    /**
     * @brief Get the singleton instance of the MemoryManager
     *
     * @return MemoryManager& The memory manager instance
     */
    static MemoryManager& instance();

    /**
     * @brief Set the default allocator
     *
     * @param allocator The allocator to use
     */
    void setDefaultAllocator(std::shared_ptr<IAllocator> allocator);

    /**
     * @brief Get the default allocator
     *
     * @return std::shared_ptr<IAllocator> The default allocator
     */
    std::shared_ptr<IAllocator> getDefaultAllocator() const;

    /**
     * @brief Register a named allocator
     *
     * @param name The name of the allocator
     * @param allocator The allocator to register
     */
    void registerAllocator(const std::string& name, std::shared_ptr<IAllocator> allocator);

    /**
     * @brief Get a named allocator
     *
     * @param name The name of the allocator
     * @return std::shared_ptr<IAllocator> The allocator, or nullptr if not found
     */
    std::shared_ptr<IAllocator> getAllocator(const std::string& name) const;

    /**
     * @brief Allocate memory using the default allocator
     *
     * @param size The number of bytes to allocate
     * @param alignment The alignment requirement (power of 2)
     * @return void* Pointer to the allocated memory, or nullptr if allocation failed
     */
    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));

    /**
     * @brief Deallocate memory using the default allocator
     *
     * @param ptr Pointer to the memory to deallocate
     * @param size The size of the allocation
     * @param alignment The alignment used for the allocation
     */
    void deallocate(void* ptr, size_t size, size_t alignment = alignof(std::max_align_t));

private:
    MemoryManager();  ///< Private constructor for singleton
    std::shared_ptr<IAllocator> m_defaultAllocator;  ///< The default allocator
    std::unordered_map<std::string, std::shared_ptr<IAllocator>> m_allocators;  ///< Named allocators
};

/**
 * @brief Custom deleter for std::unique_ptr that uses a specific allocator
 *
 * @tparam T The type of object to delete
 */
template<typename T>
struct AllocatorDeleter {
    /**
     * @brief Construct a new Allocator Deleter
     *
     * @param allocator The allocator to use for deallocation
     */
    explicit AllocatorDeleter(std::shared_ptr<IAllocator> allocator = MemoryManager::instance().getDefaultAllocator())
        : m_allocator(allocator) {}

    /**
     * @brief Delete the object and deallocate its memory
     *
     * @param ptr The object to delete
     */
    void operator()(T* ptr) {
        if (ptr) {
            ptr->~T(); // Call destructor
            m_allocator->deallocate(ptr, sizeof(T), alignof(T));
        }
    }

private:
    std::shared_ptr<IAllocator> m_allocator;  ///< The allocator to use
};

/**
 * @brief Create a new object using a specific allocator
 *
 * @tparam T The type of object to create
 * @tparam Args The types of arguments to pass to the constructor
 * @param allocator The allocator to use, or nullptr for the default
 * @param args The arguments to pass to the constructor
 * @return std::unique_ptr<T, AllocatorDeleter<T>> The created object
 */
template<typename T, typename... Args>
std::unique_ptr<T, AllocatorDeleter<T>> makeUnique(std::shared_ptr<IAllocator> allocator, Args&&... args) {
    // Use the provided allocator or the default if nullptr
    auto actualAllocator = allocator ? allocator : MemoryManager::instance().getDefaultAllocator();

    // Allocate memory
    void* memory = actualAllocator->allocate(sizeof(T), alignof(T));
    if (!memory) {
        return nullptr;
    }

    // Construct the object
    T* object = new(memory) T(std::forward<Args>(args)...);

    // Return a unique_ptr with the custom deleter
    return std::unique_ptr<T, AllocatorDeleter<T>>(object, AllocatorDeleter<T>(actualAllocator));
}

/**
 * @brief Create a new object using the default allocator
 *
 * @tparam T The type of object to create
 * @tparam Args The types of arguments to pass to the constructor
 * @param args The arguments to pass to the constructor
 * @return std::unique_ptr<T, AllocatorDeleter<T>> The created object
 */
template<typename T, typename... Args>
std::unique_ptr<T, AllocatorDeleter<T>> makeUnique(Args&&... args) {
    return makeUnique<T>(nullptr, std::forward<Args>(args)...);
}

} // namespace core
} // namespace fmus
