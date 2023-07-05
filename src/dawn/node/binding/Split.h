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

#ifndef SRC_DAWN_NODE_BINDING_SPLIT_H_
#define SRC_DAWN_NODE_BINDING_SPLIT_H_

#include <string>
#include <vector>

namespace wgpu::binding {
// Returns a list of non-empty strings formed by splitting the given string at
// the given delimiter character.
std::vector<std::string> Split(const std::string& s, char delim);
}  // namespace wgpu::binding
   //
#endif  // SRC_DAWN_NODE_BINDING_SPLIT_H_
