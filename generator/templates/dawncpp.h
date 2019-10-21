//* Copyright 2017 The Dawn Authors
//*
//* Licensed under the Apache License, Version 2.0 (the "License");
//* you may not use this file except in compliance with the License.
//* You may obtain a copy of the License at
//*
//*     http://www.apache.org/licenses/LICENSE-2.0
//*
//* Unless required by applicable law or agreed to in writing, software
//* distributed under the License is distributed on an "AS IS" BASIS,
//* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//* See the License for the specific language governing permissions and
//* limitations under the License.

// This temporary header translates all the previous Dawn C++ API to the webgpu_cpp.h
// API so that during a small transition period both headers are supported.

#ifndef DAWN_DAWNCPP_H_
#define DAWN_DAWNCPP_H_

#include "dawn/dawn.h"
#include "dawn/webgpu_cpp.h"

namespace dawn {

    static constexpr uint64_t kWholeSize = wgpu::kWholeSize;

    {% for type in by_category["enum"] %}
        using {{as_cppType(type.name)}} = wgpu::{{as_cppType(type.name)}};
    {% endfor %}

    {% for type in by_category["bitmask"] %}
        using {{as_cppType(type.name)}} = wgpu::{{as_cppType(type.name)}};
    {% endfor %}

    using Proc = wgpu::Proc;
    {% for type in by_category["natively defined"] %}
        using {{as_cppType(type.name)}} = wgpu::{{as_cppType(type.name)}};
    {% endfor %}

    {% for type in by_category["object"] %}
        using {{as_cppType(type.name)}} = wgpu::{{as_cppType(type.name)}};
    {% endfor %}

    {% for type in by_category["structure"] %}
        using {{as_cppType(type.name)}} = wgpu::{{as_cppType(type.name)}};
    {% endfor %}

    static inline Proc GetProcAddress(Device const& device, const char* procName) {
        return wgpu::GetProcAddress(device, procName);
    }
}  // namespace dawn

#endif // DAWN_DAWNCPP_H_
