// Copyright 2022 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn/native/vulkan/PipelineCacheVk.h"

#include <memory>

#include "dawn/native/Device.h"
#include "dawn/native/Error.h"
#include "dawn/native/vulkan/DeviceVk.h"
#include "dawn/native/vulkan/FencedDeleter.h"
#include "dawn/native/vulkan/VulkanError.h"

namespace dawn::native::vulkan {

// static
Ref<PipelineCache> PipelineCache::Create(DeviceBase* device, const CacheKey& key) {
    Ref<PipelineCache> cache = AcquireRef(new PipelineCache(device, key));
    cache->Initialize();
    return cache;
}

PipelineCache::PipelineCache(DeviceBase* device, const CacheKey& key)
    : PipelineCacheBase(device->GetBlobCache(), key), mDevice(device) {}

PipelineCache::~PipelineCache() {
    if (mHandle == VK_NULL_HANDLE) {
        return;
    }
    Device* device = ToBackend(GetDevice());
    device->fn.DestroyPipelineCache(device->GetVkDevice(), mHandle, nullptr);
    mHandle = VK_NULL_HANDLE;
}

DeviceBase* PipelineCache::GetDevice() const {
    return mDevice;
}

VkPipelineCache PipelineCache::GetHandle() const {
    return mHandle;
}

MaybeError PipelineCache::SerializeToBlobImpl(Blob* blob) {
    if (mHandle == VK_NULL_HANDLE) {
        // Pipeline cache isn't created successfully
        return {};
    }

    size_t bufferSize;
    Device* device = ToBackend(GetDevice());
    DAWN_TRY(CheckVkSuccess(
        device->fn.GetPipelineCacheData(device->GetVkDevice(), mHandle, &bufferSize, nullptr),
        "GetPipelineCacheData"));
    if (bufferSize == 0) {
        return {};
    }
    *blob = CreateBlob(bufferSize);
    DAWN_TRY(CheckVkSuccess(
        device->fn.GetPipelineCacheData(device->GetVkDevice(), mHandle, &bufferSize, blob->Data()),
        "GetPipelineCacheData"));
    return {};
}

void PipelineCache::Initialize() {
    Blob blob = PipelineCacheBase::Initialize();

    VkPipelineCacheCreateInfo createInfo;
    createInfo.flags = 0;
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.initialDataSize = blob.Size();
    createInfo.pInitialData = blob.Data();

    Device* device = ToBackend(GetDevice());
    mHandle = VK_NULL_HANDLE;

    // Attempts to create the pipeline cache but does not bubble the error, instead only logging.
    // This should be fine because the handle will be left as null and pipeline creation should
    // continue as if there was no cache.
    MaybeError maybeError = CheckVkSuccess(
        device->fn.CreatePipelineCache(device->GetVkDevice(), &createInfo, nullptr, &*mHandle),
        "CreatePipelineCache");
    if (maybeError.IsError()) {
        std::unique_ptr<ErrorData> error = maybeError.AcquireError();
        GetDevice()->EmitLog(WGPULoggingType_Info, error->GetFormattedMessage().c_str());
    }
}

}  // namespace dawn::native::vulkan
