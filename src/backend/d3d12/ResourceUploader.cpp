// Copyright 2017 The NXT Authors
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

#include "ResourceUploader.h"

#include "D3D12Backend.h"

namespace backend {
namespace d3d12 {

    ResourceUploader::ResourceUploader(Device* device) : device(device) {
    }

    void ResourceUploader::UploadToBuffer(ComPtr<ID3D12Resource> resource, uint32_t start, uint32_t count, const uint8_t* data) {
        D3D12_RESOURCE_DESC resourceDescriptor;
        resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDescriptor.Alignment = 0;
        resourceDescriptor.Width = count;
        resourceDescriptor.Height = 1;
        resourceDescriptor.DepthOrArraySize = 1;
        resourceDescriptor.MipLevels = 1;
        resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
        resourceDescriptor.SampleDesc.Count = 1;
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

        ComPtr<ID3D12Resource> uploadResource;

        D3D12_HEAP_PROPERTIES heapProperties;
        heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 0;
        heapProperties.VisibleNodeMask = 0;

        // TODO(enga@google.com): Use a ResourceAllocationManager
        ASSERT_SUCCESS(device->GetD3D12Device()->CreateCommittedResource(
            &heapProperties,
            D3D12_HEAP_FLAG_NONE,
            &resourceDescriptor,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadResource)
        ));

        D3D12_RANGE readRange;
        readRange.Begin = 0;
        readRange.End = 0;

        D3D12_RANGE writeRange;
        writeRange.Begin = 0;
        writeRange.End = count;

        uint8_t* mappedResource = nullptr;

        ASSERT_SUCCESS(uploadResource->Map(0, &readRange, reinterpret_cast<void**>(&mappedResource)));
        memcpy(mappedResource, data, count);
        uploadResource->Unmap(0, &writeRange);
        device->GetPendingCommandList()->CopyBufferRegion(resource.Get(), start, uploadResource.Get(), 0, count);

        pendingResources.push_back(uploadResource);
    }

    void ResourceUploader::EnqueueUploadingResources(const uint64_t serial) {
        if (pendingResources.size() > 0) {
            uploadingResources.push_back(std::make_pair(serial, std::move(pendingResources)));
            pendingResources.clear();
        }
    }

    void ResourceUploader::FreeCompletedResources(const uint64_t lastCompletedSerial) {
        auto it = uploadingResources.begin();
        while (it != uploadingResources.end()) {
            if (it->first < lastCompletedSerial) {
                it++;
            } else {
                break;
            }
        }
        uploadingResources.erase(uploadingResources.begin(), it);
    }

}
}
