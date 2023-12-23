#include <stdio.h>
#include <stdlib.h>

#define SEGMENT_SIZE 1024
#define NUM_SEGMENTS 100

typedef struct Segment {
    int is_free;
    int size;
    void *address; // Adresa početka bloka
} Segment;

Segment segments[NUM_SEGMENTS];  // Niz segmenata

int next_fit_index = 0;  // Indeks za 'Next fit' algoritam

void initialize_heap_manager() {
    for (int i = 0; i < NUM_SEGMENTS; ++i) {
        segments[i].is_free = 1;  // Svi blokovi su na početku slobodni
        segments[i].size = SEGMENT_SIZE;
        segments[i].address = malloc(SEGMENT_SIZE);  // Alokacija memorije za svaki blok
    }
}

void *allocate_memory(int size) {
    int remaining_size = size;
    void *allocated_block = NULL;

    while (remaining_size > 0 && next_fit_index < NUM_SEGMENTS) {
        if (segments[next_fit_index].is_free && segments[next_fit_index].size >= remaining_size) {
            allocated_block = segments[next_fit_index].address;
            segments[next_fit_index].is_free = 0;  // Oznaci blok kao zauzet

            // Ako je blok veći od potrebne veličine, podeli ga na dva bloka
            if (segments[next_fit_index].size > size) {
                int remaining_segment_size = segments[next_fit_index].size - size;
                segments[next_fit_index].size = size;

                // Napravi novi slobodan blok od preostalog dela segmenta
                segments[next_fit_index + 1].is_free = 1;
                segments[next_fit_index + 1].size = remaining_segment_size;
                segments[next_fit_index + 1].address = (char *)allocated_block + size;
            }

            next_fit_index++;

            remaining_size = 0;  // Izađi iz petlje
        } else {
            next_fit_index++;  // Pomeri se na sledeći blok
        }
    }

    return allocated_block;
}

void free_memory(void *address) {
    for (int i = 0; i < NUM_SEGMENTS; ++i) {
        if (segments[i].address == address) {
            segments[i].is_free = 1;  // Oznaci blok kao slobodan

            // Spajanje uzastopnih slobodnih segmenata
            while (i < NUM_SEGMENTS - 1 && segments[i + 1].is_free) {
                segments[i].size += segments[i + 1].size;
                ++i;
            }

            break;
        }
    }
}

int main() {
    initialize_heap_manager();

    // Primer korišćenja
    void *ptr1 = allocate_memory(200);  // Alokacija memorije od 200 bajtova
    void *ptr2 = allocate_memory(400);
    free_memory(ptr1);

    return 0;
}
