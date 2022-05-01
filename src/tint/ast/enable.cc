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

#include "src/tint/ast/enable.h"

#include "src/tint/program_builder.h"
#include "src/tint/sem/variable.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::Enable);

namespace tint::ast {

Enable::ExtensionKind Enable::NameToKind(const std::string& name) {
    // The reserved internal extension name for testing
    if (name == "InternalExtensionForTesting") {
        return Enable::ExtensionKind::kInternalExtensionForTesting;
    }

    return Enable::ExtensionKind::kNotAnExtension;
}

std::string Enable::KindToName(ExtensionKind kind) {
    switch (kind) {
        // The reserved internal extension for testing
        case ExtensionKind::kInternalExtensionForTesting:
            return "InternalExtensionForTesting";
        case ExtensionKind::kNotAnExtension:
            // Return an empty string for kNotAnExtension
            return {};
            // No default case, as this switch must cover all ExtensionKind values.
    }
    // This return shall never get hit.
    return {};
}

Enable::Enable(ProgramID pid, const Source& src, const std::string& ext_name)
    : Base(pid, src), name(ext_name), kind(NameToKind(ext_name)) {}

Enable::Enable(Enable&&) = default;

Enable::~Enable() = default;

const Enable* Enable::Clone(CloneContext* ctx) const {
    auto src = ctx->Clone(source);
    return ctx->dst->create<Enable>(src, name);
}
}  // namespace tint::ast
