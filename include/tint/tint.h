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

#ifndef INCLUDE_TINT_TINT_H_
#define INCLUDE_TINT_TINT_H_

// TODO(tint:88): When implementing support for an install target, all of these
//                headers will need to be moved to include/tint/.

#include "src/context.h"
#include "src/reader/reader.h"
#include "src/type_determiner.h"
#include "src/type_manager.h"
#include "src/validator.h"
#include "src/writer/writer.h"

#include "src/ast/transform/vertex_pulling_transform.h"

#if TINT_BUILD_SPV_READER
#include "src/reader/spirv/parser.h"
#endif  // TINT_BUILD_SPV_READER

#if TINT_BUILD_WGSL_READER
#include "src/reader/wgsl/parser.h"
#endif  // TINT_BUILD_WGSL_READER

#if TINT_BUILD_SPV_WRITER
#include "spirv-tools/libspirv.hpp"
#include "src/writer/spirv/generator.h"
#endif  // TINT_BUILD_SPV_WRITER

#if TINT_BUILD_WGSL_WRITER
#include "src/writer/wgsl/generator.h"
#endif  // TINT_BUILD_WGSL_WRITER

#if TINT_BUILD_MSL_WRITER
#include "src/writer/msl/generator.h"
#endif  // TINT_BUILD_MSL_WRITER

#if TINT_BUILD_HLSL_WRITER
#include "src/writer/hlsl/generator.h"
#endif  // TINT_BUILD_HLSL_WRITER

#endif  // INCLUDE_TINT_TINT_H_
