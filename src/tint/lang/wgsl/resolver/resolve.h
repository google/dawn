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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_RESOLVE_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_RESOLVE_H_

namespace tint {
class Program;
class ProgramBuilder;
}  // namespace tint

namespace tint::resolver {

/// Performs semantic analysis and validation on the program builder @p builder
/// @returns the resolved Program. Program.Diagnostics() may contain validation errors.
Program Resolve(ProgramBuilder& builder);

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_RESOLVE_H_
