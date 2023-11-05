#ifndef MATRIX_DOTNET_CORE_VK_PHYS_DEVICE_SETUP_H
#define MATRIX_DOTNET_CORE_VK_PHYS_DEVICE_SETUP_H

#include <stdint.h>
#include "vulkan/vulkan.h"

VkPhysicalDevice* vkGetPhysicalDevices(VkInstance instance, uint32_t* physical_devices_count) {
    uint32_t num_physical_devices;
    CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &num_physical_devices, NULL));

    *physical_devices_count = num_physical_devices;
    VkPhysicalDevice*physical_devices = (VkPhysicalDevice *) malloc(sizeof(VkPhysicalDevice) * num_physical_devices);
    CHECK_RESULT(vkEnumeratePhysicalDevices(instance, &num_physical_devices, physical_devices));
    return physical_devices;
}

int vkGetDeviceIndexByExtensionName(VkPhysicalDevice *physical_devices, uint32_t num_physical_devices, const char* ext_name) {
    VkResult result;
    int device_index = -1;

    for (int i = 0; i < num_physical_devices; ++i) {
        uint32_t num_extensions = 0;

        result = vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &num_extensions, NULL);
        CHECK_RESULT(result);

        VkExtensionProperties *extensions = (VkExtensionProperties *) malloc(
                sizeof(VkExtensionProperties) * num_extensions);
        result = vkEnumerateDeviceExtensionProperties(physical_devices[i], NULL, &num_extensions, extensions);
        CHECK_RESULT(result);

        for (int j = 0; j < num_extensions; ++j) {
            if (strcmp(extensions[j].extensionName, ext_name) == 0) {
                device_index = i;
                break;
            }
        }
    }
    return device_index;
}

#endif //MATRIX_DOTNET_CORE_VK_PHYS_DEVICE_SETUP_H
