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

#include "src/dawn/node/binding/Flags.h"

namespace wgpu::binding {
void Flags::Set(const std::string& key, const std::string& value) {
    flags_[key] = value;
}

std::optional<std::string> Flags::Get(const std::string& key) const {
    auto iter = flags_.find(key);
    if (iter != flags_.end()) {
        return iter->second;
    }
    return {};
}
}  // namespace wgpu::binding
