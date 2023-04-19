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

#ifndef SRC_TINT_TRANSFORM_VERTEX_PULLING_H_
#define SRC_TINT_TRANSFORM_VERTEX_PULLING_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/tint/reflection.h"
#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Describes the format of data in a vertex buffer
enum class VertexFormat {
    kUint8x2,    // uint8x2
    kUint8x4,    // uint8x4
    kSint8x2,    // sint8x2
    kSint8x4,    // sint8x4
    kUnorm8x2,   // unorm8x2
    kUnorm8x4,   // unorm8x4
    kSnorm8x2,   // snorm8x2
    kSnorm8x4,   // snorm8x4
    kUint16x2,   // uint16x2
    kUint16x4,   // uint16x4
    kSint16x2,   // sint16x2
    kSint16x4,   // sint16x4
    kUnorm16x2,  // unorm16x2
    kUnorm16x4,  // unorm16x4
    kSnorm16x2,  // snorm16x2
    kSnorm16x4,  // snorm16x4
    kFloat16x2,  // float16x2
    kFloat16x4,  // float16x4
    kFloat32,    // float32
    kFloat32x2,  // float32x2
    kFloat32x3,  // float32x3
    kFloat32x4,  // float32x4
    kUint32,     // uint32
    kUint32x2,   // uint32x2
    kUint32x3,   // uint32x3
    kUint32x4,   // uint32x4
    kSint32,     // sint32
    kSint32x2,   // sint32x2
    kSint32x3,   // sint32x3
    kSint32x4,   // sint32x4

    kLastEntry = kSint32x4,
};

/// Describes if a vertex attributes increments with vertex index or instance
/// index
enum class VertexStepMode { kVertex, kInstance, kLastEntry = kInstance };

/// Describes a vertex attribute within a buffer
struct VertexAttributeDescriptor {
    /// The format of the attribute
    VertexFormat format;
    /// The byte offset of the attribute in the buffer
    uint32_t offset;
    /// The shader location used for the attribute
    uint32_t shader_location;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(format, offset, shader_location);
};

/// Describes a buffer containing multiple vertex attributes
struct VertexBufferLayoutDescriptor {
    /// Constructor
    VertexBufferLayoutDescriptor();
    /// Constructor
    /// @param in_array_stride the array stride of the in buffer
    /// @param in_step_mode the step mode of the in buffer
    /// @param in_attributes the in attributes
    VertexBufferLayoutDescriptor(uint32_t in_array_stride,
                                 VertexStepMode in_step_mode,
                                 std::vector<VertexAttributeDescriptor> in_attributes);
    /// Copy constructor
    /// @param other the struct to copy
    VertexBufferLayoutDescriptor(const VertexBufferLayoutDescriptor& other);

    /// Assignment operator
    /// @param other the struct to copy
    /// @returns this struct
    VertexBufferLayoutDescriptor& operator=(const VertexBufferLayoutDescriptor& other);

    ~VertexBufferLayoutDescriptor();

    /// The array stride used in the in buffer
    uint32_t array_stride = 0u;
    /// The input step mode used
    VertexStepMode step_mode = VertexStepMode::kVertex;
    /// The vertex attributes
    std::vector<VertexAttributeDescriptor> attributes;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(array_stride, step_mode, attributes);
};

/// Describes vertex state, which consists of many buffers containing vertex
/// attributes
using VertexStateDescriptor = std::vector<VertexBufferLayoutDescriptor>;

/// Converts a program to use vertex pulling
///
/// Variables which accept vertex input are var<in> with a location attribute.
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
///
/// The SingleEntryPoint transform must have run before VertexPulling.
class VertexPulling final : public utils::Castable<VertexPulling, Transform> {
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

        /// The vertex state descriptor, containing info about attributes
        VertexStateDescriptor vertex_state;

        /// The "group" we will put all our vertex buffers into (as storage buffers)
        /// Default to 4 as it is past the limits of user-accessible groups
        uint32_t pulling_group = 4u;

        /// Reflect the fields of this class so that it can be used by tint::ForeachField()
        TINT_REFLECT(vertex_state, pulling_group);
    };

    /// Constructor
    VertexPulling();

    /// Destructor
    ~VertexPulling() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;

    Config cfg_;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_VERTEX_PULLING_H_
