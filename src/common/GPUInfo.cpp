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

#include "common/GPUInfo.h"

#include "common/Assert.h"

#include <algorithm>
#include <array>

namespace gpu_info {
    namespace {
        // Intel
        // Referenced from the following Mesa source code:
        // https://github.com/mesa3d/mesa/blob/master/include/pci_ids/i965_pci_ids.h
        // gen9
        const std::array<uint32_t, 25> Skylake = {
            {0x1902, 0x1906, 0x190A, 0x190B, 0x190E, 0x1912, 0x1913, 0x1915, 0x1916,
             0x1917, 0x191A, 0x191B, 0x191D, 0x191E, 0x1921, 0x1923, 0x1926, 0x1927,
             0x192A, 0x192B, 0x192D, 0x1932, 0x193A, 0x193B, 0x193D}};
        // gen9p5
        const std::array<uint32_t, 20> Kabylake = {
            {0x5916, 0x5913, 0x5906, 0x5926, 0x5921, 0x5915, 0x590E, 0x591E, 0x5912, 0x5917,
             0x5902, 0x591B, 0x593B, 0x590B, 0x591A, 0x590A, 0x591D, 0x5908, 0x5923, 0x5927}};
        const std::array<uint32_t, 17> Coffeelake = {
            {0x87CA, 0x3E90, 0x3E93, 0x3E99, 0x3E9C, 0x3E91, 0x3E92, 0x3E96, 0x3E98, 0x3E9A, 0x3E9B,
             0x3E94, 0x3EA9, 0x3EA5, 0x3EA6, 0x3EA7, 0x3EA8}};
        const std::array<uint32_t, 5> Whiskylake = {{0x3EA1, 0x3EA4, 0x3EA0, 0x3EA3, 0x3EA2}};
        const std::array<uint32_t, 21> Cometlake = {
            {0x9B21, 0x9BA0, 0x9BA2, 0x9BA4, 0x9BA5, 0x9BA8, 0x9BAA, 0x9BAB, 0x9BAC, 0x9B41, 0x9BC0,
             0x9BC2, 0x9BC4, 0x9BC5, 0x9BC6, 0x9BC8, 0x9BCA, 0x9BCB, 0x9BCC, 0x9BE6, 0x9BF6}};

        bool IsOldIntelD3DVersionScheme(const D3DDriverVersion& driverVersion) {
            // See https://www.intel.com/content/www/us/en/support/articles/000005654/graphics.html
            // for more details
            return driverVersion[2] < 100u;
        }

    }  // anonymous namespace

    bool IsAMD(PCIVendorID vendorId) {
        return vendorId == kVendorID_AMD;
    }
    bool IsARM(PCIVendorID vendorId) {
        return vendorId == kVendorID_ARM;
    }
    bool IsImgTec(PCIVendorID vendorId) {
        return vendorId == kVendorID_ImgTec;
    }
    bool IsIntel(PCIVendorID vendorId) {
        return vendorId == kVendorID_Intel;
    }
    bool IsNvidia(PCIVendorID vendorId) {
        return vendorId == kVendorID_Nvidia;
    }
    bool IsQualcomm(PCIVendorID vendorId) {
        return vendorId == kVendorID_Qualcomm;
    }
    bool IsSwiftshader(PCIVendorID vendorId, PCIDeviceID deviceId) {
        return vendorId == kVendorID_Google && deviceId == kDeviceID_Swiftshader;
    }
    bool IsWARP(PCIVendorID vendorId, PCIDeviceID deviceId) {
        return vendorId == kVendorID_Microsoft && deviceId == kDeviceID_WARP;
    }

    int CompareD3DDriverVersion(PCIVendorID vendorId,
                                const D3DDriverVersion& version1,
                                const D3DDriverVersion& version2) {
        if (IsIntel(vendorId)) {
            // The Intel graphics driver version schema has had a change since the Windows 10 April
            // 2018 Update release. In the new schema the 3rd number of the driver version is always
            // 100, while on the older drivers it is always less than 100. For the drivers using the
            // same driver version schema, the newer driver always has the bigger 4th number.
            // See https://www.intel.com/content/www/us/en/support/articles/000005654/graphics.html
            // for more details.
            bool isOldIntelDriver1 = IsOldIntelD3DVersionScheme(version1);
            bool isOldIntelDriver2 = IsOldIntelD3DVersionScheme(version2);
            if (isOldIntelDriver1 && !isOldIntelDriver2) {
                return -1;
            } else if (!isOldIntelDriver1 && isOldIntelDriver2) {
                return 1;
            } else {
                return static_cast<int32_t>(version1[3]) - static_cast<int32_t>(version2[3]);
            }
        }

        // TODO(jiawei.shao@intel.com): support other GPU vendors
        UNREACHABLE();
        return 0;
    }

    // Intel GPUs
    bool IsSkylake(PCIDeviceID deviceId) {
        return std::find(Skylake.cbegin(), Skylake.cend(), deviceId) != Skylake.cend();
    }
    bool IsKabylake(PCIDeviceID deviceId) {
        return std::find(Kabylake.cbegin(), Kabylake.cend(), deviceId) != Kabylake.cend();
    }
    bool IsCoffeelake(PCIDeviceID deviceId) {
        return (std::find(Coffeelake.cbegin(), Coffeelake.cend(), deviceId) != Coffeelake.cend()) ||
               (std::find(Whiskylake.cbegin(), Whiskylake.cend(), deviceId) != Whiskylake.cend()) ||
               (std::find(Cometlake.cbegin(), Cometlake.cend(), deviceId) != Cometlake.cend());
    }
}  // namespace gpu_info
