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

#ifndef SRC_TINT_TRANSFORM_TRUNCATE_INTERSTAGE_VARIABLES_H_
#define SRC_TINT_TRANSFORM_TRUNCATE_INTERSTAGE_VARIABLES_H_

#include <bitset>

#include "src/tint/sem/binding_point.h"
#include "src/tint/transform/transform.h"

namespace tint::transform {

/// TruncateInterstageVariables is a transform that truncate interstage variables.
/// It must be run after CanonicalizeEntryPointIO which guarantees all interstage variables of
/// a given entry point are grouped into one shader IO struct.
/// It replaces `original shader IO struct` with a `new wrapper struct` containing builtin IOs
/// and user-defined IO whose locations are marked in the interstage_locations bitset from the
/// config. The return statements of `original shader IO struct` are wrapped by a mapping function
/// that initializes the members of `new wrapper struct` with values from `original shader IO
/// struct`. IO attributes of members in `original shader IO struct` are removed, other attributes
/// still preserve.
///
/// For example:
///
/// ```
///  struct ShaderIO {
///    @builtin(position) @invariant pos: vec4<f32>,
///    @location(1) f_1: f32,
///    @location(3) @align(16) f_3: f32,
///    @location(5) @interpolate(flat) @align(16) @size(16) f_5: u32,
///  }
///  @vertex
///  fn f() -> ShaderIO {
///    var io: ShaderIO;
///    io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
///    io.f_1 = 1.0;
///    io.f_3 = io.f_1 + 3.0;
///    io.f_5 = 1u;
///    return io;
///  }
/// ```
///
/// With config.interstage_locations[3] and [5] set to true, is transformed to:
///
/// ```
///  struct tint_symbol {
///    @builtin(position) @invariant
///    pos : vec4<f32>,
///    @location(3) @align(16)
///    f_3 : f32,
///    @location(5) @interpolate(flat) @align(16) @size(16)
///    f_5 : u32,
///  }
///
///  fn truncate_shader_output(io : ShaderIO) -> tint_symbol {
///    return tint_symbol(io.pos, io.f_3, io.f_5);
///  }
///
///  struct ShaderIO {
///    pos : vec4<f32>,
///    f_1 : f32,
///    @align(16)
///    f_3 : f32,
///    @align(16) @size(16)
///    f_5 : u32,
///  }
///
///  @vertex
///  fn f() -> tint_symbol {
///    var io : ShaderIO;
///    io.pos = vec4<f32>(1.0, 1.0, 1.0, 1.0);
///    io.f_1 = 1.0;
///    io.f_3 = (io.f_1 + 3.0);
///    io.f_5 = 1u;
///    return truncate_shader_output(io);
///  }
/// ```
///
class TruncateInterstageVariables final
    : public utils::Castable<TruncateInterstageVariables, Transform> {
  public:
    /// Configuration options for the transform
    struct Config final : public utils::Castable<Config, Data> {
        /// Constructor
        Config();

        /// Copy constructor
        Config(const Config&);

        /// Destructor
        ~Config() override;

        /// Assignment operator
        /// @returns this Config
        Config& operator=(const Config&);

        /// Indicate which interstage io locations are actually used by the later stage.
        /// There can be at most 16 user defined interstage variables with locations.
        std::bitset<16> interstage_locations;

        /// Reflect the fields of this class so that it can be used by tint::ForeachField()
        TINT_REFLECT(interstage_variables);
    };

    /// Constructor using a the configuration provided in the input Data
    TruncateInterstageVariables();

    /// Destructor
    ~TruncateInterstageVariables() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_TRUNCATE_INTERSTAGE_VARIABLES_H_
