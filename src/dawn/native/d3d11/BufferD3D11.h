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
#include <utility>

#include "dawn/native/Buffer.h"
#include "dawn/native/d3d/d3d_platform.h"
#include "dawn/native/d3d11/Forward.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn::native::d3d11 {

class Device;
class ScopedCommandRecordingContext;
class ScopedSwapStateCommandRecordingContext;

class Buffer : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> Create(Device* device,
                                             const UnpackedPtr<BufferDescriptor>& descriptor,
                                             const ScopedCommandRecordingContext* commandContext,
                                             bool allowUploadBufferEmulation = true);

    MaybeError EnsureDataInitialized(const ScopedCommandRecordingContext* commandContext);
    MaybeError EnsureDataInitializedAsDestination(
        const ScopedCommandRecordingContext* commandContext,
        uint64_t offset,
        uint64_t size);
    MaybeError EnsureDataInitializedAsDestination(
        const ScopedCommandRecordingContext* commandContext,
        const CopyTextureToBufferCmd* copy);

    MaybeError Clear(const ScopedCommandRecordingContext* commandContext,
                     uint8_t clearValue,
                     uint64_t offset,
                     uint64_t size);
    MaybeError Write(const ScopedCommandRecordingContext* commandContext,
                     uint64_t offset,
                     const void* data,
                     size_t size);

    static MaybeError Copy(const ScopedCommandRecordingContext* commandContext,
                           Buffer* source,
                           uint64_t sourceOffset,
                           size_t size,
                           Buffer* destination,
                           uint64_t destinationOffset);

    // Actually map the buffer when its last usage serial has passed.
    MaybeError FinalizeMap(ScopedCommandRecordingContext* commandContext,
                           ExecutionSerial completedSerial,
                           wgpu::MapMode mode);

    bool IsCPUWritable() const;
    bool IsCPUReadable() const;

    // This performs GPU Clear. Unlike Clear(), this will always be affected by ID3D11Predicate.
    // Whereas Clear() might be unaffected by ID3D11Predicate if it's pure CPU clear.
    virtual MaybeError PredicatedClear(const ScopedSwapStateCommandRecordingContext* commandContext,
                                       ID3D11Predicate* predicate,
                                       uint8_t clearValue,
                                       uint64_t offset,
                                       uint64_t size);

    // Write the buffer without checking if the buffer is initialized.
    virtual MaybeError WriteInternal(const ScopedCommandRecordingContext* commandContext,
                                     uint64_t bufferOffset,
                                     const void* data,
                                     size_t size) = 0;
    // Copy this buffer to the destination without checking if the buffer is initialized.
    virtual MaybeError CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                                      uint64_t sourceOffset,
                                      size_t size,
                                      Buffer* destination,
                                      uint64_t destinationOffset) = 0;
    // Copy from a D3D buffer to this buffer without checking if the buffer is initialized.
    virtual MaybeError CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                           ID3D11Buffer* srcD3D11Buffer,
                                           uint64_t sourceOffset,
                                           size_t size,
                                           uint64_t destinationOffset) = 0;

    class ScopedMap : public NonCopyable {
      public:
        // Map buffer and return a ScopedMap object. If the buffer is not mappable,
        // scopedMap.GetMappedData() will return nullptr.
        static ResultOrError<ScopedMap> Create(const ScopedCommandRecordingContext* commandContext,
                                               Buffer* buffer,
                                               wgpu::MapMode mode);

        ScopedMap();
        ~ScopedMap();

        ScopedMap(ScopedMap&& other);
        ScopedMap& operator=(ScopedMap&& other);

        uint8_t* GetMappedData() const;

        void Reset();

      private:
        ScopedMap(const ScopedCommandRecordingContext* commandContext,
                  Buffer* buffer,
                  bool needsUnmap);

        raw_ptr<const ScopedCommandRecordingContext> mCommandContext = nullptr;
        raw_ptr<Buffer> mBuffer = nullptr;
        // Whether the buffer needs to be unmapped when the ScopedMap object is destroyed.
        bool mNeedsUnmap = false;
    };

  protected:
    Buffer(DeviceBase* device,
           const UnpackedPtr<BufferDescriptor>& descriptor,
           wgpu::BufferUsage internalMappableFlags);
    ~Buffer() override;

    void DestroyImpl() override;

    virtual MaybeError InitializeInternal() = 0;

    virtual MaybeError MapInternal(const ScopedCommandRecordingContext* commandContext,
                                   wgpu::MapMode mode);
    virtual void UnmapInternal(const ScopedCommandRecordingContext* commandContext);

    // Clear the buffer without checking if the buffer is initialized.
    MaybeError ClearWholeBuffer(const ScopedCommandRecordingContext* commandContext,
                                uint8_t clearValue);
    virtual MaybeError ClearInternal(const ScopedCommandRecordingContext* commandContext,
                                     uint8_t clearValue,
                                     uint64_t offset,
                                     uint64_t size);

    virtual MaybeError ClearPaddingInternal(const ScopedCommandRecordingContext* commandContext);

    raw_ptr<uint8_t, AllowPtrArithmetic> mMappedData = nullptr;

  private:
    MaybeError Initialize(bool mappedAtCreation,
                          const ScopedCommandRecordingContext* commandContext);
    MaybeError ClearInitialResource(const ScopedCommandRecordingContext* commandContext);
    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    void UnmapImpl() override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointer() override;

    MaybeError InitializeToZero(const ScopedCommandRecordingContext* commandContext);

    // Internal usage indicating the native buffer supports mapping for read and/or write or not.
    const wgpu::BufferUsage mInternalMappableFlags;
    ExecutionSerial mMapReadySerial = kMaxExecutionSerial;
};

// Buffer that can be used by GPU.
class GPUUsableBuffer : public Buffer {
  public:
    virtual ResultOrError<ID3D11Buffer*> GetD3D11ConstantBuffer(
        const ScopedCommandRecordingContext* commandContext) = 0;
    virtual ResultOrError<ID3D11Buffer*> GetD3D11NonConstantBuffer(
        const ScopedCommandRecordingContext* commandContext) = 0;
    ID3D11Buffer* GetD3D11ConstantBufferForTesting();
    ID3D11Buffer* GetD3D11NonConstantBufferForTesting();

    virtual ResultOrError<ComPtr<ID3D11ShaderResourceView>> UseAsSRV(
        const ScopedCommandRecordingContext* commandContext,
        uint64_t offset,
        uint64_t size) = 0;

    // Use this buffer as UAV and mark it as being mutated by shader.
    virtual ResultOrError<ComPtr<ID3D11UnorderedAccessView1>> UseAsUAV(
        const ScopedCommandRecordingContext* commandContext,
        uint64_t offset,
        uint64_t size) = 0;

  protected:
    using Buffer::Buffer;

    ResultOrError<ComPtr<ID3D11ShaderResourceView>> CreateD3D11ShaderResourceViewFromD3DBuffer(
        ID3D11Buffer* d3d11Buffer,
        uint64_t offset,
        uint64_t size);
    ResultOrError<ComPtr<ID3D11UnorderedAccessView1>> CreateD3D11UnorderedAccessViewFromD3DBuffer(
        ID3D11Buffer* d3d11Buffer,
        uint64_t offset,
        uint64_t size);

    MaybeError UpdateD3D11ConstantBuffer(const ScopedCommandRecordingContext* commandContext,
                                         ID3D11Buffer* d3d11Buffer,
                                         bool firstTimeUpdate,
                                         uint64_t bufferOffset,
                                         const void* data,
                                         size_t size);
};

static inline GPUUsableBuffer* ToGPUUsableBuffer(BufferBase* buffer) {
    return static_cast<GPUUsableBuffer*>(ToBackend(buffer));
}

static inline Ref<GPUUsableBuffer> ToGPUUsableBuffer(Ref<BufferBase>&& buffer) {
    return std::move(buffer).Cast<Ref<GPUUsableBuffer>>();
}

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_BUFFERD3D11_H_
