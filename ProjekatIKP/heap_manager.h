// heap_manager.h
#ifndef HEAP_MANAGER_H_INCLUDED
#define HEAP_MANAGER_H_INCLUDED

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

struct MemorySegment {
    void* address;
    int size;
    bool allocated;
};

struct HeapManager {
    struct MemorySegment* memorySegments;
    int segmentsCount;
    int allocationsCount;
    int deallocationsCount;
    int nextFitIndex;
    pthread_mutex_t mutex;
};

void initialize_heap_manager(struct HeapManager* heapManager, int segmentsCount, int segmentSize);

void* allocate_memory(struct HeapManager* heapManager, int size);

void free_memory(struct HeapManager* heapManager, void* address);

double fragmentation_degree(const struct HeapManager* heapManager);

void cleanup_heap_manager(struct HeapManager* heapManager);

int get_allocations_count(const struct HeapManager* heapManager);

int get_deallocations_count(const struct HeapManager* heapManager);

double get_fragmentation_degree(const struct HeapManager* heapManager);

#endif // HEAP_MANAGER_H_INCLUDED
