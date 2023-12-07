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

#ifndef SRC_DAWN_NATIVE_D3D11_COMMANDRECORDINGCONTEXT_D3D11_H_
#define SRC_DAWN_NATIVE_D3D11_COMMANDRECORDINGCONTEXT_D3D11_H_

#include "dawn/common/NonCopyable.h"
#include "dawn/common/Ref.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {
class CommandAllocatorManager;
class Buffer;
class Device;

class CommandRecordingContext {
  public:
    MaybeError Initialize(Device* device);
    void Release();
    MaybeError ExecuteCommandList();

    bool IsOpen() const { return mIsOpen; }
    bool NeedsSubmit() const { return mNeedsSubmit; }
    void SetNeedsSubmit() { mNeedsSubmit = true; }

  private:
    friend class ScopedCommandRecordingContext;
    friend class ScopedSwapStateCommandRecordingContext;

    bool mScopedAccessed = false;
    bool mIsOpen = false;
    bool mNeedsSubmit = false;
    ComPtr<ID3D11Device> mD3D11Device;
    ComPtr<ID3DDeviceContextState> mD3D11DeviceContextState;
    ComPtr<ID3D11DeviceContext4> mD3D11DeviceContext4;
    ComPtr<ID3D11Multithread> mD3D11Multithread;
    ComPtr<ID3DUserDefinedAnnotation> mD3DUserDefinedAnnotation;

    // The maximum number of builtin elements is 4 (vec4). It must be multiple of 4.
    static constexpr size_t kMaxNumBuiltinElements = 4;
    // The uniform buffer for built-in variables.
    Ref<Buffer> mUniformBuffer;
    std::array<uint32_t, kMaxNumBuiltinElements> mUniformBufferData;
    bool mUniformBufferDirty = true;

    Ref<Device> mDevice;
};

// For using ID3D11DeviceContext methods which don't change device context state.
class ScopedCommandRecordingContext : NonMovable {
  public:
    explicit ScopedCommandRecordingContext(CommandRecordingContext* commandContext);
    ~ScopedCommandRecordingContext();

    Device* GetDevice() const;

    // Wrapper method which don't depend on context state.
    void UpdateSubresource(ID3D11Resource* pDstResource,
                           UINT DstSubresource,
                           const D3D11_BOX* pDstBox,
                           const void* pSrcData,
                           UINT SrcRowPitch,
                           UINT SrcDepthPitch) const;
    void CopyResource(ID3D11Resource* pDstResource, ID3D11Resource* pSrcResource) const;
    void CopySubresourceRegion(ID3D11Resource* pDstResource,
                               UINT DstSubresource,
                               UINT DstX,
                               UINT DstY,
                               UINT DstZ,
                               ID3D11Resource* pSrcResource,
                               UINT SrcSubresource,
                               const D3D11_BOX* pSrcBox) const;
    void ClearRenderTargetView(ID3D11RenderTargetView* pRenderTargetView,
                               const FLOAT ColorRGBA[4]) const;
    void ClearDepthStencilView(ID3D11DepthStencilView* pDepthStencilView,
                               UINT ClearFlags,
                               FLOAT Depth,
                               UINT8 Stencil) const;
    HRESULT Map(ID3D11Resource* pResource,
                UINT Subresource,
                D3D11_MAP MapType,
                UINT MapFlags,
                D3D11_MAPPED_SUBRESOURCE* pMappedResource) const;
    void Unmap(ID3D11Resource* pResource, UINT Subresource) const;
    HRESULT Signal(ID3D11Fence* pFence, UINT64 Value) const;
    HRESULT Wait(ID3D11Fence* pFence, UINT64 Value) const;

    // Write the built-in variable value to the uniform buffer.
    void WriteUniformBuffer(uint32_t offset, uint32_t element) const;
    MaybeError FlushUniformBuffer() const;

  protected:
    CommandRecordingContext* const mCommandContext;
    ComPtr<ID3D11Multithread> mD3D11Multithread;
};

// For using ID3D11DeviceContext directly. It swaps and resets ID3DDeviceContextState of
// ID3D11DeviceContext for a scope. It is needed for sharing ID3D11Device between dawn and ANGLE.
class ScopedSwapStateCommandRecordingContext : public ScopedCommandRecordingContext {
  public:
    explicit ScopedSwapStateCommandRecordingContext(CommandRecordingContext* commandContext);
    ~ScopedSwapStateCommandRecordingContext();

    ID3D11Device* GetD3D11Device() const;
    ID3D11DeviceContext4* GetD3D11DeviceContext4() const;
    ID3DUserDefinedAnnotation* GetD3DUserDefinedAnnotation() const;
    Buffer* GetUniformBuffer() const;

  private:
    const bool mSwapContextState;
    ComPtr<ID3DDeviceContextState> mPreviousState;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_COMMANDRECORDINGCONTEXT_D3D11_H_
