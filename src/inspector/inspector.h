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

#ifndef SRC_INSPECTOR_INSPECTOR_H_
#define SRC_INSPECTOR_INSPECTOR_H_

#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "src/ast/pipeline_stage.h"
#include "src/inspector/entry_point.h"
#include "src/inspector/scalar.h"
#include "src/program.h"

namespace tint {
namespace inspector {

/// Container for information about how a resource is bound
struct ResourceBinding {
  /// The dimensionality of a texture
  enum class TextureDimension {
    /// Invalid texture
    kNone = -1,
    /// 1 dimensional texture
    k1d,
    /// 1 dimenstional array texture
    k1dArray,
    /// 2 dimensional texture
    k2d,
    /// 2 dimensional array texture
    k2dArray,
    /// 3 dimensional texture
    k3d,
    /// cube texture
    kCube,
    /// cube array texture
    kCubeArray,
  };

  /// Component type of the texture's data. Same as the Sampled Type parameter
  /// in SPIR-V OpTypeImage.
  enum class SampledKind { kUnknown = -1, kFloat, kUInt, kSInt };

  /// Bind group the binding belongs
  uint32_t bind_group;
  /// Identifier to identify this binding within the bind group
  uint32_t binding;
  /// Minimum size required for this binding, in bytes, if defined.
  uint64_t min_buffer_binding_size;
  /// Dimensionality of this binding, if defined.
  TextureDimension dim;
  /// Kind of data being sampled, if defined.
  SampledKind sampled_kind;
};

/// Extracts information from a program
class Inspector {
 public:
  /// Constructor
  /// @param program Shader program to extract information from.
  explicit Inspector(const Program* program);

  /// Destructor
  ~Inspector();

  /// @returns error messages from the Inspector
  const std::string& error() { return error_; }
  /// @returns true if an error was encountered
  bool has_error() const { return !error_.empty(); }

  /// @returns vector of entry point information
  std::vector<EntryPoint> GetEntryPoints();

  /// @param entry_point name of the entry point to get the remapped version of
  /// @returns the remapped name of the entry point, or the empty string if it
  ///          isn't a known entry point.
  std::string GetRemappedNameForEntryPoint(const std::string& entry_point);

  /// @returns map of const_id to initial value
  std::map<uint32_t, Scalar> GetConstantIDs();

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for uniform buffers.
  std::vector<ResourceBinding> GetUniformBufferResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for storage buffers.
  std::vector<ResourceBinding> GetStorageBufferResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for read-only storage buffers.
  std::vector<ResourceBinding> GetReadOnlyStorageBufferResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for regular samplers.
  std::vector<ResourceBinding> GetSamplerResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for comparison samplers.
  std::vector<ResourceBinding> GetComparisonSamplerResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for sampled textures.
  std::vector<ResourceBinding> GetSampledTextureResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for multisampled textures.
  std::vector<ResourceBinding> GetMultisampledTextureResourceBindings(
      const std::string& entry_point);

 private:
  const Program* program_;
  std::string error_;

  /// @param name name of the entry point to find
  /// @returns a pointer to the entry point if it exists, otherwise returns
  ///          nullptr and sets the error string.
  ast::Function* FindEntryPointByName(const std::string& name);

  /// @param entry_point name of the entry point to get information about.
  /// @param read_only get only read only if true, otherwise get everything
  ///                  else.
  /// @returns vector of all of the bindings for the request storage buffers.
  std::vector<ResourceBinding> GetStorageBufferResourceBindingsImpl(
      const std::string& entry_point,
      bool read_only);

  /// @param entry_point name of the entry point to get information about.
  /// @param multisampled_only only get multisampled textures if true, otherwise
  ///                          only get sampled textures.
  /// @returns vector of all of the bindings for the request storage buffers.
  std::vector<ResourceBinding> GetSampledTextureResourceBindingsImpl(
      const std::string& entry_point,
      bool multisampled_only);
};

}  // namespace inspector
}  // namespace tint

#endif  // SRC_INSPECTOR_INSPECTOR_H_
