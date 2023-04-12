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

#include "dawn/native/d3d11/UtilsD3D11.h"

#include "dawn/common/Assert.h"
#include "dawn/native/Format.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"

namespace dawn::native::d3d11 {

D3D11_COMPARISON_FUNC ToD3D11ComparisonFunc(wgpu::CompareFunction func) {
    switch (func) {
        case wgpu::CompareFunction::Never:
            return D3D11_COMPARISON_NEVER;
        case wgpu::CompareFunction::Less:
            return D3D11_COMPARISON_LESS;
        case wgpu::CompareFunction::LessEqual:
            return D3D11_COMPARISON_LESS_EQUAL;
        case wgpu::CompareFunction::Greater:
            return D3D11_COMPARISON_GREATER;
        case wgpu::CompareFunction::GreaterEqual:
            return D3D11_COMPARISON_GREATER_EQUAL;
        case wgpu::CompareFunction::Equal:
            return D3D11_COMPARISON_EQUAL;
        case wgpu::CompareFunction::NotEqual:
            return D3D11_COMPARISON_NOT_EQUAL;
        case wgpu::CompareFunction::Always:
            return D3D11_COMPARISON_ALWAYS;
        case wgpu::CompareFunction::Undefined:
            UNREACHABLE();
    }
}

void SetDebugName(Device* device,
                  ID3D11DeviceChild* object,
                  const char* prefix,
                  std::string label) {
    if (!object) {
        return;
    }

    if (label.empty() || !device->IsToggleEnabled(Toggle::UseUserDefinedLabelsInBackend)) {
        object->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(prefix), prefix);
        return;
    }

    std::string objectName = prefix;
    objectName += "_";
    objectName += label;
    object->SetPrivateData(WKPDID_D3DDebugObjectName, objectName.length(), objectName.c_str());
}

}  // namespace dawn::native::d3d11
