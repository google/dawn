// Copyright 2023 The Dawn Authors
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

#include "src/dawn/node/binding/TogglesLoader.h"
#include "src/dawn/node/binding/Split.h"

namespace wgpu::binding {

TogglesLoader::TogglesLoader(const Flags& flags) {
    if (auto values = flags.Get("enable-dawn-features")) {
        enabledTogglesStrings_ = Split(*values, ',');
        for (auto& t : enabledTogglesStrings_) {
            enabledToggles_.emplace_back(t.c_str());
        }
    }
    if (auto values = flags.Get("disable-dawn-features")) {
        disabledTogglesStrings_ = Split(*values, ',');
        for (auto& t : disabledTogglesStrings_) {
            disabledToggles_.emplace_back(t.c_str());
        }
    }
}

DawnTogglesDescriptor TogglesLoader::GetDescriptor() {
    DawnTogglesDescriptor result;
    result.enabledToggleCount = enabledToggles_.size();
    result.enabledToggles = enabledToggles_.data();
    result.disabledToggleCount = disabledToggles_.size();
    result.disabledToggles = disabledToggles_.data();
    return result;
}
}  // namespace wgpu::binding
