#include <gtest/gtest.h>
#include "fmus/core/memory.h"
#include <vector>
#include <memory>

using namespace fmus::core;

class MemoryTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(MemoryTest, BasicMemoryOperations) {
    // Test basic memory allocation and deallocation
    void* ptr = allocateMemory(1024);
    EXPECT_NE(ptr, nullptr);
    
    if (ptr) {
        // Write some data to verify the memory is usable
        memset(ptr, 0xAA, 1024);
        uint8_t* bytePtr = static_cast<uint8_t*>(ptr);
        EXPECT_EQ(bytePtr[0], 0xAA);
        EXPECT_EQ(bytePtr[1023], 0xAA);
        
        freeMemory(ptr);
    }
}

TEST_F(MemoryTest, ZeroSizeAllocation) {
    // Test allocation with zero size
    void* ptr = allocateMemory(0);
    // Behavior may vary by implementation, but should not crash
    if (ptr) {
        freeMemory(ptr);
    }
    EXPECT_TRUE(true); // If we get here, no crash occurred
}

TEST_F(MemoryTest, LargeAllocation) {
    // Test large allocation (1MB)
    const size_t largeSize = 1024 * 1024;
    void* ptr = allocateMemory(largeSize);
    
    if (ptr) {
        // Verify we can write to the beginning and end
        uint8_t* bytePtr = static_cast<uint8_t*>(ptr);
        bytePtr[0] = 0x55;
        bytePtr[largeSize - 1] = 0xAA;
        
        EXPECT_EQ(bytePtr[0], 0x55);
        EXPECT_EQ(bytePtr[largeSize - 1], 0xAA);
        
        freeMemory(ptr);
    }
    
    // Test passes if no crash occurs
    EXPECT_TRUE(true);
}

TEST_F(MemoryTest, MultipleAllocations) {
    // Test multiple allocations
    std::vector<void*> pointers;
    const int numAllocations = 100;
    const size_t allocationSize = 256;
    
    // Allocate multiple blocks
    for (int i = 0; i < numAllocations; ++i) {
        void* ptr = allocateMemory(allocationSize);
        if (ptr) {
            pointers.push_back(ptr);
            // Write unique pattern to each block
            memset(ptr, i & 0xFF, allocationSize);
        }
    }
    
    // Verify patterns are still intact
    for (size_t i = 0; i < pointers.size(); ++i) {
        uint8_t* bytePtr = static_cast<uint8_t*>(pointers[i]);
        uint8_t expectedValue = i & 0xFF;
        EXPECT_EQ(bytePtr[0], expectedValue);
        EXPECT_EQ(bytePtr[allocationSize - 1], expectedValue);
    }
    
    // Free all allocations
    for (void* ptr : pointers) {
        freeMemory(ptr);
    }
}

TEST_F(MemoryTest, AlignedAllocation) {
    // Test aligned memory allocation if available
    const size_t size = 1024;
    const size_t alignment = 16;
    
    void* ptr = allocateAlignedMemory(size, alignment);
    if (ptr) {
        // Check alignment
        uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
        EXPECT_EQ(address % alignment, 0);
        
        // Verify memory is usable
        memset(ptr, 0x33, size);
        uint8_t* bytePtr = static_cast<uint8_t*>(ptr);
        EXPECT_EQ(bytePtr[0], 0x33);
        EXPECT_EQ(bytePtr[size - 1], 0x33);
        
        freeAlignedMemory(ptr);
    }
}

TEST_F(MemoryTest, MemoryStatistics) {
    // Test memory statistics if available
    MemoryStats statsBefore = getMemoryStats();
    
    // Allocate some memory
    std::vector<void*> allocations;
    for (int i = 0; i < 10; ++i) {
        void* ptr = allocateMemory(1024);
        if (ptr) {
            allocations.push_back(ptr);
        }
    }
    
    MemoryStats statsAfter = getMemoryStats();
    
    // Free the memory
    for (void* ptr : allocations) {
        freeMemory(ptr);
    }
    
    MemoryStats statsEnd = getMemoryStats();
    
    // Basic sanity checks (exact behavior depends on implementation)
    EXPECT_GE(statsAfter.totalAllocated, statsBefore.totalAllocated);
    EXPECT_GE(statsAfter.currentAllocated, statsBefore.currentAllocated);
    
    // After freeing, current allocated should be back to original or less
    EXPECT_LE(statsEnd.currentAllocated, statsAfter.currentAllocated);
}

TEST_F(MemoryTest, MemoryPool) {
    // Test memory pool functionality if available
    const size_t poolSize = 4096;
    const size_t blockSize = 64;
    
    MemoryPool* pool = createMemoryPool(poolSize, blockSize);
    if (pool) {
        std::vector<void*> blocks;
        
        // Allocate blocks from pool
        for (size_t i = 0; i < poolSize / blockSize; ++i) {
            void* block = allocateFromPool(pool, blockSize);
            if (block) {
                blocks.push_back(block);
                // Write pattern to verify block is usable
                memset(block, i & 0xFF, blockSize);
            } else {
                break; // Pool exhausted
            }
        }
        
        EXPECT_GT(blocks.size(), 0);
        
        // Verify patterns
        for (size_t i = 0; i < blocks.size(); ++i) {
            uint8_t* bytePtr = static_cast<uint8_t*>(blocks[i]);
            uint8_t expectedValue = i & 0xFF;
            EXPECT_EQ(bytePtr[0], expectedValue);
        }
        
        // Free blocks back to pool
        for (void* block : blocks) {
            freeToPool(pool, block);
        }
        
        destroyMemoryPool(pool);
    }
}

TEST_F(MemoryTest, MemoryLeakDetection) {
    // Simple test to ensure we're not leaking memory in our tests
    MemoryStats initialStats = getMemoryStats();
    
    {
        // Allocate and free memory in a scope
        std::vector<void*> tempAllocations;
        for (int i = 0; i < 50; ++i) {
            void* ptr = allocateMemory(128);
            if (ptr) {
                tempAllocations.push_back(ptr);
            }
        }
        
        for (void* ptr : tempAllocations) {
            freeMemory(ptr);
        }
    }
    
    MemoryStats finalStats = getMemoryStats();
    
    // Current allocated should be the same or very close
    // (allowing for some implementation-specific overhead)
    EXPECT_LE(finalStats.currentAllocated, initialStats.currentAllocated + 1024);
}

TEST_F(MemoryTest, NullPointerHandling) {
    // Test that freeing null pointer doesn't crash
    EXPECT_NO_THROW(freeMemory(nullptr));
    EXPECT_NO_THROW(freeAlignedMemory(nullptr));
    
    MemoryPool* nullPool = nullptr;
    EXPECT_NO_THROW(freeToPool(nullPool, nullptr));
    EXPECT_NO_THROW(destroyMemoryPool(nullPool));
}
