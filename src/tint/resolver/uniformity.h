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

#ifndef SRC_TINT_RESOLVER_UNIFORMITY_H_
#define SRC_TINT_RESOLVER_UNIFORMITY_H_

// Forward declarations.
namespace tint::resolver {
struct DependencyGraph;
}  // namespace tint::resolver
namespace tint {
class ProgramBuilder;
}  // namespace tint

namespace tint::resolver {

/// If true, uniformity analysis failures will be treated as an error, else as a warning.
constexpr bool kUniformityFailuresAsError = true;

/// Analyze the uniformity of a program.
/// @param builder the program to analyze
/// @param dependency_graph the dependency-ordered module-scope declarations
/// @returns true if there are no uniformity issues, false otherwise
bool AnalyzeUniformity(ProgramBuilder* builder, const resolver::DependencyGraph& dependency_graph);

}  // namespace tint::resolver

#endif  // SRC_TINT_RESOLVER_UNIFORMITY_H_
