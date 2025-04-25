#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sorting.h"

#define MAX_DATA 2000000
#define MAX_WORD_LEN 100
#define DATA_ANGKA "data_angka.txt"
#define DATA_KATA "data_kata.txt"

typedef struct {
    char name[20];
    double time_sec;
    double memory_mb;
} Result;

int* load_int_data(const char* filename, size_t count) {
    FILE* file = fopen(filename, "r");
    if (!file) { perror("Gagal membuka file angka"); exit(EXIT_FAILURE); }
    int* data = malloc(count * sizeof(int));
    for (size_t i = 0; i < count; i++) fscanf(file, "%d", &data[i]);
    fclose(file);
    return data;
}

char** load_str_data(const char* filename, size_t count) {
    FILE* file = fopen(filename, "r");
    if (!file) { perror("Gagal membuka file kata"); exit(EXIT_FAILURE); }
    char** data = malloc(count * sizeof(char*));
    char buffer[MAX_WORD_LEN];
    for (size_t i = 0; i < count; i++) {
        fgets(buffer, MAX_WORD_LEN, file);
        buffer[strcspn(buffer, "\n")] = '\0';
        data[i] = strdup(buffer);
    }
    fclose(file);
    return data;
}

void free_str_data(char** data, size_t count) {
    for (size_t i = 0; i < count; i++) free(data[i]);
    free(data);
}

int* copy_int_array(int* source, size_t n) {
    int* copy = malloc(n * sizeof(int));
    memcpy(copy, source, n * sizeof(int));
    return copy;
}

char** copy_str_array(char** source, size_t n) {
    char** copy = malloc(n * sizeof(char*));
    for (size_t i = 0; i < n; i++) copy[i] = strdup(source[i]);
    return copy;
}

double to_mb(size_t bytes) {
    return (double)bytes / (1024.0 * 1024.0);
}

Result measure_int_sort_mem(const char* name, void (*sort_func)(int*, size_t), int* data, size_t size) {
    int* copy = copy_int_array(data, size);
    clock_t start = clock();
    sort_func(copy, size);
    clock_t end = clock();
    double time_sec = (double)(end - start) / CLOCKS_PER_SEC;
    size_t mem_bytes = size * sizeof(int) * 2; // original + copy
    Result r = {0};
    strcpy(r.name, name);
    r.time_sec = time_sec;
    r.memory_mb = to_mb(mem_bytes);
    free(copy);
    return r;
}

Result measure_int_sort_range_mem(const char* name, void (*sort_func)(int*, int, int), int* data, size_t size, int extra_factor) {
    int* copy = copy_int_array(data, size);
    clock_t start = clock();
    sort_func(copy, 0, size - 1);
    clock_t end = clock();
    double time_sec = (double)(end - start) / CLOCKS_PER_SEC;
    size_t mem_bytes = size * sizeof(int) * (1 + extra_factor); // copy + extra if needed
    Result r = {0};
    strcpy(r.name, name);
    r.time_sec = time_sec;
    r.memory_mb = to_mb(mem_bytes);
    free(copy);
    return r;
}

Result measure_str_sort_mem(const char* name, void (*sort_func)(char**, size_t), char** data, size_t size) {
    char** copy = copy_str_array(data, size);
    clock_t start = clock();
    sort_func(copy, size);
    clock_t end = clock();
    double time_sec = (double)(end - start) / CLOCKS_PER_SEC;

    size_t mem_bytes = sizeof(char*) * size; // pointer array
    for (size_t i = 0; i < size; i++) {
        mem_bytes += strlen(copy[i]) + 1;
        free(copy[i]);
    }
    free(copy);

    Result r = {0};
    strcpy(r.name, name);
    r.time_sec = time_sec;
    r.memory_mb = to_mb(mem_bytes);
    return r;
}

Result measure_str_sort_range_mem(const char* name, void (*sort_func)(char**, int, int), char** data, size_t size, int extra_factor) {
    char** copy = copy_str_array(data, size);
    clock_t start = clock();
    sort_func(copy, 0, size - 1);
    clock_t end = clock();
    double time_sec = (double)(end - start) / CLOCKS_PER_SEC;

    size_t mem_bytes = sizeof(char*) * size * (1 + extra_factor); // pointer array copy + extra
    for (size_t i = 0; i < size; i++) {
        mem_bytes += strlen(copy[i]) + 1;
        free(copy[i]);
    }
    free(copy);

    Result r = {0};
    strcpy(r.name, name);
    r.time_sec = time_sec;
    r.memory_mb = to_mb(mem_bytes);
    return r;
}

void print_table(Result* results, int count, const char* title) {
    printf("\n%s\n", title);
    printf("+-----------------+------------+--------------+\n");
    printf("| %-15s | %-10s | %-12s |\n", "Algoritma", "Waktu (s)", "Memori (MB)");
    printf("+-----------------+------------+--------------+\n");
    for (int i = 0; i < count; i++) {
        printf("| %-15s | %10.2f | %12.2f |\n", results[i].name, results[i].time_sec, results[i].memory_mb);
    }
    printf("+-----------------+------------+--------------+\n");
}

int main() {
    char ulang;
    do {
        size_t jumlah;
        printf("Masukkan jumlah data yang ingin diuji (max %d): ", MAX_DATA);
        scanf("%zu", &jumlah);

        // Data Angka
        int* angka = load_int_data(DATA_ANGKA, jumlah);
        Result r_angka[6] = {
            measure_int_sort_mem("Bubble Sort", bubble_sort, angka, jumlah),
            measure_int_sort_mem("Selection Sort", selection_sort, angka, jumlah),
            measure_int_sort_mem("Insertion Sort", insertion_sort, angka, jumlah),
            measure_int_sort_range_mem("Merge Sort", merge_sort, angka, jumlah, 1),
            measure_int_sort_range_mem("Quick Sort", quick_sort, angka, jumlah, 1),
            measure_int_sort_mem("Shell Sort", shell_sort, angka, jumlah)
        };
        free(angka);

        // Data Kata
        char** kata = load_str_data(DATA_KATA, jumlah);
        Result r_kata[6] = {
            measure_str_sort_mem("Bubble Sort", bubble_sort_str, kata, jumlah),
            measure_str_sort_mem("Selection Sort", selection_sort_str, kata, jumlah),
            measure_str_sort_mem("Insertion Sort", insertion_sort_str, kata, jumlah),
            measure_str_sort_range_mem("Merge Sort", merge_sort_str, kata, jumlah, 1),
            measure_str_sort_range_mem("Quick Sort", quick_sort_str, kata, jumlah, 1),
            measure_str_sort_mem("Shell Sort", shell_sort_str, kata, jumlah)
        };
        free_str_data(kata, jumlah);

        // Cetak hasil
        print_table(r_angka, 6, "Hasil Sorting Data Angka:");
        print_table(r_kata, 6, "Hasil Sorting Data Kata:");

        printf("\nIngin menjalankan program lagi? (y/n): ");
        scanf(" %c", &ulang);
    } while (ulang == 'y' || ulang == 'Y');

    return 0;
}