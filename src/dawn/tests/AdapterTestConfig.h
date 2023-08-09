// Copyright 2022 The Dawn Authors
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

#ifndef SRC_DAWN_TESTS_ADAPTERTESTCONFIG_H_
#define SRC_DAWN_TESTS_ADAPTERTESTCONFIG_H_

#include <stdint.h>

#include <initializer_list>
#include <ostream>
#include <string>
#include <vector>

#include "dawn/webgpu_cpp.h"

struct BackendTestConfig {
    BackendTestConfig(wgpu::BackendType backendType,
                      std::initializer_list<const char*> forceEnabledWorkarounds = {},
                      std::initializer_list<const char*> forceDisabledWorkarounds = {});

    wgpu::BackendType backendType;

    std::vector<const char*> forceEnabledWorkarounds;
    std::vector<const char*> forceDisabledWorkarounds;
};

struct TestAdapterProperties {
    TestAdapterProperties(const wgpu::AdapterProperties& properties, bool selected);
    uint32_t vendorID;
    std::string vendorName;
    std::string architecture;
    uint32_t deviceID;
    std::string name;
    std::string driverDescription;
    wgpu::AdapterType adapterType;
    wgpu::BackendType backendType;
    bool compatibilityMode;
    bool selected;

    std::string ParamName() const;
    std::string AdapterTypeName() const;
};

struct AdapterTestParam {
    AdapterTestParam(const BackendTestConfig& config,
                     const TestAdapterProperties& adapterProperties);

    TestAdapterProperties adapterProperties;
    std::vector<const char*> forceEnabledWorkarounds;
    std::vector<const char*> forceDisabledWorkarounds;
};

std::ostream& operator<<(std::ostream& os, const AdapterTestParam& param);

BackendTestConfig D3D11Backend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                               std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig D3D12Backend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                               std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig MetalBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                               std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig NullBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                              std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig OpenGLBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                                std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig OpenGLESBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                                  std::initializer_list<const char*> forceDisabledWorkarounds = {});

BackendTestConfig VulkanBackend(std::initializer_list<const char*> forceEnabledWorkarounds = {},
                                std::initializer_list<const char*> forceDisabledWorkarounds = {});

#endif  // SRC_DAWN_TESTS_ADAPTERTESTCONFIG_H_
