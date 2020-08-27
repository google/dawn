// Copyright 2020 The Tint Authors.
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

#include "src/validator_test_helper.h"
#include "src/type_manager.h"

namespace tint {

ValidatorTestHelper::ValidatorTestHelper() {
  td_ = std::make_unique<TypeDeterminer>(&ctx_, &mod_);
  v_ = std::make_unique<ValidatorImpl>();
}

ValidatorTestHelper::~ValidatorTestHelper() = default;

void ValidatorTestHelper::AddFakeEntryPoint() {
  // entry_point vertex as "fake_entry_point" = fake_func
  // fn fake_func() -> void {}
  auto ep = std::make_unique<ast::EntryPoint>(ast::PipelineStage::kVertex,
                                              "fake_entry_point", "fake_func");
  ast::VariableList fake_params;
  auto fake_func = std::make_unique<ast::Function>(
      "fake_func", std::move(fake_params),
      ctx_.type_mgr().Get(std::make_unique<ast::type::VoidType>()));
  auto fake_body = std::make_unique<ast::BlockStatement>();
  auto return_stmt = std::make_unique<ast::ReturnStatement>();
  fake_body->append(std::move(return_stmt));
  fake_func->set_body(std::move(fake_body));
  mod()->AddFunction(std::move(fake_func));
  mod()->AddEntryPoint(std::move(ep));
  return;
}

}  // namespace tint
