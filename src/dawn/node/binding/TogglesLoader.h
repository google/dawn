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

#ifndef SRC_DAWN_NODE_BINDING_TOGGLESLOADER_H_
#define SRC_DAWN_NODE_BINDING_TOGGLESLOADER_H_

#include <string>
#include <vector>

#include "dawn/webgpu_cpp.h"
#include "src/dawn/node/binding/Flags.h"

namespace wgpu::binding {
class TogglesLoader {
  public:
    // Constructor, reading toggles from the "enable-dawn-features"
    // and "disable-dawn-features" flags.
    explicit TogglesLoader(const Flags& flags);

    // Returns a DawnTogglesDescriptor populated with toggles
    // read at constructor time. It is only valid for the lifetime
    // of this TogglesLoader object.
    DawnTogglesDescriptor GetDescriptor();

  private:
    // Ban copy-assignment and copy-construction. The compiler will
    // create implicitly-declared move-constructor and move-assignment
    // implementations as needed.
    TogglesLoader(const TogglesLoader& other) = delete;
    TogglesLoader& operator=(const TogglesLoader&) = delete;

    // DawnTogglesDescriptor::enabledToggles and disabledToggles are vectors
    // of 'const char*', so keep local copies of the strings, and don't allow
    // them to be relocated.
    std::vector<std::string> enabledTogglesStrings_;
    std::vector<std::string> disabledTogglesStrings_;
    std::vector<const char*> enabledToggles_;
    std::vector<const char*> disabledToggles_;
};
}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_TOGGLESLOADER_H_
