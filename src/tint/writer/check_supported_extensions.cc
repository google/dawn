// Copyright 2022 The Tint Authors.
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

#include "src/tint/writer/check_supported_extensions.h"

#include <string>

#include "src/tint/ast/module.h"
#include "src/tint/diagnostic/diagnostic.h"
#include "src/tint/utils/hashset.h"
#include "src/tint/utils/string.h"

namespace tint::writer {

bool CheckSupportedExtensions(std::string_view writer_name,
                              const ast::Module& module,
                              diag::List& diags,
                              utils::VectorRef<builtin::Extension> supported) {
    utils::Hashset<builtin::Extension, 32> set;
    for (auto ext : supported) {
        set.Add(ext);
    }

    for (auto* enable : module.Enables()) {
        for (auto* ext : enable->extensions) {
            if (!set.Contains(ext->name)) {
                diags.add_error(diag::System::Writer,
                                std::string(writer_name) + " backend does not support extension '" +
                                    utils::ToString(ext->name) + "'",
                                ext->source);
                return false;
            }
        }
    }
    return true;
}

}  // namespace tint::writer
