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

#include "dawn_native/d3d12/BufferD3D12.h"

#include "common/Assert.h"
#include "common/Constants.h"
#include "common/Math.h"
#include "dawn_native/d3d12/DeviceD3D12.h"

namespace dawn_native { namespace d3d12 {

    namespace {
        D3D12_RESOURCE_FLAGS D3D12ResourceFlags(dawn::BufferUsage usage) {
            D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

            if (usage & dawn::BufferUsage::Storage) {
                flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            }

            return flags;
        }

        D3D12_RESOURCE_STATES D3D12BufferUsage(dawn::BufferUsage usage) {
            D3D12_RESOURCE_STATES resourceState = D3D12_RESOURCE_STATE_COMMON;

            if (usage & dawn::BufferUsage::CopySrc) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_SOURCE;
            }
            if (usage & dawn::BufferUsage::CopyDst) {
                resourceState |= D3D12_RESOURCE_STATE_COPY_DEST;
            }
            if (usage & (dawn::BufferUsage::Vertex | dawn::BufferUsage::Uniform)) {
                resourceState |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            }
            if (usage & dawn::BufferUsage::Index) {
                resourceState |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
            }
            if (usage & dawn::BufferUsage::Storage) {
                resourceState |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
            }
            if (usage & dawn::BufferUsage::Indirect) {
                resourceState |= D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
            }

            return resourceState;
        }

        D3D12_HEAP_TYPE D3D12HeapType(dawn::BufferUsage allowedUsage) {
            if (allowedUsage & dawn::BufferUsage::MapRead) {
                return D3D12_HEAP_TYPE_READBACK;
            } else if (allowedUsage & dawn::BufferUsage::MapWrite) {
                return D3D12_HEAP_TYPE_UPLOAD;
            } else {
                return D3D12_HEAP_TYPE_DEFAULT;
            }
        }
    }  // namespace

    Buffer::Buffer(Device* device, const BufferDescriptor* descriptor)
        : BufferBase(device, descriptor) {
    }

    MaybeError Buffer::Initialize() {
        D3D12_RESOURCE_DESC resourceDescriptor;
        resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resourceDescriptor.Alignment = 0;
        resourceDescriptor.Width = GetD3D12Size();
        resourceDescriptor.Height = 1;
        resourceDescriptor.DepthOrArraySize = 1;
        resourceDescriptor.MipLevels = 1;
        resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
        resourceDescriptor.SampleDesc.Count = 1;
        resourceDescriptor.SampleDesc.Quality = 0;
        resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        // Add CopyDst for non-mappable buffer initialization in CreateBufferMapped
        // and robust resource initialization.
        resourceDescriptor.Flags = D3D12ResourceFlags(GetUsage() | dawn::BufferUsage::CopyDst);

        auto heapType = D3D12HeapType(GetUsage());
        auto bufferUsage = D3D12_RESOURCE_STATE_COMMON;

        // D3D12 requires buffers on the READBACK heap to have the D3D12_RESOURCE_STATE_COPY_DEST
        // state
        if (heapType == D3D12_HEAP_TYPE_READBACK) {
            bufferUsage |= D3D12_RESOURCE_STATE_COPY_DEST;
            mFixedResourceState = true;
            mLastUsage = dawn::BufferUsage::CopyDst;
        }

        // D3D12 requires buffers on the UPLOAD heap to have the D3D12_RESOURCE_STATE_GENERIC_READ
        // state
        if (heapType == D3D12_HEAP_TYPE_UPLOAD) {
            bufferUsage |= D3D12_RESOURCE_STATE_GENERIC_READ;
            mFixedResourceState = true;
            mLastUsage = dawn::BufferUsage::CopySrc;
        }

        DAWN_TRY_ASSIGN(
            mResourceAllocation,
            ToBackend(GetDevice())
                ->AllocateMemory(heapType, resourceDescriptor, bufferUsage, D3D12_HEAP_FLAG_NONE));
        return {};
    }

    Buffer::~Buffer() {
        DestroyInternal();
    }

    uint32_t Buffer::GetD3D12Size() const {
        // TODO(enga@google.com): TODO investigate if this needs to be a constraint at the API level
        return Align(GetSize(), 256);
    }

    ComPtr<ID3D12Resource> Buffer::GetD3D12Resource() const {
        return mResourceAllocation.GetD3D12Resource();
    }

    // When true is returned, a D3D12_RESOURCE_BARRIER has been created and must be used in a
    // ResourceBarrier call. Failing to do so will cause the tracked state to become invalid and can
    // cause subsequent errors.
    bool Buffer::TransitionUsageAndGetResourceBarrier(D3D12_RESOURCE_BARRIER* barrier,
                                                      dawn::BufferUsage newUsage) {
        // Resources in upload and readback heaps must be kept in the COPY_SOURCE/DEST state
        if (mFixedResourceState) {
            ASSERT(mLastUsage == newUsage);
            return false;
        }

        // We can skip transitions to already current usages.
        // TODO(cwallez@chromium.org): Need some form of UAV barriers at some point.
        if ((mLastUsage & newUsage) == newUsage) {
            return false;
        }

        D3D12_RESOURCE_STATES lastState = D3D12BufferUsage(mLastUsage);
        D3D12_RESOURCE_STATES newState = D3D12BufferUsage(newUsage);
        mLastUsage = newUsage;

        // The COMMON state represents a state where no write operations can be pending, which makes
        // it possible to transition to and from some states without synchronizaton (i.e. without an
        // explicit ResourceBarrier call). A buffer can be implicitly promoted to 1) a single write
        // state, or 2) multiple read states. A buffer that is accessed within a command list will
        // always implicitly decay to the COMMON state after the call to ExecuteCommandLists
        // completes - this is because all buffer writes are guaranteed to be completed before the
        // next ExecuteCommandLists call executes.
        // https://docs.microsoft.com/en-us/windows/desktop/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#implicit-state-transitions

        // To track implicit decays, we must record the pending serial on which a transition will
        // occur. When that buffer is used again, the previously recorded serial must be compared to
        // the last completed serial to determine if the buffer has implicity decayed to the common
        // state.
        const Serial pendingCommandSerial = ToBackend(GetDevice())->GetPendingCommandSerial();
        if (pendingCommandSerial > mLastUsedSerial) {
            lastState = D3D12_RESOURCE_STATE_COMMON;
            mLastUsedSerial = pendingCommandSerial;
        }

        // All possible buffer states used by Dawn are eligible for implicit promotion from COMMON.
        // These are: COPY_SOURCE, VERTEX_AND_COPY_BUFFER, INDEX_BUFFER, COPY_DEST,
        // UNORDERED_ACCESS, and INDIRECT_ARGUMENT. Note that for implicit promotion, the
        // destination state cannot be 1) more than one write state, or 2) both a read and write
        // state. This goes unchecked here because it should not be allowed through render/compute
        // pass validation.
        if (lastState == D3D12_RESOURCE_STATE_COMMON) {
            return false;
        }

        barrier->Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier->Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier->Transition.pResource = GetD3D12Resource().Get();
        barrier->Transition.StateBefore = lastState;
        barrier->Transition.StateAfter = newState;
        barrier->Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

        return true;
    }

    void Buffer::TransitionUsageNow(ComPtr<ID3D12GraphicsCommandList> commandList,
                                    dawn::BufferUsage usage) {
        D3D12_RESOURCE_BARRIER barrier;

        if (TransitionUsageAndGetResourceBarrier(&barrier, usage)) {
            commandList->ResourceBarrier(1, &barrier);
        }
    }

    D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetVA() const {
        return mResourceAllocation.GetGPUPointer();
    }

    void Buffer::OnMapCommandSerialFinished(uint32_t mapSerial, void* data, bool isWrite) {
        if (isWrite) {
            CallMapWriteCallback(mapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data, GetSize());
        } else {
            CallMapReadCallback(mapSerial, DAWN_BUFFER_MAP_ASYNC_STATUS_SUCCESS, data, GetSize());
        }
    }

    bool Buffer::IsMapWritable() const {
        // TODO(enga): Handle CPU-visible memory on UMA
        return (GetUsage() & (dawn::BufferUsage::MapRead | dawn::BufferUsage::MapWrite)) != 0;
    }

    MaybeError Buffer::MapAtCreationImpl(uint8_t** mappedPointer) {
        mWrittenMappedRange = {0, GetSize()};
        ASSERT_SUCCESS(GetD3D12Resource()->Map(0, &mWrittenMappedRange,
                                               reinterpret_cast<void**>(mappedPointer)));
        return {};
    }

    MaybeError Buffer::MapReadAsyncImpl(uint32_t serial) {
        mWrittenMappedRange = {};
        D3D12_RANGE readRange = {0, GetSize()};
        char* data = nullptr;
        ASSERT_SUCCESS(GetD3D12Resource()->Map(0, &readRange, reinterpret_cast<void**>(&data)));
        // There is no need to transition the resource to a new state: D3D12 seems to make the GPU
        // writes available when the fence is passed.
        MapRequestTracker* tracker = ToBackend(GetDevice())->GetMapRequestTracker();
        tracker->Track(this, serial, data, false);
        return {};
    }

    MaybeError Buffer::MapWriteAsyncImpl(uint32_t serial) {
        mWrittenMappedRange = {0, GetSize()};
        char* data = nullptr;
        ASSERT_SUCCESS(
            GetD3D12Resource()->Map(0, &mWrittenMappedRange, reinterpret_cast<void**>(&data)));
        // There is no need to transition the resource to a new state: D3D12 seems to make the CPU
        // writes available on queue submission.
        MapRequestTracker* tracker = ToBackend(GetDevice())->GetMapRequestTracker();
        tracker->Track(this, serial, data, true);
        return {};
    }

    void Buffer::UnmapImpl() {
        GetD3D12Resource()->Unmap(0, &mWrittenMappedRange);
        mWrittenMappedRange = {};
    }

    void Buffer::DestroyImpl() {
        ToBackend(GetDevice())->DeallocateMemory(mResourceAllocation);
    }

    MapRequestTracker::MapRequestTracker(Device* device) : mDevice(device) {
    }

    MapRequestTracker::~MapRequestTracker() {
        ASSERT(mInflightRequests.Empty());
    }

    void MapRequestTracker::Track(Buffer* buffer, uint32_t mapSerial, void* data, bool isWrite) {
        Request request;
        request.buffer = buffer;
        request.mapSerial = mapSerial;
        request.data = data;
        request.isWrite = isWrite;

        mInflightRequests.Enqueue(std::move(request), mDevice->GetPendingCommandSerial());
    }

    void MapRequestTracker::Tick(Serial finishedSerial) {
        for (auto& request : mInflightRequests.IterateUpTo(finishedSerial)) {
            request.buffer->OnMapCommandSerialFinished(request.mapSerial, request.data,
                                                       request.isWrite);
        }
        mInflightRequests.ClearUpTo(finishedSerial);
    }

}}  // namespace dawn_native::d3d12
