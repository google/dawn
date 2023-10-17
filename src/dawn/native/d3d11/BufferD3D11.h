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

#ifndef SRC_DAWN_NATIVE_D3D11_BUFFERD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_BUFFERD3D11_H_

#include <limits>
#include <memory>

#include "dawn/native/Buffer.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class CommandRecordingContext;
class Device;

class Buffer final : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> Create(Device* device, const BufferDescriptor* descriptor);

    MaybeError EnsureDataInitialized(CommandRecordingContext* commandContext);
    MaybeError EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                  uint64_t offset,
                                                  uint64_t size);
    MaybeError EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                  const CopyTextureToBufferCmd* copy);

    // Dawn API
    void SetLabelImpl() override;

    ID3D11Buffer* GetD3D11ConstantBuffer() const { return mD3d11ConstantBuffer.Get(); }
    ID3D11Buffer* GetD3D11NonConstantBuffer() const { return mD3d11NonConstantBuffer.Get(); }
    // Mark the mD3d11NonConstantBuffer is mutated by shaders, if mD3d11ConstantBuffer exists,
    // it will be synced with mD3d11NonConstantBuffer before binding it to the constant buffer slot.
    void MarkMutated();
    // Update content of the mD3d11ConstantBuffer from mD3d11NonConstantBuffer if needed.
    void EnsureConstantBufferIsUpdated(CommandRecordingContext* commandContext);
    ResultOrError<ComPtr<ID3D11ShaderResourceView>> CreateD3D11ShaderResourceView(
        uint64_t offset,
        uint64_t size) const;
    ResultOrError<ComPtr<ID3D11UnorderedAccessView1>> CreateD3D11UnorderedAccessView1(
        uint64_t offset,
        uint64_t size) const;
    MaybeError Clear(CommandRecordingContext* commandContext,
                     uint8_t clearValue,
                     uint64_t offset,
                     uint64_t size);
    MaybeError Write(CommandRecordingContext* commandContext,
                     uint64_t offset,
                     const void* data,
                     size_t size);

    static MaybeError Copy(CommandRecordingContext* commandContext,
                           Buffer* source,
                           uint64_t sourceOffset,
                           size_t size,
                           Buffer* destination,
                           uint64_t destinationOffset);

    class ScopedMap : public NonCopyable {
      public:
        // Map buffer and return a ScopedMap object. If the buffer is not mappable,
        // scopedMap.GetMappedData() will return nullptr.
        static ResultOrError<ScopedMap> Create(Buffer* buffer);

        ScopedMap();
        ~ScopedMap();

        ScopedMap(ScopedMap&& other);
        ScopedMap& operator=(ScopedMap&& other);

        uint8_t* GetMappedData() const;

        void Reset();

      private:
        ScopedMap(Buffer* buffer, bool needsUnmap);

        Buffer* mBuffer = nullptr;

        // Whether the buffer needs to be unmapped when the ScopedMap object is destroyed.
        bool mNeedsUnmap = false;
    };

  private:
    using BufferBase::BufferBase;

    ~Buffer() override;

    MaybeError Initialize(bool mappedAtCreation);
    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    void UnmapImpl() override;
    void DestroyImpl() override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointer() override;

    MaybeError MapInternal();
    void UnmapInternal();

    MaybeError InitializeToZero(CommandRecordingContext* commandContext);
    // Clear the buffer without checking if the buffer is initialized.
    MaybeError ClearInternal(CommandRecordingContext* commandContext,
                             uint8_t clearValue,
                             uint64_t offset = 0,
                             uint64_t size = 0);
    // Write the buffer without checking if the buffer is initialized.
    MaybeError WriteInternal(CommandRecordingContext* commandContext,
                             uint64_t bufferOffset,
                             const void* data,
                             size_t size);
    // Copy the buffer without checking if the buffer is initialized.
    static MaybeError CopyInternal(CommandRecordingContext* commandContext,
                                   Buffer* source,
                                   uint64_t sourceOffset,
                                   size_t size,
                                   Buffer* destination,
                                   uint64_t destinationOffset);
    // The buffer object for constant buffer usage.
    ComPtr<ID3D11Buffer> mD3d11ConstantBuffer;
    // The buffer object for non-constant buffer usages(e.g. storage buffer, vertex buffer, etc.)
    ComPtr<ID3D11Buffer> mD3d11NonConstantBuffer;
    bool mConstantBufferIsUpdated = true;
    uint8_t* mMappedData = nullptr;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_BUFFERD3D11_H_
