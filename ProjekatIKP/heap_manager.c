// heap_manager.c
#include "heap_manager.h"

void initialize_heap_manager(struct HeapManager* heapManager, int segmentsCount, int segmentSize) {
    heapManager->memorySegments = (struct MemorySegment*)malloc(segmentsCount * sizeof(struct MemorySegment));
    heapManager->segmentsCount = segmentsCount;
    heapManager->allocationsCount = 0;
    heapManager->deallocationsCount = 0;
    heapManager->nextFitIndex = 0;
    pthread_mutex_init(&heapManager->mutex, NULL);

    for (int i = 0; i < segmentsCount; ++i) {
        // Allocate memory for each segment
        heapManager->memorySegments[i].address = malloc(sizeof(char) * segmentSize);
        heapManager->memorySegments[i].size = segmentSize;
        heapManager->memorySegments[i].allocated = false;
    }
}


void* allocate_memory(struct HeapManager* heapManager, int size) {
    pthread_mutex_lock(&heapManager->mutex);

    // Next Fit algorithm
    int startIndex = heapManager->nextFitIndex;
    for (int i = 0; i < heapManager->segmentsCount; ++i) {
        int index = (startIndex + i) % heapManager->segmentsCount;


        if (!heapManager->memorySegments[index].allocated && heapManager->memorySegments[index].size >= size) {
            heapManager->memorySegments[index].allocated = true;
            heapManager->allocationsCount++;
            heapManager->nextFitIndex = (index + 1) % heapManager->segmentsCount;
            pthread_mutex_unlock(&heapManager->mutex);

            printf("Allocation successful. Returning address: %p\n", heapManager->memorySegments[index].address);

            return heapManager->memorySegments[index].address;
        }
    }

    // If no suitable segment is found
    printf("Allocation failed.\n");

    pthread_mutex_unlock(&heapManager->mutex);
    return NULL;
}




void free_memory(struct HeapManager* heapManager, void* address) {
    pthread_mutex_lock(&heapManager->mutex);

    for (int i = 0; i < heapManager->segmentsCount; ++i) {
        if (heapManager->memorySegments[i].address == address && heapManager->memorySegments[i].allocated) {
            heapManager->memorySegments[i].allocated = false;
            heapManager->deallocationsCount++;
            break;
        }
    }

    pthread_mutex_unlock(&heapManager->mutex);
}

double fragmentation_degree(const struct HeapManager* heapManager) {
    int freeSegmentsCount = 0;
    for (int i = 0; i < heapManager->segmentsCount; ++i) {
        if (!heapManager->memorySegments[i].allocated) {
            freeSegmentsCount++;
        }
    }

    return (double)freeSegmentsCount / heapManager->segmentsCount;
}

void cleanup_heap_manager(struct HeapManager* heapManager) {
    for (int i = 0; i < heapManager->segmentsCount; ++i) {
        free(heapManager->memorySegments[i].address);
    }
    free(heapManager->memorySegments);
    pthread_mutex_destroy(&heapManager->mutex);
}


int get_allocations_count(const struct HeapManager* heapManager) {
    return heapManager->allocationsCount;
}

int get_deallocations_count(const struct HeapManager* heapManager) {
    return heapManager->deallocationsCount;
}

double get_fragmentation_degree(const struct HeapManager* heapManager) {
    int freeSegmentsCount = 0;
    for (int i = 0; i < heapManager->segmentsCount; ++i) {
        if (!heapManager->memorySegments[i].allocated) {
            freeSegmentsCount++;
        }
    }

    return (double)freeSegmentsCount / heapManager->segmentsCount;
}
