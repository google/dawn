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

#include "src/dawn/node/binding/Split.h"

namespace wgpu::binding {

std::vector<std::string> Split(const std::string& s, char delim) {
    if (s.empty()) {
        return {};
    }

    std::vector<std::string> result;
    const size_t lastIndex = s.length() - 1;
    size_t startIndex = 0;
    size_t i = startIndex;

    while (i <= lastIndex) {
        if (s[i] == delim) {
            auto token = s.substr(startIndex, i - startIndex);
            if (!token.empty()) {  // Discard empty tokens
                result.push_back(token);
            }
            startIndex = i + 1;
        } else if (i == lastIndex) {
            auto token = s.substr(startIndex, i - startIndex + 1);
            if (!token.empty()) {  // Discard empty tokens
                result.push_back(token);
            }
        }
        ++i;
    }
    return result;
}

}  // namespace wgpu::binding
