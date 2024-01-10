// heap_manager.c
#include "heap_manager.hpp"

void initialize_heap_manager(struct HeapManager* heapManager, int segmentsCount) {
    heapManager->memorySegments = (struct MemorySegment*)malloc(segmentsCount * sizeof(struct MemorySegment));
    heapManager->segmentsCount = segmentsCount;
    heapManager->allocationsCount = 0;
    heapManager->deallocationsCount = 0;
    heapManager->nextFitIndex = 0;
    pthread_mutex_init(&heapManager->mutex, NULL);

    // Inicijalizacija segmenata prema potrebi
    for (int i = 0; i < segmentsCount; ++i) {
        heapManager->memorySegments[i].address = NULL;
        heapManager->memorySegments[i].size = 0;
        heapManager->memorySegments[i].allocated = false;
    }
}

void* allocate_memory(struct HeapManager* heapManager, int size) {
    pthread_mutex_lock(&heapManager->mutex);

    // Next Fit algoritam
    for (int i = 0; i < heapManager->segmentsCount; ++i) {
        int index = (heapManager->nextFitIndex + i) % heapManager->segmentsCount;
        if (!heapManager->memorySegments[index].allocated && heapManager->memorySegments[index].size >= size) {
            heapManager->memorySegments[index].allocated = true;
            heapManager->allocationsCount++;
            heapManager->nextFitIndex = (index + 1) % heapManager->segmentsCount;
            pthread_mutex_unlock(&heapManager->mutex);
            return heapManager->memorySegments[index].address;
        }
    }

    // Ako nije pronađen odgovarajući segment
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
