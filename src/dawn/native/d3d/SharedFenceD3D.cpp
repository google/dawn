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

#include "dawn/native/d3d/SharedFenceD3D.h"

#include "dawn/native/ChainUtils.h"
#include "dawn/native/d3d/DeviceD3D.h"

namespace dawn::native::d3d {

SharedFence::SharedFence(Device* device, const char* label, HANDLE ownedHandle)
    : SharedFenceBase(device, label), mHandle(ownedHandle) {}

SharedFence::~SharedFence() {
    if (mHandle) {
        ::CloseHandle(mHandle);
    }
}

MaybeError SharedFence::ExportInfoImpl(SharedFenceExportInfo* info) const {
    info->type = wgpu::SharedFenceType::DXGISharedHandle;

    DAWN_TRY(
        ValidateSingleSType(info->nextInChain, wgpu::SType::SharedFenceDXGISharedHandleExportInfo));

    SharedFenceDXGISharedHandleExportInfo* exportInfo = nullptr;
    FindInChain(info->nextInChain, &exportInfo);

    if (exportInfo != nullptr) {
        exportInfo->handle = mHandle;
    }
    return {};
}

}  // namespace dawn::native::d3d
