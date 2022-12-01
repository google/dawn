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

#include <string>

#include "dawn/common/GPUInfo_autogen.h"
#include "dawn/common/StackContainer.h"

namespace gpu_info {

// Four uint16 fields could cover almost all driver version schemas:
// D3D12: AA.BB.CCC.DDDD
// Vulkan: AAA.BBB.CCC.DDD on Nvidia, CCC.DDDD for Intel Windows, and AA.BB.CCC for others,
// See https://vulkan.gpuinfo.org/
static constexpr uint32_t kMaxVersionFields = 4;

class DriverVersion {
  public:
    DriverVersion();
    DriverVersion(const std::initializer_list<uint16_t>& version);

    uint16_t& operator[](size_t i);
    const uint16_t& operator[](size_t i) const;

    uint32_t size() const;
    std::string ToString() const;

  private:
    StackVector<uint16_t, kMaxVersionFields> mDriverVersion;
};

// Do comparison between two driver versions. Currently we only support the comparison between
// Intel Windows driver versions.
// - Return -1 if build number of version1 is smaller
// - Return 1 if build number of version1 is bigger
// - Return 0 if version1 and version2 represent same driver version
int CompareWindowsDriverVersion(PCIVendorID vendorId,
                                const DriverVersion& version1,
                                const DriverVersion& version2);

// Intel architectures
bool IsSkylake(PCIDeviceID deviceId);

}  // namespace gpu_info
#endif  // SRC_DAWN_COMMON_GPUINFO_H_
