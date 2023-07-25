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

#include "src/tint/lang/wgsl/ast/transform/data.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::Data);

namespace tint::ast::transform {

Data::Data() = default;
Data::Data(const Data&) = default;
Data::~Data() = default;
Data& Data::operator=(const Data&) = default;

DataMap::DataMap() = default;
DataMap::DataMap(DataMap&&) = default;
DataMap::~DataMap() = default;
DataMap& DataMap::operator=(DataMap&&) = default;

}  // namespace tint::ast::transform
