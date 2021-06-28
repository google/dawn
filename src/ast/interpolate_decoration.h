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

#ifndef SRC_AST_INTERPOLATE_DECORATION_H_
#define SRC_AST_INTERPOLATE_DECORATION_H_

#include <ostream>
#include <string>

#include "src/ast/decoration.h"

namespace tint {
namespace ast {

/// The interpolation type.
enum class InterpolationType { kPerspective, kLinear, kFlat };

/// The interpolation sampling.
enum class InterpolationSampling { kNone = -1, kCenter, kCentroid, kSample };

/// An interpolate decoration
class InterpolateDecoration
    : public Castable<InterpolateDecoration, Decoration> {
 public:
  /// Create an interpolate decoration.
  /// @param program_id the identifier of the program that owns this node
  /// @param source the source of this decoration
  /// @param type the interpolation type
  /// @param sampling the interpolation sampling
  InterpolateDecoration(ProgramID program_id,
                        const Source& source,
                        InterpolationType type,
                        InterpolationSampling sampling);
  ~InterpolateDecoration() override;

  /// @returns the interpolation type
  InterpolationType type() const { return type_; }

  /// @returns the interpolation sampling
  InterpolationSampling sampling() const { return sampling_; }

  /// @returns the WGSL name for the decoration
  std::string name() const override;

  /// Outputs the decoration to the given stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  InterpolateDecoration* Clone(CloneContext* ctx) const override;

 private:
  InterpolationType const type_;
  InterpolationSampling const sampling_;
};

/// @param out the std::ostream to write to
/// @param type the interpolation type
/// @return the std::ostream so calls can be chained
std::ostream& operator<<(std::ostream& out, InterpolationType type);

/// @param out the std::ostream to write to
/// @param sampling the interpolation sampling
/// @return the std::ostream so calls can be chained
std::ostream& operator<<(std::ostream& out, InterpolationSampling sampling);

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_INTERPOLATE_DECORATION_H_
