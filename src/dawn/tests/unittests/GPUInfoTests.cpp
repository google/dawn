// Copyright 2021 The Dawn Authors
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

#include <gtest/gtest.h>

#include "dawn/common/GPUInfo.h"

namespace {
const PCIVendorID vendorID = 0x8086;
const gpu_info::D3DDriverVersion version1 = {20, 19, 15, 5107};
const gpu_info::D3DDriverVersion version2 = {21, 20, 16, 5077};
const gpu_info::D3DDriverVersion version3 = {27, 20, 100, 9946};
const gpu_info::D3DDriverVersion version4 = {27, 20, 101, 2003};
}  // anonymous namespace

TEST(GPUInfo, CompareD3DDriverVersion) {
    EXPECT_EQ(gpu_info::CompareD3DDriverVersion(vendorID, version1, version2), -1);
    EXPECT_EQ(gpu_info::CompareD3DDriverVersion(vendorID, version2, version3), -1);
    EXPECT_EQ(gpu_info::CompareD3DDriverVersion(vendorID, version3, version4), -1);
}
