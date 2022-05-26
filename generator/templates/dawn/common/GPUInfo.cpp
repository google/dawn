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

#include <algorithm>
#include <array>
#include <sstream>
#include <iomanip>

#include "dawn/common/GPUInfo_autogen.h"

#include "dawn/common/Assert.h"

namespace gpu_info {

namespace {

enum class Architecture {
    Unknown,
    {% for vendor in vendors %}
        {% for architecture in vendor.architectures %}
            {{vendor.name.CamelCase()}}_{{architecture.name.CamelCase()}},
        {% endfor %}
    {% endfor %}
};

Architecture GetArchitecture(PCIVendorID vendorId, PCIDeviceID deviceId) {
    switch(vendorId) {
        {% for vendor in vendors %}
            {% if len(vendor.architectures) %}

                case kVendorID_{{vendor.name.CamelCase()}}: {
                    switch (deviceId{{vendor.maskDeviceId()}}) {
                        {% for architecture in vendor.architectures %}
                            {% for device in architecture.devices %}
                                case {{device}}:
                            {% endfor %}
                                return Architecture::{{vendor.name.CamelCase()}}_{{architecture.name.CamelCase()}};
                        {% endfor %}
                    }
                } break;
            {% endif %}
        {% endfor %}
    }

    return Architecture::Unknown;
}

}  // namespace

// Vendor checks
{% for vendor in vendors %}
    bool Is{{vendor.name.CamelCase()}}(PCIVendorID vendorId) {
        return vendorId == kVendorID_{{vendor.name.CamelCase()}};
    }
{% endfor %}

// Architecture checks

{% for vendor in vendors %}
    {% if len(vendor.architectures) %}
        // {{vendor.name.get()}} architectures
        {% for architecture in vendor.architectures %}
            bool Is{{vendor.name.CamelCase()}}{{architecture.name.CamelCase()}}(PCIVendorID vendorId, PCIDeviceID deviceId) {
                return GetArchitecture(vendorId, deviceId) == Architecture::{{vendor.name.CamelCase()}}_{{architecture.name.CamelCase()}};
            }
        {% endfor %}
    {% endif %}
{% endfor %}

// GPUAdapterInfo fields
std::string GetVendorName(PCIVendorID vendorId) {
    switch(vendorId) {
        {% for vendor in vendors %}
            case kVendorID_{{vendor.name.CamelCase()}}: return "{{vendor.name.js_enum_case()}}";
        {% endfor %}
    }

    return "";
}

std::string GetArchitectureName(PCIVendorID vendorId, PCIDeviceID deviceId) {
    Architecture arch = GetArchitecture(vendorId, deviceId);
    switch(arch) {
        case Architecture::Unknown:
            return "";
        {% for vendor in vendors %}
            {% for architecture in vendor.architectures %}
                case Architecture::{{vendor.name.CamelCase()}}_{{architecture.name.CamelCase()}}:
                    return "{{architecture.name.js_enum_case()}}";
            {% endfor %}
        {% endfor %}
    }
}

}  // namespace gpu_info
