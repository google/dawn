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

#ifndef SRC_DAWN_COMMON_GPUINFO_AUTOGEN_H_
#define SRC_DAWN_COMMON_GPUINFO_AUTOGEN_H_

#include <cstdint>
#include <string>

using PCIVendorID = uint32_t;
using PCIDeviceID = uint32_t;

namespace gpu_info {

// Vendor IDs
{% for vendor in vendors %}
    static constexpr PCIVendorID kVendorID_{{vendor.name.CamelCase()}} = {{vendor.id}};
{% endfor %}

// Vendor checks
{% for vendor in vendors %}
    bool Is{{vendor.name.CamelCase()}}(PCIVendorID vendorId);
{% endfor %}

// Architecture checks
{% for vendor in vendors %}
    {% if len(vendor.architecture_names) %}

        // {{vendor.name.get()}} architectures
        {% for architecture_name in vendor.architecture_names %}
            bool Is{{vendor.name.CamelCase()}}{{architecture_name.CamelCase()}}(PCIVendorID vendorId, PCIDeviceID deviceId);
        {% endfor %}
        {% for architecture_name in vendor.internal_architecture_names %}
            bool Is{{vendor.name.CamelCase()}}{{architecture_name.CamelCase()}}(PCIVendorID vendorId, PCIDeviceID deviceId);
        {% endfor %}
    {% endif %}
{% endfor %}

// GPUAdapterInfo fields
std::string GetVendorName(PCIVendorID vendorId);
std::string GetArchitectureName(PCIVendorID vendorId, PCIDeviceID deviceId);

} // namespace gpu_info
#endif  // SRC_DAWN_COMMON_GPUINFO_AUTOGEN_H_
