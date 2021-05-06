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

#ifndef SRC_TRANSFORM_VERTEX_PULLING_H_
#define SRC_TRANSFORM_VERTEX_PULLING_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// Describes the format of data in a vertex buffer
enum class VertexFormat {
  kVec2U8,
  kVec4U8,
  kVec2I8,
  kVec4I8,
  kVec2U8Norm,
  kVec4U8Norm,
  kVec2I8Norm,
  kVec4I8Norm,
  kVec2U16,
  kVec4U16,
  kVec2I16,
  kVec4I16,
  kVec2U16Norm,
  kVec4U16Norm,
  kVec2I16Norm,
  kVec4I16Norm,
  kVec2F16,
  kVec4F16,
  kF32,
  kVec2F32,
  kVec3F32,
  kVec4F32,
  kU32,
  kVec2U32,
  kVec3U32,
  kVec4U32,
  kI32,
  kVec2I32,
  kVec3I32,
  kVec4I32,
  kLastEntry = kVec4I32
};

/// Describes if a vertex attributes increments with vertex index or instance
/// index
enum class InputStepMode { kVertex, kInstance, kLastEntry = kInstance };

/// Describes a vertex attribute within a buffer
struct VertexAttributeDescriptor {
  /// The format of the attribute
  VertexFormat format;
  /// The byte offset of the attribute in the buffer
  uint64_t offset;
  /// The shader location used for the attribute
  uint32_t shader_location;
};

/// Describes a buffer containing multiple vertex attributes
struct VertexBufferLayoutDescriptor {
  /// Constructor
  VertexBufferLayoutDescriptor();
  /// Constructor
  /// @param in_array_stride the array stride of the in buffer
  /// @param in_step_mode the step mode of the in buffer
  /// @param in_attributes the in attributes
  VertexBufferLayoutDescriptor(
      uint64_t in_array_stride,
      InputStepMode in_step_mode,
      std::vector<VertexAttributeDescriptor> in_attributes);
  /// Copy constructor
  /// @param other the struct to copy
  VertexBufferLayoutDescriptor(const VertexBufferLayoutDescriptor& other);

  /// Assignment operator
  /// @param other the struct to copy
  /// @returns this struct
  VertexBufferLayoutDescriptor& operator=(
      const VertexBufferLayoutDescriptor& other);

  ~VertexBufferLayoutDescriptor();

  /// The array stride used in the in buffer
  uint64_t array_stride = 0u;
  /// The input step mode used
  InputStepMode step_mode = InputStepMode::kVertex;
  /// The vertex attributes
  std::vector<VertexAttributeDescriptor> attributes;
};

/// Describes vertex state, which consists of many buffers containing vertex
/// attributes
using VertexStateDescriptor = std::vector<VertexBufferLayoutDescriptor>;

/// Converts a program to use vertex pulling
///
/// Variables which accept vertex input are var<in> with a location decoration.
/// This transform will convert those to be assigned from storage buffers
/// instead. The intention is to allow vertex input to rely on a storage buffer
/// clamping pass for out of bounds reads. We bind the storage buffers as arrays
/// of u32, so any read to byte position `p` will actually need to read position
/// `p / 4`, since `sizeof(u32) == 4`.
///
/// `VertexFormat` represents the input type of the attribute. This isn't
/// related to the type of the variable in the shader. For example,
/// `VertexFormat::kVec2F16` tells us that the buffer will contain `f16`
/// elements, to be read as vec2. In the shader, a user would make a `vec2<f32>`
/// to be able to use them. The conversion between `f16` and `f32` will need to
/// be handled by us (using unpack functions).
///
/// To be clear, there won't be types such as `f16` or `u8` anywhere in WGSL
/// code, but these are types that the data may arrive as. We need to convert
/// these smaller types into the base types such as `f32` and `u32` for the
/// shader to use.
class VertexPulling : public Transform {
 public:
  /// Configuration options for the transform
  struct Config : public Castable<Config, Data> {
    /// Constructor
    Config();

    /// Copy constructor
    Config(const Config&);

    /// Destructor
    ~Config() override;

    /// Assignment operator
    /// @returns this Config
    Config& operator=(const Config&);

    /// The entry point to add assignments into
    std::string entry_point_name;

    /// The vertex state descriptor, containing info about attributes
    VertexStateDescriptor vertex_state;

    /// The "group" we will put all our vertex buffers into (as storage buffers)
    /// Default to 4 as it is past the limits of user-accessible groups
    uint32_t pulling_group = 4u;
  };

  /// Constructor
  VertexPulling();

  /// Destructor
  ~VertexPulling() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific input data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;

 private:
  Config cfg_;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_VERTEX_PULLING_H_
