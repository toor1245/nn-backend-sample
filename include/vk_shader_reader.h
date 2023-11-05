#ifndef MATRIX_DOTNET_CORE_VK_SHADER_READER_H
#define MATRIX_DOTNET_CORE_VK_SHADER_READER_H

#include "stdio.h"
#include "stdlib.h"
#include "vulkan/vulkan.h"

typedef struct {
    unsigned char* shader_code_ptr;
    size_t bytes;
} ShaderInfo;

ShaderInfo vkGetShaderInfo(const char* path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        printf("file %s not found!", path);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *shader_code = (unsigned char *) malloc(file_size);

    if (shader_code == NULL) {
        printf("Failed to allocate memory for shader code.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    size_t bytes_read = fread(shader_code, 1, file_size, file); // Read the shader code

    if (bytes_read != file_size) {
        printf("Failed to read shader file.\n");

        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);

    return (ShaderInfo) {
        .shader_code_ptr = shader_code,
        .bytes = bytes_read
    };
}

#endif //MATRIX_DOTNET_CORE_VK_SHADER_READER_H
