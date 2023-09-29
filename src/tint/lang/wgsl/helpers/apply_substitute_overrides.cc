// Copyright 2023 The Tint Authors.
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

#include "src/tint/lang/wgsl/helpers/apply_substitute_overrides.h"

#include <memory>
#include <utility>

#include "src/tint/lang/wgsl/ast/transform/manager.h"
#include "src/tint/lang/wgsl/ast/transform/substitute_override.h"
#include "src/tint/lang/wgsl/inspector/inspector.h"
#include "src/tint/lang/wgsl/program/program.h"

namespace tint::wgsl {

std::optional<Program> ApplySubstituteOverrides(const Program& program) {
    ast::transform::SubstituteOverride::Config cfg;
    inspector::Inspector inspector(program);
    auto default_values = inspector.GetOverrideDefaultValues();
    for (const auto& [override_id, scalar] : default_values) {
        // If the override is not null, then it has a default value, we can just let it use the
        // provided default instead of overriding.
        if (scalar.IsNull()) {
            cfg.map.insert({override_id, 0.0});
        }
    }

    if (default_values.empty()) {
        return std::nullopt;
    }

    ast::transform::DataMap override_data;
    override_data.Add<ast::transform::SubstituteOverride::Config>(cfg);

    ast::transform::Manager mgr;
    mgr.append(std::make_unique<ast::transform::SubstituteOverride>());

    ast::transform::DataMap outputs;
    return mgr.Run(program, override_data, outputs);
}

}  // namespace tint::wgsl
