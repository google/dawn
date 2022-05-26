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

#ifndef SRC_DAWN_COMMON_GPUINFO_H_
#define SRC_DAWN_COMMON_GPUINFO_H_

#include "dawn/common/GPUInfo_autogen.h"

#include <array>

namespace gpu_info {

using D3DDriverVersion = std::array<uint16_t, 4>;

// Do comparison between two driver versions. Currently we only support the comparison between
// Intel D3D driver versions.
// - Return -1 if build number of version1 is smaller
// - Return 1 if build number of version1 is bigger
// - Return 0 if version1 and version2 represent same driver version
int CompareD3DDriverVersion(PCIVendorID vendorId,
                            const D3DDriverVersion& version1,
                            const D3DDriverVersion& version2);

// Intel architectures
bool IsSkylake(PCIDeviceID deviceId);

}  // namespace gpu_info
#endif  // SRC_DAWN_COMMON_GPUINFO_H_
