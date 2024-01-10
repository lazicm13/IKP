#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "heap_manager.h"

void* thread_function(void* arg) {
    struct HeapManager* heapManager = (struct HeapManager*)arg;

    for (int i = 0; i < 3; i++) {
        int* value = (int*)allocate_memory(heapManager, sizeof(int));
        if (value != NULL) {
            *value = i;
            printf("Thread %lu allocated value: %d\n", pthread_self(), *value);
        } else {
            printf("Thread %lu failed to allocate memory\n", pthread_self());
        }
    }

    return NULL;
}

int main() {
    const int segmentsCount = 50;
    const int segmentSize = sizeof(int);  // Adjust segment size for the desired type
    struct HeapManager heapManager;
    initialize_heap_manager(&heapManager, segmentsCount, segmentSize);

    // Testing Heap Manager in the main thread
    for (int i = 0; i < 5; i++) {
        int* intValue = (int*)allocate_memory(&heapManager, sizeof(int));
        if (intValue != NULL) {
            printf("Main thread allocated int value: %d\n", *intValue);
        } else {
            printf("Main thread failed to allocate memory\n");
        }
    }

    for (int i = 0; i < 3; i++) {
        int* value = (int*)allocate_memory(&heapManager, sizeof(int));
        if (value != NULL) {
            printf("Main thread allocated int value: %d\n", *value);

            free_memory(&heapManager, value);
            printf("Main thread deallocated int value: %d\n", *value);
        } else {
            printf("Main thread failed to allocate memory\n");
        }
    }

    // Testing Heap Manager in additional threads
    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_function, &heapManager);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    // Displaying the number of allocations, deallocations, and fragmentation degree
    printf("\nAllocations count: %d\n", get_allocations_count(&heapManager));
    printf("Deallocations count: %d\n", get_deallocations_count(&heapManager));
    printf("Fragmentation degree: %f\n", get_fragmentation_degree(&heapManager));

    cleanup_heap_manager(&heapManager);

    return 0;
}
