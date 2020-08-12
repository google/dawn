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

#ifndef SRC_AST_TRANSFORM_VERTEX_PULLING_TRANSFORM_H_
#define SRC_AST_TRANSFORM_VERTEX_PULLING_TRANSFORM_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace tint {

class Context;

namespace ast {

class EntryPoint;
class Expression;
class Function;
class Module;
class Statement;
class Variable;

namespace type {
class Type;
}  // namespace type

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
  kVec4I32
};

/// Describes if a vertex attribtes increments with vertex index or instance
/// index
enum class InputStepMode { kVertex, kInstance };

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
struct VertexStateDescriptor {
  /// Constructor
  VertexStateDescriptor();
  /// Constructor
  /// @param in_vertex_buffers the vertex buffers
  VertexStateDescriptor(
      std::vector<VertexBufferLayoutDescriptor> in_vertex_buffers);
  /// Copy constructor
  /// @param other the struct to copy
  VertexStateDescriptor(const VertexStateDescriptor& other);
  ~VertexStateDescriptor();

  /// The vertex buffers
  std::vector<VertexBufferLayoutDescriptor> vertex_buffers;
};

/// Converts a module to use vertex pulling
///
/// Variables which accept vertex input are var<in> with a location decoration.
/// This transform will convert those to be assigned from storage buffers
/// instead. The intention is to allow vertex input to rely on a storage buffer
/// clamping pass for out of bounds reads. We bind the storage buffers as arrays
/// of u32, so any read to byte position |p| will actually need to read position
/// |p / 4|, since sizeof(u32) == 4.
///
/// |VertexFormat| represents the input type of the attribute. This isn't
/// related to the type of the variable in the shader. For example,
/// `VertexFormat::kVec2F16` tells us that the buffer will contain f16 elements,
/// to be read as vec2. In the shader, a user would make a vec2<f32> to be able
/// to use them. The conversion between f16 and f32 will need to be handled by
/// us (using unpack functions).
///
/// To be clear, there won't be types such as f16 or u8 anywhere in WGSL code,
/// but these are types that the data may arrive as. We need to convert these
/// smaller types into the base types such as f32 and u32 for the shader to use.
class VertexPullingTransform {
 public:
  /// Constructor
  /// @param ctx the tint context
  /// @param mod the module to convert to vertex pulling
  VertexPullingTransform(Context* ctx, Module* mod);
  ~VertexPullingTransform();

  /// Sets the vertex state descriptor, containing info about attributes
  /// @param vertex_state the vertex state descriptor
  void SetVertexState(std::unique_ptr<VertexStateDescriptor> vertex_state);

  /// Sets the entry point to add assignments into
  /// @param entry_point the vertex stage entry point
  void SetEntryPoint(std::string entry_point);

  /// Sets the "set" we will put all our vertex buffers into (as storage
  /// buffers)
  /// @param number the set number we will use
  void SetPullingBufferBindingSet(uint32_t number);

  /// @returns true if the transformation was successful
  bool Run();

  /// @returns error messages
  const std::string& GetError() { return error_; }

 private:
  void SetError(const std::string& error);

  /// Generate the vertex buffer binding name
  /// @param index index to append to buffer name
  std::string GetVertexBufferName(uint32_t index);

  /// Inserts vertex_idx binding, or finds the existing one
  void FindOrInsertVertexIndexIfUsed();

  /// Inserts instance_idx binding, or finds the existing one
  void FindOrInsertInstanceIndexIfUsed();

  /// Converts var<in> with a location decoration to var<private>
  void ConvertVertexInputVariablesToPrivate();

  /// Adds storage buffer decorated variables for the vertex buffers
  void AddVertexStorageBuffers();

  /// Adds assignment to the variables from the buffers
  void AddVertexPullingPreamble(Function* vertex_func);

  /// Generates an expression holding a constant uint
  /// @param value uint value
  std::unique_ptr<Expression> GenUint(uint32_t value);

  /// Generates an expression to read the shader value |kPullingPosVarName|
  std::unique_ptr<Expression> CreatePullingPositionIdent();

  /// Generates an expression reading from a buffer a specific format.
  /// This reads the value wherever |kPullingPosVarName| points to at the time
  /// of the read.
  /// @param buffer the index of the vertex buffer
  /// @param format the format to read
  std::unique_ptr<Expression> AccessByFormat(uint32_t buffer,
                                             VertexFormat format);

  /// Generates an expression reading a uint32 from a vertex buffer
  /// @param buffer the index of the vertex buffer
  /// @param pos an expression for the position of the access, in bytes
  std::unique_ptr<Expression> AccessU32(uint32_t buffer,
                                        std::unique_ptr<Expression> pos);

  /// Generates an expression reading an int32 from a vertex buffer
  /// @param buffer the index of the vertex buffer
  /// @param pos an expression for the position of the access, in bytes
  std::unique_ptr<Expression> AccessI32(uint32_t buffer,
                                        std::unique_ptr<Expression> pos);

  /// Generates an expression reading a float from a vertex buffer
  /// @param buffer the index of the vertex buffer
  /// @param pos an expression for the position of the access, in bytes
  std::unique_ptr<Expression> AccessF32(uint32_t buffer,
                                        std::unique_ptr<Expression> pos);

  /// Generates an expression reading a basic type (u32, i32, f32) from a vertex
  /// buffer
  /// @param buffer the index of the vertex buffer
  /// @param pos an expression for the position of the access, in bytes
  /// @param format the underlying vertex format
  std::unique_ptr<Expression> AccessPrimitive(uint32_t buffer,
                                              std::unique_ptr<Expression> pos,
                                              VertexFormat format);

  /// Generates an expression reading a vec2/3/4 from a vertex buffer.
  /// This reads the value wherever |kPullingPosVarName| points to at the time
  /// of the read.
  /// @param buffer the index of the vertex buffer
  /// @param element_stride stride between elements, in bytes
  /// @param base_type underlying AST type
  /// @param base_format underlying vertex format
  /// @param count how many elements the vector has
  std::unique_ptr<Expression> AccessVec(uint32_t buffer,
                                        uint32_t element_stride,
                                        type::Type* base_type,
                                        VertexFormat base_format,
                                        uint32_t count);

  // Used to grab corresponding types from the type manager
  type::Type* GetU32Type();
  type::Type* GetI32Type();
  type::Type* GetF32Type();

  Context* ctx_ = nullptr;
  Module* mod_ = nullptr;
  std::string entry_point_name_;
  std::string error_;

  std::string vertex_index_name_;
  std::string instance_index_name_;

  // Default to 4 as it is past the limits of user-accessible sets
  uint32_t pulling_set_ = 4u;

  std::unordered_map<uint32_t, Variable*> location_to_var_;
  std::unique_ptr<VertexStateDescriptor> vertex_state_;
};

}  // namespace transform
}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_TRANSFORM_VERTEX_PULLING_TRANSFORM_H_
