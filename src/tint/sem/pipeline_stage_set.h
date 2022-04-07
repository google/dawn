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

#ifndef SRC_TINT_SEM_PIPELINE_STAGE_SET_H_
#define SRC_TINT_SEM_PIPELINE_STAGE_SET_H_

#include "src/tint/ast/pipeline_stage.h"
#include "src/tint/utils/enum_set.h"

namespace tint::sem {

using PipelineStageSet = utils::EnumSet<ast::PipelineStage>;

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_PIPELINE_STAGE_SET_H_
