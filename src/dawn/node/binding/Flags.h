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

#ifndef SRC_DAWN_NODE_BINDING_FLAGS_H_
#define SRC_DAWN_NODE_BINDING_FLAGS_H_

#include <optional>
#include <string>
#include <unordered_map>

namespace wgpu::binding {
// Flags maintains a key-value mapping of input flags passed into the module's create()
// function, used to configure dawn_node.
class Flags {
  public:
    void Set(const std::string& key, const std::string& value);
    std::optional<std::string> Get(const std::string& key) const;

  private:
    std::unordered_map<std::string, std::string> flags_;
};
}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_FLAGS_H_
