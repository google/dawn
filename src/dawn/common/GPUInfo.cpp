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

#include "dawn/common/GPUInfo.h"

#include <algorithm>

#include "dawn/common/Assert.h"

namespace gpu_info {
namespace {
// Intel
// Referenced from the following Mesa source code:
// https://github.com/mesa3d/mesa/blob/main/include/pci_ids/iris_pci_ids.h
// gen9
const std::array<uint32_t, 25> Skylake = {{0x1902, 0x1906, 0x190A, 0x190B, 0x190E, 0x1912, 0x1913,
                                           0x1915, 0x1916, 0x1917, 0x191A, 0x191B, 0x191D, 0x191E,
                                           0x1921, 0x1923, 0x1926, 0x1927, 0x192A, 0x192B, 0x192D,
                                           0x1932, 0x193A, 0x193B, 0x193D}};

// According to Intel graphics driver version schema, build number is generated from the
// last two fields.
// See https://www.intel.com/content/www/us/en/support/articles/000005654/graphics.html for
// more details.
uint32_t GetIntelD3DDriverBuildNumber(const D3DDriverVersion& driverVersion) {
    return driverVersion[2] * 10000 + driverVersion[3];
}

}  // anonymous namespace

int CompareD3DDriverVersion(PCIVendorID vendorId,
                            const D3DDriverVersion& version1,
                            const D3DDriverVersion& version2) {
    if (IsIntel(vendorId)) {
        uint32_t buildNumber1 = GetIntelD3DDriverBuildNumber(version1);
        uint32_t buildNumber2 = GetIntelD3DDriverBuildNumber(version2);
        return buildNumber1 < buildNumber2 ? -1 : (buildNumber1 == buildNumber2 ? 0 : 1);
    }

    // TODO(crbug.com/dawn/823): support other GPU vendors
    UNREACHABLE();
    return 0;
}

// Intel GPUs
bool IsSkylake(PCIDeviceID deviceId) {
    return std::find(Skylake.cbegin(), Skylake.cend(), deviceId) != Skylake.cend();
}

}  // namespace gpu_info
