#ifndef MATRIX_DOTNET_CORE_MACROS_H
#define MATRIX_DOTNET_CORE_MACROS_H

#include <stdio.h>
#include <stdlib.h>
#include "vulkan/vulkan.h"

#define CHECK_RESULT(r) do { \
    if ((r) != VK_SUCCESS) {    \
        printf("result = %d, line = %d\n", (r), __LINE__);  \
        exit(0);  \
    }   \
} while (0)

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

#endif //MATRIX_DOTNET_CORE_MACROS_H
