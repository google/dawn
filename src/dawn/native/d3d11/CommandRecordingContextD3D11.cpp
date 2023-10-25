// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"

#include <string>
#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/Forward.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {

MaybeError CommandRecordingContext::Intialize(Device* device) {
    DAWN_ASSERT(!IsOpen());
    DAWN_ASSERT(device);
    mDevice = device;
    mNeedsSubmit = false;

    ID3D11Device* d3d11Device = device->GetD3D11Device();

    ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
    device->GetD3D11Device()->GetImmediateContext(&d3d11DeviceContext);

    ComPtr<ID3D11DeviceContext4> d3d11DeviceContext4;
    DAWN_TRY(CheckHRESULT(d3d11DeviceContext.As(&d3d11DeviceContext4),
                          "D3D11 querying immediate context for ID3D11DeviceContext4 interface"));

    DAWN_TRY(
        CheckHRESULT(d3d11DeviceContext4.As(&mD3DUserDefinedAnnotation),
                     "D3D11 querying immediate context for ID3DUserDefinedAnnotation interface"));

    if (device->HasFeature(Feature::D3D11MultithreadProtected)) {
        DAWN_TRY(CheckHRESULT(d3d11DeviceContext.As(&mD3D11Multithread),
                              "D3D11 querying immediate context for ID3D11Multithread interface"));
        mD3D11Multithread->SetMultithreadProtected(TRUE);
    }

    mD3D11Device = d3d11Device;
    mD3D11DeviceContext4 = std::move(d3d11DeviceContext4);
    mIsOpen = true;

    // Create a uniform buffer for built in variables.
    BufferDescriptor descriptor;
    descriptor.size = sizeof(uint32_t) * kMaxNumBuiltinElements;
    descriptor.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
    descriptor.mappedAtCreation = false;
    descriptor.label = "builtin uniform buffer";

    Ref<BufferBase> uniformBuffer;
    {
        // Lock the device to protect the clearing of the built-in uniform buffer.
        auto deviceLock(device->GetScopedLock());
        DAWN_TRY_ASSIGN(uniformBuffer, device->CreateBuffer(&descriptor));
    }
    mUniformBuffer = ToBackend(std::move(uniformBuffer));

    // Always bind the uniform buffer to the reserved slot for all pipelines.
    // This buffer will be updated with the correct values before each draw or dispatch call.
    ID3D11Buffer* bufferPtr = mUniformBuffer->GetD3D11ConstantBuffer();
    mD3D11DeviceContext4->VSSetConstantBuffers(PipelineLayout::kReservedConstantBufferSlot, 1,
                                               &bufferPtr);
    mD3D11DeviceContext4->CSSetConstantBuffers(PipelineLayout::kReservedConstantBufferSlot, 1,
                                               &bufferPtr);

    return {};
}

MaybeError CommandRecordingContext::ExecuteCommandList(Device* device) {
    // Consider using deferred DeviceContext.
    mNeedsSubmit = false;
    return {};
}

ID3D11Device* CommandRecordingContext::GetD3D11Device() const {
    return mD3D11Device.Get();
}

ID3D11DeviceContext4* CommandRecordingContext::GetD3D11DeviceContext4() const {
    DAWN_ASSERT(mDevice->IsLockedByCurrentThreadIfNeeded());
    return mD3D11DeviceContext4.Get();
}

ID3DUserDefinedAnnotation* CommandRecordingContext::GetD3DUserDefinedAnnotation() const {
    return mD3DUserDefinedAnnotation.Get();
}

Buffer* CommandRecordingContext::GetUniformBuffer() const {
    return mUniformBuffer.Get();
}

Device* CommandRecordingContext::GetDevice() const {
    DAWN_ASSERT(mDevice.Get());
    return mDevice.Get();
}

void CommandRecordingContext::Release() {
    if (mIsOpen) {
        DAWN_ASSERT(mDevice->IsLockedByCurrentThreadIfNeeded());
        mIsOpen = false;
        mNeedsSubmit = false;
        mUniformBuffer = nullptr;
        mDevice = nullptr;
        ID3D11Buffer* nullBuffer = nullptr;
        mD3D11DeviceContext4->VSSetConstantBuffers(PipelineLayout::kReservedConstantBufferSlot, 1,
                                                   &nullBuffer);
        mD3D11DeviceContext4->CSSetConstantBuffers(PipelineLayout::kReservedConstantBufferSlot, 1,
                                                   &nullBuffer);
        mD3D11DeviceContext4 = nullptr;
        mD3D11Device = nullptr;
    }
}

bool CommandRecordingContext::IsOpen() const {
    return mIsOpen;
}

bool CommandRecordingContext::NeedsSubmit() const {
    return mNeedsSubmit;
}

void CommandRecordingContext::SetNeedsSubmit() {
    mNeedsSubmit = true;
}

CommandRecordingContext::ScopedCriticalSection::ScopedCriticalSection(
    ComPtr<ID3D11Multithread> d3d11Multithread)
    : mD3D11Multithread(std::move(d3d11Multithread)) {
    if (mD3D11Multithread) {
        mD3D11Multithread->Enter();
    }
}

CommandRecordingContext::ScopedCriticalSection::~ScopedCriticalSection() {
    if (mD3D11Multithread) {
        mD3D11Multithread->Leave();
    }
}

CommandRecordingContext::ScopedCriticalSection
CommandRecordingContext::EnterScopedCriticalSection() {
    return ScopedCriticalSection(mD3D11Multithread);
}

void CommandRecordingContext::WriteUniformBuffer(uint32_t offset, uint32_t element) {
    DAWN_ASSERT(offset < kMaxNumBuiltinElements);
    if (mUniformBufferData[offset] != element) {
        mUniformBufferData[offset] = element;
        mUniformBufferDirty = true;
    }
}

MaybeError CommandRecordingContext::FlushUniformBuffer() {
    if (mUniformBufferDirty) {
        DAWN_TRY(mUniformBuffer->Write(this, 0, mUniformBufferData.data(),
                                       mUniformBufferData.size() * sizeof(uint32_t)));
        mUniformBufferDirty = false;
    }
    return {};
}

}  // namespace dawn::native::d3d11
