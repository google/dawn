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

#ifndef SRC_TINT_WRITER_SYNTAX_TREE_GENERATOR_H_
#define SRC_TINT_WRITER_SYNTAX_TREE_GENERATOR_H_

#include <memory>
#include <string>

#include "src/tint/writer/text.h"

// Forward declarations
namespace tint {
class Program;
}  // namespace tint

namespace tint::writer::syntax_tree {

class GeneratorImpl;

/// Configuration options used for generating AST.
struct Options {};

/// The result produced when generating AST.
struct Result {
    /// Constructor
    Result();

    /// Destructor
    ~Result();

    /// Copy constructor
    Result(const Result&);

    /// True if generation was successful.
    bool success = false;

    /// The errors generated during code generation, if any.
    std::string error;

    /// The generated AST.
    std::string ast = "";
};

/// Generate an AST dump for a program, according to a set of configuration options.
/// The result will contain the AST, as well as success status and diagnostic information.
/// @param program the program to dump
/// @param options the configuration options to use when dumping
/// @returns the resulting AST dump and supplementary information
Result Generate(const Program* program, const Options& options);

}  // namespace tint::writer::syntax_tree

#endif  // SRC_TINT_WRITER_SYNTAX_TREE_GENERATOR_H_
