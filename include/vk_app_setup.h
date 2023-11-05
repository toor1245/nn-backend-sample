#ifndef MATRIX_DOTNET_CORE_VK_APP_SETUP_H
#define MATRIX_DOTNET_CORE_VK_APP_SETUP_H

#include "macros.h"

static const char *enabledInstanceExtensions[] = {
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

static const char *validationLayers[] = {
        "VK_LAYER_KHRONOS_validation"
};

static bool vkIsExistsValidationLayer() {
    VkResult result;

    uint32_t layerCount;
    result = vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    CHECK_RESULT(result);

    VkLayerProperties* availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
    result = vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    CHECK_RESULT(result);

    bool layerFound = false;
    for (int i = 0; i < 1; ++i) {
        const char* layerName = validationLayers[i];

        for (int j = 0; j < layerCount; ++j) {
            if (strcmp(layerName, availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
            if (!layerFound) {
                layerFound = false;
            }
        }
    }
    return layerFound;
}

VkResult vkCreateDefaultApplication(VkInstance* instance) {
    VkApplicationInfo applicationInfo = {
            VK_STRUCTURE_TYPE_APPLICATION_INFO,
            NULL,
            "CNN library",
            1,
            "none",
            0,
            VK_MAKE_VERSION(1, 3, 0),
    };

    VkInstanceCreateInfo instanceCreateInfo = {
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            NULL,
            0,
            &applicationInfo,
            1,
            validationLayers,
            2,
            enabledInstanceExtensions,
    };

    if (!vkIsExistsValidationLayer())
        exit(EXIT_FAILURE);

    VkResult result;
    result = vkCreateInstance(&instanceCreateInfo, NULL, instance);
    return result;
}

#endif //MATRIX_DOTNET_CORE_VK_APP_SETUP_H
