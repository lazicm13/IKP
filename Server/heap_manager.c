#include "heap_manager.h"
#include<stddef.h>
#define SEGMENT_SIZE 1024 // Predefinisana veličina segmenta
#define FREE_SEGMENT_LIMIT 5 // Granica za oslobađanje nekorišćenih segmenata
#include <windows.h>

// Global variables to keep track of the current position in the heap, the number of free segments, and the last allocated block
static Block* head = NULL;
static int freeSegments = 0;
static Block* lastAllocatedBlock = NULL;
Block* allocatedBlocks = NULL;
CRITICAL_SECTION heapCriticalSection;

// Function to find a free block using next fit algorithm
struct Block* find_free_block(int size) {
    struct Block* current_block = lastAllocatedBlock;

    // Iterate through the free blocks starting from the last allocated block
    do {
        if (current_block == NULL || current_block->size >= size) {
            // Found a suitable block or reached the end, break the loop
            break;
        }
        current_block = current_block->next;
    } while (current_block != lastAllocatedBlock);

    // Update the last allocated block for the next iteration
    lastAllocatedBlock = current_block;

    return current_block;
}

// Function to allocate memory using the next fit algorithm
void* allocate_memory(int size) {
    // Find a suitable free block using next fit algorithm
    struct Block* free_block = find_free_block(size);

    // If no suitable block is found, allocate a new block
    if (free_block == NULL || free_block->size < size) {
        // Calculate the number of segments needed for the requested size
        int segments_needed = (size + sizeof(struct Block) + SEGMENT_SIZE - 1) / SEGMENT_SIZE;

        // Allocate memory for the requested segments
        struct Block* new_block = (struct Block*)malloc(sizeof(struct Block) + segments_needed * SEGMENT_SIZE);
        if (new_block == NULL) {
            // Memory allocation failed
            return NULL;
        }

        new_block->size = segments_needed * SEGMENT_SIZE;
        new_block->next = head;
        new_block->status = ALLOCATED;
        head = new_block;

        lastAllocatedBlock = new_block; // Update the last allocated block
        return (void*)(new_block + 1); // Return the address right after the block header
    } else {
        // Use the current free block
        free_block->status = ALLOCATED;
        lastAllocatedBlock = free_block;
        return (void*)(free_block + 1); // Return the address right after the block header
    }
}

// Function to free memory
void free_memory(void* address) {
    if (address == NULL) {
        return;
    }

    // Move back to the block header
    struct Block* block = ((struct Block*)address) - 1;

    // Lock the heap manager for exclusive access
    EnterCriticalSection(&heapCriticalSection);

    if (block->status == ALLOCATED) {
        // Link the block to the free list
        block->next = head;
        block->status = FREE;
        head = block;

        // Increment the count of free segments
        freeSegments++;
    } else {
        printf("Error: Attempted to free an already free block.\n");
        LeaveCriticalSection(&heapCriticalSection);
        return;
    }

    // Unlock the heap manager
    LeaveCriticalSection(&heapCriticalSection);

    // Check if the limit for free segments is reached, and free memory if needed
    if (freeSegments >= FREE_SEGMENT_LIMIT) {
        // Lock the heap manager for exclusive access
        EnterCriticalSection(&heapCriticalSection);

        // Find the first free block
        struct Block* firstFreeBlock = head;
        while (firstFreeBlock != NULL && firstFreeBlock->status != FREE) {
            firstFreeBlock = firstFreeBlock->next;
        }

        if (firstFreeBlock != NULL) {
            // Set head to the first free block
            head = firstFreeBlock;

            // Free all consecutive free segments
            while (head != NULL && head->next != NULL && head->next->status == FREE) {
                struct Block* temp = head->next;
                head->next = temp->next;
                free(temp);
                freeSegments--;
            }
        }

        // Unlock the heap manager
        LeaveCriticalSection(&heapCriticalSection);
    }
}

// Function to calculate the fragmentation degree
double fragmentation_degree() {
    EnterCriticalSection(&heapCriticalSection);

    int freeSegmentsCount = 0;
    int totalSegmentsCount = 0;

    struct Block* currentBlock = head;

    // Iterate through the list and count the total number of segments and free segments
    while (currentBlock != NULL) {
        totalSegmentsCount++;
        if (currentBlock->status == FREE) {
            freeSegmentsCount++;
            printf("Free segments counter: %d\n", freeSegmentsCount);
        }
        currentBlock = currentBlock->next;
    }

    LeaveCriticalSection(&heapCriticalSection);

    if (totalSegmentsCount == 0) {
        // If we have no allocated memory, fragmentation is 0%
        return 0.0;
    }
    printf("Total segments counter: %d\n", totalSegmentsCount);
    // Calculate the fragmentation degree as the ratio of free segments to total segments
    double fragmentationDegree = (double)freeSegmentsCount / totalSegmentsCount * 100.0;

    return fragmentationDegree;
}
