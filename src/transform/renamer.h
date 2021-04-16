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

#ifndef SRC_TRANSFORM_RENAMER_H_
#define SRC_TRANSFORM_RENAMER_H_

#include <string>
#include <unordered_map>

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// Renamer is a Transform that renames all the symbols in a program.
class Renamer : public Transform {
 public:
  /// Data is outputted by the Renamer transform.
  /// Data holds information about shader usage and constant buffer offsets.
  struct Data : public Castable<Data, transform::Data> {
    /// Remappings is a map of old symbol name to new symbol name
    using Remappings = std::unordered_map<std::string, std::string>;

    /// Constructor
    /// @param remappings the symbol remappings
    explicit Data(Remappings&& remappings);

    /// Copy constructor
    Data(const Data&);

    /// Destructor
    ~Data() override;

    /// A map of old symbol name to new symbol name
    Remappings const remappings;
  };

  /// Target is an enumerator of rename targets that can be used
  enum class Target {
    /// Rename every symbol.
    kAll,
    /// Only rename symbols that are reserved keywords in HLSL.
    kHlslKeywords,
    /// Only rename symbols that are reserved keywords in MSL.
    kMslKeywords,
  };

  /// Configuration options for the transform
  struct Config {
    /// The targets to rename
    Target target = Target::kAll;
  };

  /// Constructor using a default configuration
  Renamer();

  /// Constructor
  /// @param config the configuration for the transform
  explicit Renamer(const Config& config);

  /// Destructor
  ~Renamer() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific input data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;

 private:
  Config const cfg_;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_RENAMER_H_
