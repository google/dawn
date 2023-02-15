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

#include "src/tint/sem/switch_statement.h"

#include "src/tint/ast/case_statement.h"
#include "src/tint/ast/switch_statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::sem::CaseStatement);
TINT_INSTANTIATE_TYPEINFO(tint::sem::CaseSelector);
TINT_INSTANTIATE_TYPEINFO(tint::sem::SwitchStatement);

namespace tint::sem {

SwitchStatement::SwitchStatement(const ast::SwitchStatement* declaration,
                                 const CompoundStatement* parent,
                                 const sem::Function* function)
    : Base(declaration, parent, function) {
    TINT_ASSERT(Semantic, parent);
    TINT_ASSERT(Semantic, function);
}

SwitchStatement::~SwitchStatement() = default;

const ast::SwitchStatement* SwitchStatement::Declaration() const {
    return static_cast<const ast::SwitchStatement*>(Base::Declaration());
}

CaseStatement::CaseStatement(const ast::CaseStatement* declaration,
                             const CompoundStatement* parent,
                             const sem::Function* function)
    : Base(declaration, parent, function) {
    TINT_ASSERT(Semantic, parent);
    TINT_ASSERT(Semantic, function);
}
CaseStatement::~CaseStatement() = default;

const ast::CaseStatement* CaseStatement::Declaration() const {
    return static_cast<const ast::CaseStatement*>(Base::Declaration());
}

CaseSelector::CaseSelector(const ast::CaseSelector* decl, const constant::Value* val)
    : Base(), decl_(decl), val_(val) {}

CaseSelector::~CaseSelector() = default;

const ast::CaseSelector* CaseSelector::Declaration() const {
    return decl_;
}

}  // namespace tint::sem
