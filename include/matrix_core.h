#ifndef MATRIX_DOTNET_CORE_MATRIX_CORE_H
#define MATRIX_DOTNET_CORE_MATRIX_CORE_H

#include "vulkan/vulkan.h"

typedef struct {
    VkComponentTypeNV data_type;
    VkBuffer host_buffer;
    VkBuffer device_buffer;
    VkDeviceMemory host_memory;
    VkDeviceMemory device_memory;
} matrix_descriptor_t;

typedef struct {
    uint32_t rows;
    uint32_t columns;
    uint32_t length;
    size_t element_size;
    size_t buffer_size;
    void *ptr;
    matrix_descriptor_t descriptor;
} matrix_t;

static const matrix_t matrix_empty;

matrix_t matrix_new(void *ptr, uint32_t rows, uint32_t columns, uint32_t el_size) {
    matrix_t matrix = matrix_empty;
    matrix.ptr = ptr;
    matrix.rows = rows;
    matrix.columns = columns;
    matrix.length = rows * columns;
    matrix.element_size = el_size;
    matrix.buffer_size = matrix.length * el_size;
    return matrix;
}

int8_t matrix_get_i8(matrix_t *matrix, uint32_t i) {
    void *ptr = matrix->ptr;
    return ((int8_t *) ptr)[i];
}

uint8_t matrix_get_u8(matrix_t *matrix, uint32_t i) {
    void *ptr = matrix->ptr;
    return ((uint8_t *) ptr)[i];
}

int32_t matrix_get_i32(matrix_t *matrix, int32_t i) {
    void *ptr = matrix->ptr;
    return ((int32_t *) ptr)[i];
}

uint32_t matrix_get_u32(matrix_t *matrix, uint32_t i) {
    void *ptr = matrix->ptr;
    return ((uint32_t *) ptr)[i];
}

void matrix_set_i8(matrix_t *matrix, uint32_t i, int8_t value) {
    void *ptr = matrix->ptr;
    ((int8_t *) ptr)[i] = value;
}

void matrix_set_u8(matrix_t *matrix, uint32_t i, uint8_t value) {
    void *ptr = matrix->ptr;
    ((uint8_t *) ptr)[i] = value;
}

void matrix_set_i32(matrix_t *matrix, int32_t i, int32_t value) {
    void *ptr = matrix->ptr;
    ((int32_t *) ptr)[i] = value;
}

void matrix_set_u32(matrix_t *matrix, uint32_t i, uint32_t value) {
    void *ptr = matrix->ptr;
    ((uint32_t *) ptr)[i] = value;
}

void matrix_print(matrix_t* matrix) {
    printf("\n");
    for (int i = 0; i < 1024; ++i) {
        for (int j = 0; j < 1024; ++j) {
            printf("%d ", matrix_get_i32(matrix, i * 1024 + j));
        }
        printf("\n");
    }
}

int* matrix_init_random(int rows, int columns) {
    int *array = (int *) malloc(sizeof(int) * (rows * columns));
    for (int i = 0; i < rows * columns; ++i)
        array[i] = rand() % 100; // NOLINT(*-msc50-cpp)
    return array;
}

#endif //MATRIX_DOTNET_CORE_MATRIX_CORE_H
