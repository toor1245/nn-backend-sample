#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "vulkan/vulkan.h"
#include "include/vk_app_setup.h"
#include "include/vk_debug_setup.h"
#include "include/matrix_core.h"
#include "include/vk_memory_utils.h"
#include "include/vk_phys_device_setup.h"
#include "include/vk_descriptor_setup.h"
#include "include/vk_shader_reader.h"

int main() {
    VkResult result;

    VkInstance instance;
    result = vkCreateDefaultApplication(&instance);
    CHECK_RESULT(result);

    VkDebugUtilsMessengerEXT debugMessenger;
    result = vkCreateDebugUtilsMessenger(instance, &debugMessenger);
    CHECK_RESULT(result);

    uint32_t num_physical_devices;
    VkPhysicalDevice *physical_devices = vkGetPhysicalDevices(instance, &num_physical_devices);
    CHECK_RESULT(result);

    int nvidia_device_index = vkGetDeviceIndexByExtensionName(physical_devices, num_physical_devices, VK_NV_COOPERATIVE_MATRIX_EXTENSION_NAME);
    if (nvidia_device_index == -1) {
        printf("couldn't find physical device that supports extension\n");
        return 0;
    }

    VkPhysicalDevice nvidia_device = physical_devices[nvidia_device_index];
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(nvidia_device, &memoryProperties);

    uint32_t num_queue_families = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(nvidia_device, &num_queue_families, NULL);

    VkQueueFamilyProperties *queue_family_properties = malloc(sizeof(VkQueueFamilyProperties) * num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(nvidia_device, &num_queue_families, &queue_family_properties[0]);

    int queue_compute_index = -1;

    for (int i = 0; i < num_queue_families; ++i) {
        if (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
            queue_compute_index = i;
            break;
        }
    }

    if (queue_compute_index == -1) {
        printf("couldn't find compute queue\n");
        return 0;
    }

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            NULL,
            0,
            (uint32_t) queue_compute_index,
            1,
            &queuePriority,
    };

    VkPhysicalDeviceBufferAddressFeaturesEXT bufferDeviceAddressFeatures = {
            VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_ADDRESS_FEATURES_EXT,
            NULL,
            VK_TRUE, // bufferDeviceAddress
            VK_FALSE, // bufferDeviceAddressCaptureReplay
            VK_FALSE, // bufferDeviceAddressMultiDevice
    };

    const char *enabledDeviceExtensions[] = {
            VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
            VK_EXT_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
    };

    VkDeviceCreateInfo deviceCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            &bufferDeviceAddressFeatures,
            0,
            1,
            &deviceQueueCreateInfo,
            0,
            NULL,
            2,
            enabledDeviceExtensions,
            NULL,
    };

    VkDevice device;
    result = vkCreateDevice(nvidia_device, &deviceCreateInfo, NULL, &device);
    CHECK_RESULT(result);


    VkQueue queue;
    vkGetDeviceQueue(device, (uint32_t) queue_compute_index, 0, &queue);

    uint32_t layoutBindingsCount = 3;
    VkDescriptorSetLayoutBinding* layoutBindings = (VkDescriptorSetLayoutBinding*) malloc(sizeof(VkDescriptorSetLayoutBinding) * layoutBindingsCount);

    for (int i = 0; i < layoutBindingsCount; ++i) {
        layoutBindings[i] = vkCreateDescriptorSetLayoutBindingCompute(i);
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            NULL,
            0,
            layoutBindingsCount,
            layoutBindings,
    };

    VkDescriptorSetLayout descriptorSetLayout;
    result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, NULL, &descriptorSetLayout);
    CHECK_RESULT(result);

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            NULL,
            0,
            1,
            &descriptorSetLayout,
            0,
            NULL
    };

    VkPipelineLayout pipelineLayout;
    result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
    CHECK_RESULT(result);

    ShaderInfo shaderInfo = vkGetShaderInfo("/home/mhohsadze/CLionProjects/matrix-dotnet-core/cmake-build-debug/gemm2.spv");

    VkShaderModuleCreateInfo shader_module_create_info = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .pCode = (const uint32_t*) shaderInfo.shader_code_ptr,
            .codeSize = shaderInfo.bytes,
    };

    VkShaderModule shaderModule;
    result = vkCreateShaderModule(device, &shader_module_create_info, NULL, &shaderModule);
    CHECK_RESULT(result);

    VkComputePipelineCreateInfo pipelineCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage.stage = VK_SHADER_STAGE_COMPUTE_BIT,
            .stage.module = shaderModule,
            .stage.pName = "main",
            .layout = pipelineLayout
    };
    VkPipeline pipeline;
    result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1,
                                     &pipelineCreateInfo, NULL, &pipeline);
    CHECK_RESULT(result);
    vkDestroyShaderModule(device, shaderModule, NULL);

    const uint32_t elements = 1024;
    VkBufferCreateInfo bufferCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = sizeof(uint32_t) * elements,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = (const uint32_t*) &queue_compute_index
    };

    VkBuffer bufferA;
    result = vkCreateBuffer(device, &bufferCreateInfo, NULL, &bufferA);
    CHECK_RESULT(result);

    VkBuffer bufferB;
    result = vkCreateBuffer(device, &bufferCreateInfo, NULL, &bufferB);
    CHECK_RESULT(result);

    VkBuffer bufferC;
    result = vkCreateBuffer(device, &bufferCreateInfo, NULL, &bufferC);
    CHECK_RESULT(result);

    VkDeviceSize requiredMemorySize = 0;
    VkMemoryRequirements bufferAMemoryRequirements;
    vkGetBufferMemoryRequirements(device, bufferA, &bufferAMemoryRequirements);
    requiredMemorySize += bufferAMemoryRequirements.size;
    VkMemoryRequirements bufferBMemoryRequirements;
    vkGetBufferMemoryRequirements(device, bufferA, &bufferBMemoryRequirements);
    requiredMemorySize += bufferBMemoryRequirements.size;
    VkMemoryRequirements bufferResultMemoryRequirements;
    vkGetBufferMemoryRequirements(device, bufferA,
                                  &bufferResultMemoryRequirements);
    requiredMemorySize += bufferResultMemoryRequirements.size;

    int memoryTypeIndex = findProperties(&memoryProperties, bufferAMemoryRequirements.memoryTypeBits,
                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
                   VK_MEMORY_PROPERTY_HOST_CACHED_BIT);

    VkMemoryAllocateInfo allocateInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize = requiredMemorySize,
            .memoryTypeIndex = memoryTypeIndex,
    };
    VkDeviceMemory memory;
    result = vkAllocateMemory(device, &allocateInfo, NULL, &memory);
    CHECK_RESULT(result);

    VkDeviceSize memoryOffset = 0;
    CHECK_RESULT(vkBindBufferMemory(device, bufferA, memory, memoryOffset));

    memoryOffset += bufferAMemoryRequirements.size;
    CHECK_RESULT(vkBindBufferMemory(device, bufferB, memory, memoryOffset));

    memoryOffset += bufferBMemoryRequirements.size;
    CHECK_RESULT(vkBindBufferMemory(device, bufferC, memory, memoryOffset));

    VkDescriptorPoolSize poolSize = {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = layoutBindingsCount
    };
    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 1,
            .poolSizeCount = 1,
            .pPoolSizes = &poolSize

    };
    VkDescriptorPool descriptorPool;
    vkCreateDescriptorPool(device, &descriptorPoolCreateInfo, NULL,&descriptorPool);

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayout,
    };

    VkDescriptorSet descriptorSet;
    result = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo,&descriptorSet);
    CHECK_RESULT(result);

    VkWriteDescriptorSet* descriptorSetWrites = malloc(sizeof(VkWriteDescriptorSet) * 3);
    VkWriteDescriptorSet writeDescriptorSet = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
    };
    VkDescriptorBufferInfo bufferAInfo = {
            .buffer = bufferA,
            .offset = 0,
            .range = VK_WHOLE_SIZE
    };
    writeDescriptorSet.pBufferInfo = &bufferAInfo;
    descriptorSetWrites[0] = writeDescriptorSet;


    VkDescriptorBufferInfo bufferBInfo = {
            .buffer = bufferB,
            .offset = 0,
            .range = VK_WHOLE_SIZE
    };
    writeDescriptorSet.dstBinding = 1;
    writeDescriptorSet.pBufferInfo = &bufferBInfo;
    descriptorSetWrites[1] = writeDescriptorSet;


    VkDescriptorBufferInfo bufferCInfo = {
            .buffer = bufferC,
            .offset = 0,
            .range = VK_WHOLE_SIZE
    };

    writeDescriptorSet.dstBinding = 2;
    writeDescriptorSet.pBufferInfo = &bufferCInfo;
    descriptorSetWrites[2] = writeDescriptorSet;

    vkUpdateDescriptorSets(device, 3,descriptorSetWrites, 0, NULL);

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = queue_compute_index

    };
    VkCommandPool commandPool;
    result = vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool);
    CHECK_RESULT(result);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
            .sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = commandPool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
    };
    VkCommandBuffer commandBuffer;
    CHECK_RESULT(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,&commandBuffer));

    VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
    };
    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, &descriptorSet, 0, NULL);

    vkCmdDispatch(commandBuffer, elements, 1, 1);

    CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

    int32_t *data = NULL;
    CHECK_RESULT(vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, (void**) &data));

    int32_t *aData = data;
    int32_t *bData = data + elements;
    int32_t *resultData = bData + elements;

    for (uint32_t i = 0; i < elements; i++) {
        aData[i] = 2;
        bData[i] = 1;
        resultData[i] = 42;
    }
    vkUnmapMemory(device, memory);

    VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers = &commandBuffer
    };
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    data = NULL;
    CHECK_RESULT(vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0,(void**)&data));

    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 32; ++j) {
            printf("%d ", resultData[i]);
        }
        printf("\n");
    }

    vkUnmapMemory(device, memory);

    free(shaderInfo.shader_code_ptr);
    vkDestroyPipeline(device, pipeline, NULL);
    DestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);

    return EXIT_SUCCESS;
}
