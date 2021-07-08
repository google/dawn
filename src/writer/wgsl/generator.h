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

#ifndef SRC_WRITER_WGSL_GENERATOR_H_
#define SRC_WRITER_WGSL_GENERATOR_H_

#include <memory>
#include <string>

#include "src/writer/text.h"

namespace tint {

// Forward declarations
class Program;

namespace writer {
namespace wgsl {

class GeneratorImpl;

/// Configuration options used for generating WGSL.
struct Options {};

/// The result produced when generating WGSL.
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

  /// The generated WGSL.
  std::string wgsl = "";
};

/// Generate WGSL for a program, according to a set of configuration options.
/// The result will contain the WGSL, as well as success status and diagnostic
/// information.
/// @param program the program to translate to WGSL
/// @param options the configuration options to use when generating WGSL
/// @returns the resulting WGSL and supplementary information
Result Generate(const Program* program, const Options& options);

// TODO(jrprice): Remove this once Dawn is using the new interface.
/// Class to generate WGSL source
class Generator : public Text {
 public:
  /// Constructor
  /// @param program the program to convert
  explicit Generator(const Program* program);

  /// Destructor
  ~Generator() override;

  /// Generates the result data
  /// @returns true on successful generation; false otherwise
  bool Generate() override;

  /// @returns the result data
  std::string result() const override;

  /// @returns the error
  std::string error() const;

 private:
  Generator(const Generator&) = delete;
  Generator& operator=(const Generator&) = delete;

  std::unique_ptr<GeneratorImpl> impl_;
};

}  // namespace wgsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_WGSL_GENERATOR_H_
