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

#ifndef SRC_DAWN_NATIVE_D3D_SHARED_FENCE_D3D_H_
#define SRC_DAWN_NATIVE_D3D_SHARED_FENCE_D3D_H_

#include "dawn/native/SharedFence.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

class Device;

class SharedFence : public SharedFenceBase {
  public:
    ~SharedFence() override;

  protected:
    SharedFence(Device* device, const char* label, HANDLE ownedHandle);

  private:
    MaybeError ExportInfoImpl(SharedFenceExportInfo* info) const override;

    HANDLE mHandle = nullptr;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_SHARED_FENCE_D3D_H_
