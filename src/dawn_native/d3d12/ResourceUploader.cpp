// Copyright 2017 The Dawn Authors
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

#include "dawn_native/d3d12/ResourceUploader.h"

#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/ResourceAllocator.h"

namespace dawn_native { namespace d3d12 {

    ResourceUploader::ResourceUploader(Device* device) : mDevice(device) {
    }

    void ResourceUploader::BufferSubData(ComPtr<ID3D12Resource> resource,
                                         uint32_t start,
                                         uint32_t count,
                                         const void* data) {
        // TODO(enga@google.com): Use a handle to a subset of a large ring buffer. On Release,
        // decrease reference count on the ring buffer and free when 0. Alternatively, the
        // SerialQueue could be used to track which last point of the ringbuffer is in use, and
        // start reusing chunks of it that aren't in flight.
        UploadHandle uploadHandle = GetUploadBuffer(count);
        memcpy(uploadHandle.mappedBuffer, data, count);
        mDevice->GetPendingCommandList()->CopyBufferRegion(resource.Get(), start,
                                                           uploadHandle.resource.Get(), 0, count);
        Release(uploadHandle);
    }

    ResourceUploader::UploadHandle ResourceUploader::GetUploadBuffer(uint32_t requiredSize) {
        // TODO(enga@google.com): This will find or create a mapped buffer of sufficient size and
        // return a handle to a mapped range
        D3D12_RESOURCE_DESC resourceDescriptor;
        resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDescriptor.Alignment = 0;
        resourceDescriptor.Width = requiredSize;
        resourceDescriptor.Height = 1;
        resourceDescriptor.DepthOrArraySize = 1;
        resourceDescriptor.MipLevels = 1;
        resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
        resourceDescriptor.SampleDesc.Count = 1;
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;

        UploadHandle uploadHandle;
        uploadHandle.resource = mDevice->GetResourceAllocator()->Allocate(
            D3D12_HEAP_TYPE_UPLOAD, resourceDescriptor, D3D12_RESOURCE_STATE_GENERIC_READ);
        D3D12_RANGE readRange;
        readRange.Begin = 0;
        readRange.End = 0;

        uploadHandle.resource->Map(0, &readRange,
                                   reinterpret_cast<void**>(&uploadHandle.mappedBuffer));
        return uploadHandle;
    }

    void ResourceUploader::Release(UploadHandle uploadHandle) {
        uploadHandle.resource->Unmap(0, nullptr);
        mDevice->GetResourceAllocator()->Release(uploadHandle.resource);
    }

}}  // namespace dawn_native::d3d12
