// Copyright 2023 The Dawn Authors
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
    MaybeError Intialize(Device* device);

    void Release();
    bool IsOpen() const;
    bool NeedsSubmit() const;
    void SetNeedsSubmit();

    MaybeError ExecuteCommandList(Device* device);

    ID3D11Device* GetD3D11Device() const;
    ID3D11DeviceContext* GetD3D11DeviceContext() const;
    ID3D11DeviceContext1* GetD3D11DeviceContext1() const;
    ID3D11DeviceContext4* GetD3D11DeviceContext4() const;
    ID3DUserDefinedAnnotation* GetD3DUserDefinedAnnotation() const;
    Buffer* GetUniformBuffer() const;
    Device* GetDevice() const;

    struct ScopedCriticalSection : NonMovable {
        explicit ScopedCriticalSection(ComPtr<ID3D11Multithread>);
        ~ScopedCriticalSection();

      private:
        ComPtr<ID3D11Multithread> mD3D11Multithread;
    };
    // Returns a scoped object that marks a critical section using the
    // ID3D11Multithread Enter and Leave methods. This allows minimizing the
    // cost of D3D11 multithread protection by allowing a single mutex Acquire
    // and Release call for an entire set of operations on the immediate context
    // e.g. when executing command buffers. This only has an effect if the
    // ImplicitDeviceSynchronization feature is enabled.
    ScopedCriticalSection EnterScopedCriticalSection();

    // Write the built-in variable value to the uniform buffer.
    void WriteUniformBuffer(uint32_t offset, uint32_t element);
    MaybeError FlushUniformBuffer();

  private:
    bool mIsOpen = false;
    bool mNeedsSubmit = false;
    ComPtr<ID3D11Device> mD3D11Device;
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

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_COMMANDRECORDINGCONTEXT_D3D11_H_
