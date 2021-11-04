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
#include <unordered_map>
#include <vector>

#include "src/inspector/entry_point.h"
#include "src/inspector/resource_binding.h"
#include "src/inspector/sampler_texture_pair.h"
#include "src/inspector/scalar.h"
#include "src/program.h"
#include "src/utils/unique_vector.h"

namespace tint {
namespace inspector {

/// Extracts information from a program
class Inspector {
 public:
  /// Constructor
  /// @param program Shader program to extract information from.
  explicit Inspector(const Program* program);

  /// Destructor
  ~Inspector();

  /// @returns error messages from the Inspector
  std::string error() { return diagnostics_.str(); }
  /// @returns true if an error was encountered
  bool has_error() const { return diagnostics_.contains_errors(); }

  /// @returns vector of entry point information
  std::vector<EntryPoint> GetEntryPoints();

  /// @param entry_point name of the entry point to get the remapped version of
  /// @returns the remapped name of the entry point, or the empty string if it
  ///          isn't a known entry point.
  std::string GetRemappedNameForEntryPoint(const std::string& entry_point);

  /// @returns map of const_id to initial value
  std::map<uint32_t, Scalar> GetConstantIDs();

  /// @returns map of module-constant name to pipeline constant ID
  std::map<std::string, uint32_t> GetConstantNameToIdMap();

  /// @param entry_point name of the entry point to get information about.
  /// @returns the total size of shared storage required by an entry point,
  ///          including all uniform storage buffers.
  uint32_t GetStorageSize(const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the resource bindings.
  std::vector<ResourceBinding> GetResourceBindings(
      const std::string& entry_point);

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

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for write-only storage textures.
  std::vector<ResourceBinding> GetWriteOnlyStorageTextureResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for depth textures.
  std::vector<ResourceBinding> GetDepthTextureResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for depth textures.
  std::vector<ResourceBinding> GetDepthMultisampledTextureResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for external textures.
  std::vector<ResourceBinding> GetExternalTextureResourceBindings(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the sampler/texture sampling pairs that are used
  /// by that entry point.
  std::vector<SamplerTexturePair> GetSamplerTextureUses(
      const std::string& entry_point);

  /// @param entry_point name of the entry point to get information about.
  /// @returns the total size in bytes of all Workgroup storage-class storage
  /// referenced transitively by the entry point.
  uint32_t GetWorkgroupStorageSize(const std::string& entry_point);

 private:
  const Program* program_;
  diag::List diagnostics_;
  std::unique_ptr<
      std::unordered_map<std::string, utils::UniqueVector<SamplerTexturePair>>>
      sampler_targets_;

  /// @param name name of the entry point to find
  /// @returns a pointer to the entry point if it exists, otherwise returns
  ///          nullptr and sets the error string.
  const ast::Function* FindEntryPointByName(const std::string& name);

  /// Recursively add entry point IO variables.
  /// If `type` is a struct, recurse into members, appending the member name.
  /// Otherwise, add the variable unless it is a builtin.
  /// @param name the name of the variable being added
  /// @param type the type of the variable
  /// @param decorations the variable decorations
  /// @param variables the list to add the variables to
  void AddEntryPointInOutVariables(std::string name,
                                   const sem::Type* type,
                                   const ast::DecorationList& decorations,
                                   std::vector<StageVariable>& variables) const;

  /// Recursively determine if the type contains builtin.
  /// If `type` is a struct, recurse into members to check for the decoration.
  /// Otherwise, check `decorations` for the decoration.
  bool ContainsBuiltin(ast::Builtin builtin,
                       const sem::Type* type,
                       const ast::DecorationList& decorations) const;

  /// Gathers all the texture resource bindings of the given type for the given
  /// entry point.
  /// @param entry_point name of the entry point to get information about.
  /// @param texture_type the type of the textures to gather.
  /// @param resource_type the ResourceBinding::ResourceType for the given
  /// texture type.
  /// @returns vector of all of the bindings for depth textures.
  std::vector<ResourceBinding> GetTextureResourceBindings(
      const std::string& entry_point,
      const tint::TypeInfo& texture_type,
      ResourceBinding::ResourceType resource_type);

  /// @param entry_point name of the entry point to get information about.
  /// @param read_only if true get only read-only bindings, if false get
  ///                  write-only bindings.
  /// @returns vector of all of the bindings for the requested storage buffers.
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

  /// @param entry_point name of the entry point to get information about.
  /// @returns vector of all of the bindings for the requested storage textures.
  std::vector<ResourceBinding> GetStorageTextureResourceBindingsImpl(
      const std::string& entry_point);

  /// Constructs |sampler_targets_| if it hasn't already been instantiated.
  void GenerateSamplerTargets();

  /// For a N-uple of expressions, resolve to the appropriate global resources
  /// and call 'cb'.
  /// 'cb' may be called multiple times.
  /// Assumes that not being able to resolve the resources is an error, so will
  /// invoke TINT_ICE when that occurs.
  /// @tparam N number of expressions in the n-uple
  /// @tparam F type of the callback provided.
  /// @param exprs N-uple of expressions to resolve.
  /// @param cb is a callback function with the signature:
  /// `void(std::array<const sem::GlobalVariable*, N>)`, which is invoked
  /// whenever a set of expressions are resolved to globals.
  template <size_t N, typename F>
  void GetOriginatingResources(std::array<const ast::Expression*, N> exprs,
                               F&& cb);
};

}  // namespace inspector
}  // namespace tint

#endif  // SRC_INSPECTOR_INSPECTOR_H_
