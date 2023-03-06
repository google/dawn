// Copyright 2021 The Tint Authors.
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

#include "src/tint/ast/disable_validation_attribute.h"
#include "src/tint/clone_context.h"
#include "src/tint/program_builder.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::DisableValidationAttribute);

namespace tint::ast {

DisableValidationAttribute::DisableValidationAttribute(ProgramID pid,
                                                       NodeID nid,
                                                       DisabledValidation val)
    : Base(pid, nid, utils::Empty), validation(val) {}

DisableValidationAttribute::~DisableValidationAttribute() = default;

std::string DisableValidationAttribute::InternalName() const {
    switch (validation) {
        case DisabledValidation::kFunctionHasNoBody:
            return "disable_validation__function_has_no_body";
        case DisabledValidation::kBindingPointCollision:
            return "disable_validation__binding_point_collision";
        case DisabledValidation::kIgnoreAddressSpace:
            return "disable_validation__ignore_address_space";
        case DisabledValidation::kEntryPointParameter:
            return "disable_validation__entry_point_parameter";
        case DisabledValidation::kFunctionParameter:
            return "disable_validation__function_parameter";
        case DisabledValidation::kIgnoreStrideAttribute:
            return "disable_validation__ignore_stride";
        case DisabledValidation::kIgnoreInvalidPointerArgument:
            return "disable_validation__ignore_invalid_pointer_argument";
        case DisabledValidation::kIgnorePointerAliasing:
            return "disable_validation__ignore_pointer_aliasing";
        case DisabledValidation::kIgnoreStructMemberLimit:
            return "disable_validation__ignore_struct_member";
    }
    return "<invalid>";
}

const DisableValidationAttribute* DisableValidationAttribute::Clone(CloneContext* ctx) const {
    return ctx->dst->ASTNodes().Create<DisableValidationAttribute>(
        ctx->dst->ID(), ctx->dst->AllocateNodeID(), validation);
}

}  // namespace tint::ast
