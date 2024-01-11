#ifndef HEAP_MANAGER_H_INCLUDED
#define HEAP_MANAGER_H_INCLUDED
#include <stdbool.h>

typedef enum {
    FREE,
    ALLOCATED
} BlockStatus;

typedef struct Block {
    int size;
    BlockStatus status;
    struct Block* next;
} Block;

Block* find_free_block(int size);
void* allocate_memory(int size);
void free_memory(void* address);
double fragmentation_degree();
int isValidAddress(void* address);

#endif // HEAP_MANAGER_H_INCLUDED
