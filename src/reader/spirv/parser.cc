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

#include "src/reader/spirv/parser.h"

#include <utility>

#include "src/reader/spirv/parser_impl.h"
#include "src/transform/decompose_strided_matrix.h"
#include "src/transform/manager.h"
#include "src/transform/remove_unreachable_statements.h"
#include "src/transform/simplify_pointers.h"
#include "src/transform/unshadow.h"

namespace tint {
namespace reader {
namespace spirv {

Program Parse(const std::vector<uint32_t>& input) {
  ParserImpl parser(input);
  bool parsed = parser.Parse();

  ProgramBuilder& builder = parser.builder();
  if (!parsed) {
    // TODO(bclayton): Migrate spirv::ParserImpl to using diagnostics.
    builder.Diagnostics().add_error(diag::System::Reader, parser.error());
    return Program(std::move(builder));
  }

  // The SPIR-V parser can construct disjoint AST nodes, which is invalid for
  // the Resolver. Clone the Program to clean these up.
  builder.SetResolveOnBuild(false);
  Program program_with_disjoint_ast(std::move(builder));

  ProgramBuilder output;
  CloneContext(&output, &program_with_disjoint_ast, false).Clone();
  auto program = Program(std::move(output));
  if (!program.IsValid()) {
    return program;
  }

  // If the generated program contains matrices with a custom MatrixStride
  // attribute then we need to decompose these into an array of vectors
  if (transform::DecomposeStridedMatrix::ShouldRun(&program)) {
    transform::Manager manager;
    manager.Add<transform::Unshadow>();
    manager.Add<transform::SimplifyPointers>();
    manager.Add<transform::DecomposeStridedMatrix>();
    manager.Add<transform::RemoveUnreachableStatements>();
    return manager.Run(&program).program;
  }

  return program;
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint
