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

#include "dawn/common/RefCounted.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {
class CommandAllocatorManager;
class Buffer;
class Device;

class CommandRecordingContext {
  public:
    MaybeError Open(Device* device);

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

  private:
    bool mIsOpen = false;
    bool mNeedsSubmit = false;
    ComPtr<ID3D11Device> mD3D11Device;
    ComPtr<ID3D11DeviceContext4> mD3D11DeviceContext4;
    ComPtr<ID3DUserDefinedAnnotation> mD3D11UserDefinedAnnotation;
    // The uniform buffer for built in variables.
    Ref<Buffer> mUniformBuffer;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_COMMANDRECORDINGCONTEXT_D3D11_H_
