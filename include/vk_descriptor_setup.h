#ifndef MATRIX_DOTNET_CORE_VK_DESCRIPTOR_SETUP_H
#define MATRIX_DOTNET_CORE_VK_DESCRIPTOR_SETUP_H

#include "vulkan/vulkan.h"

VkDescriptorSetLayoutBinding vkCreateDescriptorSetLayoutBindingCompute(int binding) {
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    return layoutBinding;
}

#endif //MATRIX_DOTNET_CORE_VK_DESCRIPTOR_SETUP_H
