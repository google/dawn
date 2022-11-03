// Copyright 2019 The Dawn Authors
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
#ifndef SRC_DAWN_NATIVE_D3D12_COMMANDRECORDINGCONTEXT_H_
#define SRC_DAWN_NATIVE_D3D12_COMMANDRECORDINGCONTEXT_H_

#include <set>
#include <vector>

#include "dawn/native/Error.h"
#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d12/BufferD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {
class CommandAllocatorManager;
class Device;
class Heap;
class Texture;

class CommandRecordingContext {
  public:
    void AddToSharedTextureList(Texture* texture);
    MaybeError Open(ID3D12Device* d3d12Device, CommandAllocatorManager* commandAllocationManager);

    ID3D12GraphicsCommandList* GetCommandList() const;
    ID3D12GraphicsCommandList4* GetCommandList4() const;
    void Release();
    bool IsOpen() const;
    bool NeedsSubmit() const;
    void SetNeedsSubmit();

    MaybeError ExecuteCommandList(Device* device);

    void TrackHeapUsage(Heap* heap, ExecutionSerial serial);

    void AddToTempBuffers(Ref<Buffer> tempBuffer);

  private:
    ComPtr<ID3D12GraphicsCommandList> mD3d12CommandList;
    ComPtr<ID3D12GraphicsCommandList4> mD3d12CommandList4;
    bool mIsOpen = false;
    bool mNeedsSubmit = false;
    std::set<Texture*> mSharedTextures;
    std::vector<Heap*> mHeapsPendingUsage;

    std::vector<Ref<Buffer>> mTempBuffers;
};
}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_COMMANDRECORDINGCONTEXT_H_
