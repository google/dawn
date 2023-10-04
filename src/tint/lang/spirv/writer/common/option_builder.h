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

#ifndef SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTION_BUILDER_H_
#define SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTION_BUILDER_H_

#include <unordered_map>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/api/options/external_texture.h"
#include "src/tint/lang/spirv/writer/common/options.h"
#include "src/tint/utils/diagnostic/diagnostic.h"

namespace tint::spirv::writer {

using RemapperData = std::unordered_map<BindingPoint, BindingPoint>;

/// @param options the options
/// @returns true if the binding points are valid
bool ValidateBindingOptions(const Options& options, diag::List& diagnostics);

/// Populates data from the writer options for the remapper and external texture.
/// @param options the writer options
/// @param remapper_data where to put the remapper data
/// @param external_texture where to store the external texture options
/// Note, these are populated together because there are dependencies between the two types of data.
void PopulateRemapperAndMultiplanarOptions(const Options& options,
                                           RemapperData& remapper_data,
                                           ExternalTextureOptions& external_texture);

}  // namespace tint::spirv::writer

#endif  // SRC_TINT_LANG_SPIRV_WRITER_COMMON_OPTION_BUILDER_H_
