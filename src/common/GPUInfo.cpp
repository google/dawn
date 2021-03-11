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

#include <algorithm>

namespace gpu_info {
    namespace {
        // Intel
        // Referenced from the following Mesa source code:
        // https://github.com/mesa3d/mesa/blob/master/include/pci_ids/i965_pci_ids.h
        // gen9
        const uint32_t Skylake[] = {0x1902, 0x1906, 0x190A, 0x190B, 0x190E, 0x1912, 0x1913,
                                    0x1915, 0x1916, 0x1917, 0x191A, 0x191B, 0x191D, 0x191E,
                                    0x1921, 0x1923, 0x1926, 0x1927, 0x192A, 0x192B, 0x192D,
                                    0x1932, 0x193A, 0x193B, 0x193D};
        // gen9p5
        const uint32_t Kabylake[] = {0x5916, 0x5913, 0x5906, 0x5926, 0x5921, 0x5915, 0x590E,
                                     0x591E, 0x5912, 0x5917, 0x5902, 0x591B, 0x593B, 0x590B,
                                     0x591A, 0x590A, 0x591D, 0x5908, 0x5923, 0x5927};
        const uint32_t Coffeelake[] = {0x87CA, 0x3E90, 0x3E93, 0x3E99, 0x3E9C, 0x3E91,
                                       0x3E92, 0x3E96, 0x3E98, 0x3E9A, 0x3E9B, 0x3E94,
                                       0x3EA9, 0x3EA5, 0x3EA6, 0x3EA7, 0x3EA8};
        const uint32_t WhiskyLake[] = {0x3EA1, 0x3EA4, 0x3EA0, 0x3EA3, 0x3EA2};
        const uint32_t CometLake[] = {0x9B21, 0x9BA0, 0x9BA2, 0x9BA4, 0x9BA5, 0x9BA8, 0x9BAA,
                                      0x9BAB, 0x9BAC, 0x9B41, 0x9BC0, 0x9BC2, 0x9BC4, 0x9BC5,
                                      0x9BC6, 0x9BC8, 0x9BCA, 0x9BCB, 0x9BCC, 0x9BE6, 0x9BF6};
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

    // Intel GPUs
    bool IsSkylake(PCIDeviceID deviceId) {
        return std::find(std::begin(Skylake), std::end(Skylake), deviceId) != std::end(Skylake);
    }
    bool IsKabylake(PCIDeviceID deviceId) {
        return std::find(std::begin(Kabylake), std::end(Kabylake), deviceId) != std::end(Kabylake);
    }
    bool IsCoffeelake(PCIDeviceID deviceId) {
        return (std::find(std::begin(Coffeelake), std::end(Coffeelake), deviceId) !=
                std::end(Coffeelake)) ||
               (std::find(std::begin(WhiskyLake), std::end(WhiskyLake), deviceId) !=
                std::end(WhiskyLake)) ||
               (std::find(std::begin(CometLake), std::end(CometLake), deviceId) !=
                std::end(CometLake));
    }
}  // namespace gpu_info
