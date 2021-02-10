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

#include "src/intrinsic_table.h"

#include <algorithm>
#include <limits>
#include <string>
#include <unordered_map>
#include <utility>

#include "src/block_allocator.h"
#include "src/program_builder.h"
#include "src/semantic/intrinsic.h"
#include "src/type/access_control_type.h"
#include "src/type/depth_texture_type.h"
#include "src/type/f32_type.h"
#include "src/type/multisampled_texture_type.h"
#include "src/type/sampled_texture_type.h"
#include "src/type/storage_texture_type.h"

namespace tint {
namespace {

/// OpenTypes are the symbols used for templated types in overload signatures
enum class OpenType {
  T,
  Count,  // Number of entries in the enum. Not a usable symbol.
};

/// OpenNumber are the symbols used for templated integers in overload
/// signatures
enum class OpenNumber {
  N,  // Typically used for vecN
  M,  // Typically used for matNxM
  F,  // Typically used for texture_storage_2d<F>
};

/// @return a string of the OpenType symbol `ty`
const char* str(OpenType ty) {
  switch (ty) {
    case OpenType::T:
      return "T";

    case OpenType::Count:
      break;
  }
  return "";
}

/// @return a string of the OpenNumber symbol `num`
const char* str(OpenNumber num) {
  switch (num) {
    case OpenNumber::N:
      return "N";
    case OpenNumber::M:
      return "M";
    case OpenNumber::F:
      return "F";
  }
  return "";
}

/// A Matcher is an interface of a class used to match an overload parameter,
/// return type, or open type.
class Matcher {
 public:
  /// Current state passed to Match()
  struct MatchState {
    /// The map of open types. A new entry is assigned the first time an
    /// OpenType is encountered. If the OpenType is encountered again, a
    /// comparison is made to see if the type is consistent.
    std::unordered_map<OpenType, type::Type*> open_types;
    /// The map of open numbers. A new entry is assigned the first time an
    /// OpenNumber is encountered. If the OpenNumber is encountered again, a
    /// comparison is made to see if the number is consistent.
    std::unordered_map<OpenNumber, uint32_t> open_numbers;
  };

  /// Destructor
  virtual ~Matcher() = default;

  /// Checks whether the given argument type matches.
  /// Aliases are automatically unwrapped before matching.
  /// Match may add to, or compare against the open types and numbers in state.
  /// @returns true if the argument type is as expected.
  bool Match(MatchState& state, type::Type* argument_type) const {
    auto* unwrapped = argument_type;
    while (auto* alias = unwrapped->As<type::Alias>()) {
      unwrapped = alias->type();
    }
    return MatchUnwrapped(state, unwrapped);
  }

  /// @return true if the matcher is expecting a pointer. If this method returns
  /// false and the argument is a pointer type, then the argument should be
  /// dereferenced before calling.
  virtual bool ExpectsPointer() const { return false; }

  /// @return a string representation of the matcher. Used for printing error
  /// messages when no overload is found.
  virtual std::string str() const = 0;

 protected:
  /// Checks whether the given alias-unwrapped argument type matches.
  /// Match may add to, or compare against the open types and numbers in state.
  /// @returns true if the argument type is as expected.
  virtual bool MatchUnwrapped(MatchState& state,
                              type::Type* argument_type) const = 0;

  /// Checks `state.open_type` to see if the OpenType `t` is equal to the type
  /// `ty`. If `state.open_type` does not contain an entry for `t`, then `ty`
  /// is added and returns true.
  bool MatchOpenType(MatchState& state, OpenType t, type::Type* ty) const {
    auto it = state.open_types.find(t);
    if (it != state.open_types.end()) {
      return it->second == ty;
    }
    state.open_types[t] = ty;
    return true;
  }

  /// Checks `state.open_numbers` to see if the OpenNumber `n` is equal to
  /// `val`. If `state.open_numbers` does not contain an entry for `n`, then
  /// `val` is added and returns true.
  bool MatchOpenNumber(MatchState& state, OpenNumber n, uint32_t val) const {
    auto it = state.open_numbers.find(n);
    if (it != state.open_numbers.end()) {
      return it->second == val;
    }
    state.open_numbers[n] = val;
    return true;
  }
};

/// Builder is an extension of the Matcher interface that can also build the
/// expected type. Builders are used to generate the parameter and return types
/// on successful overload match.
class Builder : public Matcher {
 public:
  /// Final matched state passed to Build()
  struct BuildState {
    /// The type manager used to construct new types
    type::Manager& ty_mgr;
    /// The final resolved list of open types
    std::unordered_map<OpenType, type::Type*> const open_types;
    /// The final resolved list of open numbers
    std::unordered_map<OpenNumber, uint32_t> const open_numbers;
  };

  /// Destructor
  ~Builder() override = default;

  /// Constructs and returns the expected type
  virtual type::Type* Build(BuildState& state) const = 0;
};

/// OpenTypeBuilder is a Matcher / Builder for an open type (T etc).
/// The OpenTypeBuilder will match against any type (so long as it is consistent
/// for the overload), and Build() will build the type it matched against.
class OpenTypeBuilder : public Builder {
 public:
  explicit OpenTypeBuilder(OpenType open_type) : open_type_(open_type) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    return MatchOpenType(state, open_type_, ty);
  }

  type::Type* Build(BuildState& state) const override {
    return state.open_types.at(open_type_);
  }

  std::string str() const override { return tint::str(open_type_); }

 private:
  OpenType open_type_;
};

/// VoidBuilder is a Matcher / Builder for void types.
class VoidBuilder : public Builder {
 public:
  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    return ty->Is<type::Void>();
  }
  type::Type* Build(BuildState& state) const override {
    return state.ty_mgr.Get<type::Void>();
  }
  std::string str() const override { return "void"; }
};

/// BoolBuilder is a Matcher / Builder for boolean types.
class BoolBuilder : public Builder {
 public:
  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    return ty->Is<type::Bool>();
  }
  type::Type* Build(BuildState& state) const override {
    return state.ty_mgr.Get<type::Bool>();
  }
  std::string str() const override { return "bool"; }
};

/// F32Builder is a Matcher / Builder for f32 types.
class F32Builder : public Builder {
 public:
  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    return ty->Is<type::F32>();
  }
  type::Type* Build(BuildState& state) const override {
    return state.ty_mgr.Get<type::F32>();
  }
  std::string str() const override { return "f32"; }
};

/// U32Builder is a Matcher / Builder for u32 types.
class U32Builder : public Builder {
 public:
  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    return ty->Is<type::U32>();
  }
  type::Type* Build(BuildState& state) const override {
    return state.ty_mgr.Get<type::U32>();
  }
  std::string str() const override { return "u32"; }
};

/// I32Builder is a Matcher / Builder for i32 types.
class I32Builder : public Builder {
 public:
  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    return ty->Is<type::I32>();
  }
  type::Type* Build(BuildState& state) const override {
    return state.ty_mgr.Get<type::I32>();
  }
  std::string str() const override { return "i32"; }
};

/// IU32Matcher is a Matcher for i32 or u32 types.
class IU32Matcher : public Matcher {
 public:
  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    return ty->Is<type::I32>() || ty->Is<type::U32>();
  }
  std::string str() const override { return "i32 or u32"; }
};

/// FIU32Matcher is a Matcher for f32, i32 or u32 types.
class FIU32Matcher : public Matcher {
 public:
  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    return ty->Is<type::F32>() || ty->Is<type::I32>() || ty->Is<type::U32>();
  }
  std::string str() const override { return "f32, i32 or u32"; }
};

/// ScalarMatcher is a Matcher for f32, i32, u32 or boolean types.
class ScalarMatcher : public Matcher {
 public:
  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    return ty->is_scalar();
  }
  std::string str() const override { return "scalar"; }
};

/// OpenSizeVecBuilder is a Matcher / Builder for vector types of an open number
/// size.
class OpenSizeVecBuilder : public Builder {
 public:
  OpenSizeVecBuilder(OpenNumber size, Builder* element_builder)
      : size_(size), element_builder_(element_builder) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    if (auto* vec = ty->As<type::Vector>()) {
      if (!MatchOpenNumber(state, size_, vec->size())) {
        return false;
      }
      return element_builder_->Match(state, vec->type());
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    auto* el = element_builder_->Build(state);
    auto n = state.open_numbers.at(size_);
    return state.ty_mgr.Get<type::Vector>(el, n);
  }

  std::string str() const override {
    return "vec" + std::string(tint::str(size_)) + "<" +
           element_builder_->str() + ">";
  }

 protected:
  OpenNumber const size_;
  Builder* const element_builder_;
};

/// VecBuilder is a Matcher / Builder for vector types of a fixed size.
class VecBuilder : public Builder {
 public:
  VecBuilder(uint32_t size, Builder* element_builder)
      : size_(size), element_builder_(element_builder) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    if (auto* vec = ty->As<type::Vector>()) {
      if (vec->size() == size_) {
        return element_builder_->Match(state, vec->type());
      }
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    auto* el = element_builder_->Build(state);
    return state.ty_mgr.Get<type::Vector>(el, size_);
  }

  std::string str() const override {
    return "vec" + std::to_string(size_) + "<" + element_builder_->str() + ">";
  }

 protected:
  const uint32_t size_;
  Builder* const element_builder_;
};

/// OpenSizeVecBuilder is a Matcher / Builder for matrix types of an open number
/// column and row size.
class OpenSizeMatBuilder : public Builder {
 public:
  OpenSizeMatBuilder(OpenNumber columns,
                     OpenNumber rows,
                     Builder* element_builder)
      : columns_(columns), rows_(rows), element_builder_(element_builder) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    if (auto* mat = ty->As<type::Matrix>()) {
      if (!MatchOpenNumber(state, columns_, mat->columns())) {
        return false;
      }
      if (!MatchOpenNumber(state, rows_, mat->rows())) {
        return false;
      }
      return element_builder_->Match(state, mat->type());
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    auto* el = element_builder_->Build(state);
    auto columns = state.open_numbers.at(columns_);
    auto rows = state.open_numbers.at(rows_);
    return state.ty_mgr.Get<type::Matrix>(el, rows, columns);
  }

  std::string str() const override {
    return "mat" + std::string(tint::str(columns_)) + "x" +
           std::string(tint::str(rows_)) + "<" + element_builder_->str() + ">";
  }

 protected:
  OpenNumber const columns_;
  OpenNumber const rows_;
  Builder* const element_builder_;
};

/// PtrBuilder is a Matcher / Builder for pointer types.
class PtrBuilder : public Builder {
 public:
  explicit PtrBuilder(Builder* element_builder)
      : element_builder_(element_builder) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    if (auto* ptr = ty->As<type::Pointer>()) {
      return element_builder_->Match(state, ptr->type());
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    auto* el = element_builder_->Build(state);
    return state.ty_mgr.Get<type::Pointer>(el, ast::StorageClass::kNone);
  }

  bool ExpectsPointer() const override { return true; }

  std::string str() const override {
    return "ptr<" + element_builder_->str() + ">";
  }

 private:
  Builder* const element_builder_;
};

/// ArrayBuilder is a Matcher / Builder for runtime sized array types.
class ArrayBuilder : public Builder {
 public:
  explicit ArrayBuilder(Builder* element_builder)
      : element_builder_(element_builder) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    if (auto* arr = ty->As<type::Array>()) {
      if (arr->size() == 0) {
        return element_builder_->Match(state, arr->type());
      }
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    auto* el = element_builder_->Build(state);
    return state.ty_mgr.Get<type::Array>(el, 0, ast::ArrayDecorationList{});
  }

  std::string str() const override {
    return "array<" + element_builder_->str() + ">";
  }

 private:
  Builder* const element_builder_;
};

/// SampledTextureBuilder is a Matcher / Builder for sampled texture types.
class SampledTextureBuilder : public Builder {
 public:
  explicit SampledTextureBuilder(type::TextureDimension dimensions,
                                 Builder* type_builder)
      : dimensions_(dimensions), type_builder_(type_builder) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    if (auto* tex = ty->As<type::SampledTexture>()) {
      if (tex->dim() == dimensions_) {
        return type_builder_->Match(state, tex->type());
      }
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    auto* type = type_builder_->Build(state);
    return state.ty_mgr.Get<type::SampledTexture>(dimensions_, type);
  }

  std::string str() const override {
    std::stringstream ss;
    ss << "texture_" << dimensions_ << "<" << type_builder_->str() << ">";
    return ss.str();
  }

 private:
  type::TextureDimension const dimensions_;
  Builder* const type_builder_;
};

/// MultisampledTextureBuilder is a Matcher / Builder for multisampled texture
/// types.
class MultisampledTextureBuilder : public Builder {
 public:
  explicit MultisampledTextureBuilder(type::TextureDimension dimensions,
                                      Builder* type_builder)
      : dimensions_(dimensions), type_builder_(type_builder) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    if (auto* tex = ty->As<type::MultisampledTexture>()) {
      if (tex->dim() == dimensions_) {
        return type_builder_->Match(state, tex->type());
      }
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    auto* type = type_builder_->Build(state);
    return state.ty_mgr.Get<type::MultisampledTexture>(dimensions_, type);
  }

  std::string str() const override {
    std::stringstream ss;
    ss << "texture_multisampled_" << dimensions_ << "<" << type_builder_->str()
       << ">";
    return ss.str();
  }

 private:
  type::TextureDimension const dimensions_;
  Builder* const type_builder_;
};

/// DepthTextureBuilder is a Matcher / Builder for depth texture types.
class DepthTextureBuilder : public Builder {
 public:
  explicit DepthTextureBuilder(type::TextureDimension dimensions)
      : dimensions_(dimensions) {}

  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    if (auto* tex = ty->As<type::DepthTexture>()) {
      return tex->dim() == dimensions_;
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    return state.ty_mgr.Get<type::DepthTexture>(dimensions_);
  }

  std::string str() const override {
    std::stringstream ss;
    ss << "texture_depth_" << dimensions_;
    return ss.str();
  }

 private:
  type::TextureDimension const dimensions_;
};

/// StorageTextureBuilder is a Matcher / Builder for storage texture types of
/// the given texel and channel formats.
class StorageTextureBuilder : public Builder {
 public:
  explicit StorageTextureBuilder(
      type::TextureDimension dimensions,
      OpenNumber texel_format,  // a.k.a "image format"
      OpenType channel_format)  // a.k.a "storage subtype"
      : dimensions_(dimensions),
        texel_format_(texel_format),
        channel_format_(channel_format) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    if (auto* ac = ty->As<type::AccessControl>()) {
      // If we have an storage texture argument that's got an access control
      // type wrapped around it, accept it. Signatures that don't include an
      // access control imply any access. Example:
      //   textureDimensions(t : texture_storage_1d<F>) -> i32
      ty = ac->type();
    }

    if (auto* tex = ty->As<type::StorageTexture>()) {
      if (MatchOpenNumber(state, texel_format_,
                          static_cast<uint32_t>(tex->image_format()))) {
        if (MatchOpenType(state, channel_format_, tex->type())) {
          return tex->dim() == dimensions_;
        }
      }
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    auto texel_format =
        static_cast<type::ImageFormat>(state.open_numbers.at(texel_format_));
    auto* channel_format = state.open_types.at(channel_format_);
    return state.ty_mgr.Get<type::StorageTexture>(dimensions_, texel_format,
                                                  channel_format);
  }

  std::string str() const override {
    std::stringstream ss;
    ss << "texture_storage_" << dimensions_ << "<F>";
    return ss.str();
  }

 private:
  type::TextureDimension const dimensions_;
  OpenNumber const texel_format_;
  OpenType const channel_format_;
};

/// SamplerBuilder is a Matcher / Builder for sampler types of the given kind.
class SamplerBuilder : public Builder {
 public:
  explicit SamplerBuilder(type::SamplerKind kind) : kind_(kind) {}

  bool MatchUnwrapped(MatchState&, type::Type* ty) const override {
    if (auto* sampler = ty->As<type::Sampler>()) {
      return sampler->kind() == kind_;
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    return state.ty_mgr.Get<type::Sampler>(kind_);
  }

  std::string str() const override {
    switch (kind_) {
      case type::SamplerKind::kSampler:
        return "sampler";
      case type::SamplerKind::kComparisonSampler:
        return "sampler_comparison";
    }
    return "sampler";
  }

 private:
  type::SamplerKind const kind_;
};

/// AccessControlBuilder is a Matcher / Builder for AccessControl types
class AccessControlBuilder : public Builder {
 public:
  explicit AccessControlBuilder(ast::AccessControl access_control,
                                Builder* type)
      : access_control_(access_control), type_(type) {}

  bool MatchUnwrapped(MatchState& state, type::Type* ty) const override {
    if (auto* ac = ty->As<type::AccessControl>()) {
      if (ac->access_control() == access_control_) {
        return type_->Match(state, ty);
      }
    }
    return false;
  }

  type::Type* Build(BuildState& state) const override {
    auto* ty = type_->Build(state);
    return state.ty_mgr.Get<type::AccessControl>(access_control_, ty);
  }

  std::string str() const override {
    std::stringstream ss;
    ss << "[[access(" << access_control_ << ")]] " << type_->str();
    return ss.str();
  }

 private:
  ast::AccessControl const access_control_;
  Builder* const type_;
};

/// Impl is the private implementation of the IntrinsicTable interface.
class Impl : public IntrinsicTable {
 public:
  Impl();

  IntrinsicTable::Result Lookup(
      ProgramBuilder& builder,
      semantic::IntrinsicType type,
      const std::vector<type::Type*>& args) const override;

  /// Holds the information about a single overload parameter used for matching
  struct Parameter {
    Parameter(
        Builder* m)  // NOLINT - implicit constructor required for Register()
        : matcher(m) {}
    Parameter(semantic::Parameter::Usage u, Builder* m)
        : matcher(m), usage(u) {}

    Builder* const matcher;
    semantic::Parameter::Usage const usage = semantic::Parameter::Usage::kNone;
  };

  /// A single overload definition.
  struct Overload {
    /// @returns a human readable string representation of the overload
    std::string str() const;

    /// Attempts to match this overload given the IntrinsicType and argument
    /// types. If a match is made, the build intrinsic is returned, otherwise
    /// `match_score` is assigned a score of how closely the overload matched
    /// (positive representing a greater match), and nullptr is returned.
    semantic::Intrinsic* Match(ProgramBuilder& builder,
                               semantic::IntrinsicType type,
                               const std::vector<type::Type*>& arg_types,
                               int& match_score) const;

    semantic::IntrinsicType type;
    Builder* return_type;
    std::vector<Parameter> parameters;
    std::unordered_map<OpenType, Matcher*> open_type_matchers;
  };

 private:
  /// Allocator for the built Matcher / Builders
  BlockAllocator<Matcher> matcher_allocator_;

  /// Commonly used Matcher / Builders
  struct {
    VoidBuilder void_;
    BoolBuilder bool_;
    F32Builder f32;
    I32Builder i32;
    IU32Matcher iu32;
    FIU32Matcher fiu32;
    ScalarMatcher scalar;
    U32Builder u32;
    OpenTypeBuilder T{OpenType::T};
  } matchers_;

  // TODO(bclayton): Sort by type, or array these by IntrinsicType
  std::vector<Overload> overloads_;

  /// @returns a Matcher / Builder that matches a pointer with the given element
  /// type
  Builder* ptr(Builder* element_builder) {
    return matcher_allocator_.Create<PtrBuilder>(element_builder);
  }

  /// @returns a Matcher / Builder that matches a vector of size OpenNumber::N
  /// with the given element type
  Builder* vecN(Builder* element_builder) {
    return matcher_allocator_.Create<OpenSizeVecBuilder>(OpenNumber::N,
                                                         element_builder);
  }

  /// @returns a Matcher / Builder that matches a vector of the given size and
  /// element type
  Builder* vec(uint32_t size, Builder* element_builder) {
    return matcher_allocator_.Create<VecBuilder>(size, element_builder);
  }

  /// @returns a Matcher / Builder that matches a runtime sized array with the
  /// given element type
  Builder* array(Builder* element_builder) {
    return matcher_allocator_.Create<ArrayBuilder>(element_builder);
  }

  /// @returns a Matcher / Builder that matches a matrix with the given size and
  /// element type
  Builder* mat(OpenNumber columns, OpenNumber rows, Builder* element_builder) {
    return matcher_allocator_.Create<OpenSizeMatBuilder>(columns, rows,
                                                         element_builder);
  }

  /// @returns a Matcher / Builder that matches a square matrix with the column
  /// / row count of OpenNumber::N
  template <typename T>
  auto matNxN(T&& in) {
    return mat(OpenNumber::N, OpenNumber::N, std::forward<T>(in));
  }

  /// @returns a Matcher / Builder that matches a sampled texture with the given
  /// dimensions and type
  Builder* sampled_texture(type::TextureDimension dimensions, Builder* type) {
    return matcher_allocator_.Create<SampledTextureBuilder>(dimensions, type);
  }

  /// @returns a Matcher / Builder that matches a multisampled texture with the
  /// given dimensions and type
  Builder* multisampled_texture(type::TextureDimension dimensions,
                                Builder* type) {
    return matcher_allocator_.Create<MultisampledTextureBuilder>(dimensions,
                                                                 type);
  }

  /// @returns a Matcher / Builder that matches a depth texture with the
  /// given dimensions
  Builder* depth_texture(type::TextureDimension dimensions) {
    return matcher_allocator_.Create<DepthTextureBuilder>(dimensions);
  }

  /// @returns a Matcher / Builder that matches a storage texture of the given
  /// format with the given dimensions
  Builder* storage_texture(type::TextureDimension dimensions,
                           OpenNumber texel_format,
                           OpenType channel_format) {
    return matcher_allocator_.Create<StorageTextureBuilder>(
        dimensions, texel_format, channel_format);
  }

  /// @returns a Matcher / Builder that matches a sampler type
  Builder* sampler(type::SamplerKind kind) {
    return matcher_allocator_.Create<SamplerBuilder>(kind);
  }

  /// @returns a Matcher / Builder that matches an access control type
  Builder* access_control(ast::AccessControl access_control, Builder* type) {
    return matcher_allocator_.Create<AccessControlBuilder>(access_control,
                                                           type);
  }

  /// Registers an overload with the given intrinsic type, return type Matcher /
  /// Builder, and parameter Matcher / Builders.
  /// This overload of Register does not constrain any OpenTypes.
  void Register(semantic::IntrinsicType type,
                Builder* return_type,
                std::vector<Parameter> parameters) {
    Overload overload{type, return_type, std::move(parameters), {}};
    overloads_.emplace_back(overload);
  }

  /// Registers an overload with the given intrinsic type, return type Matcher /
  /// Builder, and parameter Matcher / Builders.
  /// A single OpenType is contained with the given Matcher in
  /// open_type_matcher.
  void Register(semantic::IntrinsicType type,
                Builder* return_type,
                std::vector<Parameter> parameters,
                std::pair<OpenType, Matcher*> open_type_matcher) {
    Overload overload{
        type, return_type, std::move(parameters), {open_type_matcher}};
    overloads_.emplace_back(overload);
  }
};

Impl::Impl() {
  using I = semantic::IntrinsicType;
  using Dim = type::TextureDimension;

  auto* void_ = &matchers_.void_;      // void
  auto* bool_ = &matchers_.bool_;      // bool
  auto* f32 = &matchers_.f32;          // f32
  auto* i32 = &matchers_.i32;          // i32
  auto* u32 = &matchers_.u32;          // u32
  auto* iu32 = &matchers_.iu32;        // i32 or u32
  auto* fiu32 = &matchers_.fiu32;      // f32, i32 or u32
  auto* scalar = &matchers_.scalar;    // f32, i32, u32 or bool
  auto* T = &matchers_.T;              // Any T type
  auto* array_T = array(T);            // array<T>
  auto* vec2_f32 = vec(2, f32);        // vec2<f32>
  auto* vec3_f32 = vec(3, f32);        // vec3<f32>
  auto* vec4_f32 = vec(4, f32);        // vec4<f32>
  auto* vec4_T = vec(4, T);            // vec4<T>
  auto* vec2_i32 = vec(2, i32);        // vec2<i32>
  auto* vec3_i32 = vec(3, i32);        // vec3<i32>
  auto* vecN_f32 = vecN(f32);          // vecN<f32>
  auto* vecN_T = vecN(T);              // vecN<T>
  auto* vecN_bool = vecN(bool_);       // vecN<bool>
  auto* matNxN_f32 = matNxN(f32);      // matNxN<f32>
  auto* ptr_T = ptr(T);                // ptr<T>
  auto* ptr_f32 = ptr(f32);            // ptr<f32>
  auto* ptr_vecN_T = ptr(vecN_T);      // ptr<vecN<T>>
  auto* ptr_vecN_f32 = ptr(vecN_f32);  // ptr<vecN<f32>>

  // Intrinsic overloads are registered with a call to the Register().
  //
  // The best way to explain Register() and the lookup process is by example.
  //
  // Let's begin with a simple overload declaration:
  //
  //   Register(I::kIsInf, bool_, {f32});
  //
  //   I     - is an alias to semantic::IntrinsicType.
  //           I::kIsInf is shorthand for semantic::IntrinsicType::kIsInf.
  //   bool_ - is a pointer to a pre-constructed BoolBuilder which matches and
  //           builds type::Bool types.
  //   {f32} - is the list of parameter Builders for the overload.
  //           Builders are a type of Matcher that can also build the the type.
  //           All Builders are Matchers, not all Matchers are Builders.
  //   f32     is a pointer to a pre-constructed F32Builder which matches and
  //           builds type::F32 types.
  //
  // This call registers the overload for the `isInf(f32) -> bool` intrinsic.
  //
  // Let's now see the process of Overload::Match() when passed a single f32
  // argument:
  //
  //   (1) Overload::Match() begins by attempting to match the argument types
  //       from left to right.
  //       F32Builder::Match() is called with the type::F32 argument type.
  //       F32Builder (only) matches the type::F32 type, so F32Builder::Match()
  //       returns true.
  //   (2) All the parameters have had their Matcher::Match() methods return
  //       true, there are no open-types (more about these later), so the
  //       overload has matched.
  //   (3) The semantic::Intrinsic now needs to be built, so we begin by
  //       building the overload's parameter types (these may not exactly match
  //       the argument types). Build() is called for each parameter Builder,
  //       returning the parameter type.
  //   (4) Finally, Builder::Build() is called for the return_type, and the
  //       semantic::Intrinsic is constructed and returned.
  //       Job done.
  //
  // Overload resolution also supports basic pattern matching through the use of
  // open-types and open-numbers.
  //
  // OpenTypeBuilder is a Matcher that matches a single open-type.
  //
  // An 'open-type' can be thought as a template type that is determined by the
  // arguments to the intrinsic.
  //
  // At the beginning of Overload::Match(), all open-types are undefined.
  // Open-types are closed (pinned to a fixed type) on the first attempt to
  // match against that open-type (e.g. via OpenTypeBuilder::Match()).
  // Once open-types are closed, they remain that type, and
  // OpenTypeBuilder::Match() will only ever return true if the queried type
  // matches the closed type.
  //
  // To better understand, let's consider the following hypothetical overload
  // declaration:
  //
  //    Register(I::kFoo, T, {T, T}, {OpenType::T, scalar});
  //
  //    T                  - is the matcher for the open-type OpenType::T.
  //    scalar             - is a pointer to a pre-constructed ScalarMatcher
  //                         which matches scalar types (f32, i32, u32, bool).
  // {OpenType::T, scalar} - is a constraint on the open-type OpenType::T that
  //                         it needs to resolve to a scalar.
  //
  // This call to Register() declares the foo intrinsic which accepts the
  // identical scalar type for both arguments, and returns that scalar type.
  //
  // The process for resolving this overload is as follows:
  //
  //   (1) Overload::Match() begins by attempting to match the argument types
  //       from left to right.
  //       OpenTypeBuilder::Match() is called for the first parameter, being
  //       passed the type of the first argument.
  //       The OpenType::T has not been closed yet, so the OpenType::T is closed
  //       as the type of the first argument.
  //       There's no verification that the T type is a scalar at this stage.
  //   (2) OpenTypeBuilder::Match() is called again for the second parameter
  //       with the type of the second argument.
  //       As the OpenType::T is now closed, the argument type is compared
  //       against the value of the closed-type of OpenType::T.
  //       OpenTypeBuilder::Match() returns true if these type match, otherwise
  //       false and the overload match fails.
  //   (3) If all the parameters have had their Matcher::Match() methods return
  //       true, then the open-type constraints need to be checked next.
  //       The Matcher::Match() is called for each closed type. If any return
  //       false then the overload match fails.
  //   (4) Overload::Match() now needs to build and return the output
  //       semantic::Intrinsic holding the matched overload signature.
  //   (5) The parameter types are built by calling OpenTypeBuilder::Build().
  //       This simply returns the closed type.
  //   (6) OpenTypeBuilder::Build() is called again for the return_type, and the
  //       semantic::Intrinsic is constructed and returned.
  //       Job done.
  //
  // Open-numbers are very similar to open-types, except they match against
  // integers instead of types. The rules for open-numbers are almost identical
  // to open-types, except open-numbers do not support constraints.
  //
  // vecN(f32) is an example of a Matcher that uses open-numbers.
  // vecN() constructs a OpenSizeVecBuilder that will match a vector of size
  // OpenNumber::N and of element type f32. As vecN() always uses the
  // OpenNumber::N, using vecN() multiple times in the same overload signature
  // will ensure that the vector size is identical for all vector types.
  //
  // Some Matcher implementations accept other Matchers for matching sub-types.
  // Consider:
  //
  //   Register(I::kClamp, vecN(T), {vecN(T), vecN(T), vecN(T)},
  //           {OpenType::T, fiu32});
  //
  // vecN(T) is a OpenSizeVecBuilder that matches a vector of size OpenNumber::N
  // and of element type OpenType::T, where T must be either a f32, i32, or u32.

  // clang-format off

  //       name                 return type  parameter types                    open type constraints    // NOLINT
  Register(I::kAbs,             T,           {T},                               {OpenType::T, fiu32}  ); // NOLINT
  Register(I::kAbs,             vecN_T,      {vecN_T},                          {OpenType::T, fiu32}  ); // NOLINT
  Register(I::kAcos,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kAcos,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kAll,             bool_,       {vecN_bool}                                              ); // NOLINT
  Register(I::kAny,             bool_,       {vecN_bool}                                              ); // NOLINT
  Register(I::kArrayLength,     u32,         {array_T}                                                ); // NOLINT
  Register(I::kAsin,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kAsin,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kAtan,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kAtan,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kAtan2,           f32,         {f32, f32}                                               ); // NOLINT
  Register(I::kAtan2,           vecN_f32,    {vecN_f32, vecN_f32}                                     ); // NOLINT
  Register(I::kCeil,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kCeil,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kClamp,           T,           {T, T, T},                         {OpenType::T, fiu32}  ); // NOLINT
  Register(I::kClamp,           vecN_T,      {vecN_T, vecN_T, vecN_T},          {OpenType::T, fiu32}  ); // NOLINT
  Register(I::kCos,             f32,         {f32}                                                    ); // NOLINT
  Register(I::kCos,             vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kCosh,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kCosh,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kCountOneBits,    T,           {T},                               {OpenType::T, iu32}   ); // NOLINT
  Register(I::kCountOneBits,    vecN_T,      {vecN_T},                          {OpenType::T, iu32}   ); // NOLINT
  Register(I::kCross,           vec3_f32,    {vec3_f32, vec3_f32}                                     ); // NOLINT
  Register(I::kDeterminant,     f32,         {matNxN_f32}                                             ); // NOLINT
  Register(I::kDistance,        f32,         {f32, f32}                                               ); // NOLINT
  Register(I::kDistance,        f32,         {vecN_f32, vecN_f32}                                     ); // NOLINT
  Register(I::kDot,             f32,         {vecN_f32, vecN_f32}                                     ); // NOLINT
  Register(I::kDpdx,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kDpdx,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kDpdxCoarse,      f32,         {f32}                                                    ); // NOLINT
  Register(I::kDpdxCoarse,      vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kDpdxFine,        f32,         {f32}                                                    ); // NOLINT
  Register(I::kDpdxFine,        vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kDpdy,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kDpdy,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kDpdyCoarse,      f32,         {f32}                                                    ); // NOLINT
  Register(I::kDpdyCoarse,      vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kDpdyFine,        f32,         {f32}                                                    ); // NOLINT
  Register(I::kDpdyFine,        vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kExp,             f32,         {f32}                                                    ); // NOLINT
  Register(I::kExp,             vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kExp2,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kExp2,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kFaceForward,     f32,         {f32, f32, f32}                                          ); // NOLINT
  Register(I::kFaceForward,     vecN_f32,    {vecN_f32, vecN_f32, vecN_f32}                           ); // NOLINT
  Register(I::kFloor,           f32,         {f32}                                                    ); // NOLINT
  Register(I::kFloor,           vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kFma,             f32,         {f32, f32, f32}                                          ); // NOLINT
  Register(I::kFma,             vecN_f32,    {vecN_f32, vecN_f32, vecN_f32}                           ); // NOLINT
  Register(I::kFract,           f32,         {f32}                                                    ); // NOLINT
  Register(I::kFract,           vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kFrexp,           f32,         {f32, ptr_T},                      {OpenType::T, iu32}   ); // NOLINT
  Register(I::kFrexp,           vecN_f32,    {vecN_f32, ptr_vecN_T},            {OpenType::T, iu32}   ); // NOLINT
  Register(I::kFwidth,          f32,         {f32}                                                    ); // NOLINT
  Register(I::kFwidth,          vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kFwidthCoarse,    f32,         {f32}                                                    ); // NOLINT
  Register(I::kFwidthCoarse,    vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kFwidthFine,      f32,         {f32}                                                    ); // NOLINT
  Register(I::kFwidthFine,      vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kInverseSqrt,     f32,         {f32}                                                    ); // NOLINT
  Register(I::kInverseSqrt,     vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kIsFinite,        bool_,       {f32}                                                    ); // NOLINT
  Register(I::kIsFinite,        vecN_bool,   {vecN_f32}                                               ); // NOLINT
  Register(I::kIsInf,           bool_,       {f32}                                                    ); // NOLINT
  Register(I::kIsInf,           vecN_bool,   {vecN_f32}                                               ); // NOLINT
  Register(I::kIsNan,           bool_,       {f32}                                                    ); // NOLINT
  Register(I::kIsNan,           vecN_bool,   {vecN_f32}                                               ); // NOLINT
  Register(I::kIsNormal,        bool_,       {f32}                                                    ); // NOLINT
  Register(I::kIsNormal,        vecN_bool,   {vecN_f32}                                               ); // NOLINT
  Register(I::kLdexp,           f32,         {f32, T},                          {OpenType::T, iu32}   ); // NOLINT
  Register(I::kLdexp,           vecN_f32,    {vecN_f32, vecN_T},                {OpenType::T, iu32}   ); // NOLINT
  Register(I::kLength,          f32,         {f32}                                                    ); // NOLINT
  Register(I::kLength,          f32,         {vecN_f32}                                               ); // NOLINT
  Register(I::kLog,             f32,         {f32}                                                    ); // NOLINT
  Register(I::kLog,             vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kLog2,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kLog2,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kMax,             T,           {T, T},                            {OpenType::T, fiu32}  ); // NOLINT
  Register(I::kMax,             vecN_T,      {vecN_T, vecN_T},                  {OpenType::T, fiu32}  ); // NOLINT
  Register(I::kMin,             T,           {T, T},                            {OpenType::T, fiu32}  ); // NOLINT
  Register(I::kMin,             vecN_T,      {vecN_T, vecN_T},                  {OpenType::T, fiu32}  ); // NOLINT
  Register(I::kMix,             f32,         {f32, f32, f32}                                          ); // NOLINT
  Register(I::kMix,             vecN_f32,    {vecN_f32, vecN_f32, vecN_f32}                           ); // NOLINT
  Register(I::kModf,            f32,         {f32, ptr_f32}                                           ); // NOLINT
  Register(I::kModf,            vecN_f32,    {vecN_f32, ptr_vecN_f32}                                 ); // NOLINT
  Register(I::kNormalize,       vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kPack2x16Float,   u32,         {vec2_f32}                                               ); // NOLINT
  Register(I::kPack2x16Snorm,   u32,         {vec2_f32}                                               ); // NOLINT
  Register(I::kPack2x16Unorm,   u32,         {vec2_f32}                                               ); // NOLINT
  Register(I::kPack4x8Snorm,    u32,         {vec4_f32}                                               ); // NOLINT
  Register(I::kPack4x8Unorm,    u32,         {vec4_f32}                                               ); // NOLINT
  Register(I::kPow,             f32,         {f32, f32}                                               ); // NOLINT
  Register(I::kPow,             vecN_f32,    {vecN_f32, vecN_f32}                                     ); // NOLINT
  Register(I::kReflect,         f32,         {f32, f32}                                               ); // NOLINT
  Register(I::kReflect,         vecN_f32,    {vecN_f32, vecN_f32}                                     ); // NOLINT
  Register(I::kReverseBits,     T,           {T},                               {OpenType::T, iu32}   ); // NOLINT
  Register(I::kReverseBits,     vecN_T,      {vecN_T},                          {OpenType::T, iu32}   ); // NOLINT
  Register(I::kRound,           f32,         {f32}                                                    ); // NOLINT
  Register(I::kRound,           vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kSelect,          T,           {T, T, bool_},                     {OpenType::T, scalar} ); // NOLINT
  Register(I::kSelect,          vecN_T,      {vecN_T, vecN_T, vecN_bool},       {OpenType::T, scalar} ); // NOLINT
  Register(I::kSign,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kSign,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kSin,             f32,         {f32}                                                    ); // NOLINT
  Register(I::kSin,             vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kSinh,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kSinh,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kSmoothStep,      f32,         {f32, f32, f32}                                          ); // NOLINT
  Register(I::kSmoothStep,      vecN_f32,    {vecN_f32, vecN_f32, vecN_f32}                           ); // NOLINT
  Register(I::kSqrt,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kSqrt,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kStep,            f32,         {f32, f32}                                               ); // NOLINT
  Register(I::kStep,            vecN_f32,    {vecN_f32, vecN_f32}                                     ); // NOLINT
  Register(I::kTan,             f32,         {f32}                                                    ); // NOLINT
  Register(I::kTan,             vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kTanh,            f32,         {f32}                                                    ); // NOLINT
  Register(I::kTanh,            vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kTrunc,           f32,         {f32}                                                    ); // NOLINT
  Register(I::kTrunc,           vecN_f32,    {vecN_f32}                                               ); // NOLINT
  Register(I::kUnpack2x16Float, vec2_f32,    {u32}                                                    ); // NOLINT
  Register(I::kUnpack2x16Snorm, vec2_f32,    {u32}                                                    ); // NOLINT
  Register(I::kUnpack2x16Unorm, vec2_f32,    {u32}                                                    ); // NOLINT
  Register(I::kUnpack4x8Snorm,  vec4_f32,    {u32}                                                    ); // NOLINT
  Register(I::kUnpack4x8Unorm,  vec4_f32,    {u32}                                                    ); // NOLINT
  // clang-format on

  auto* tex_1d_f32 = sampled_texture(Dim::k1d, f32);
  auto* tex_1d_T = sampled_texture(Dim::k1d, T);
  auto* tex_1d_array_f32 = sampled_texture(Dim::k1dArray, f32);
  auto* tex_1d_array_T = sampled_texture(Dim::k1dArray, T);
  auto* tex_2d_f32 = sampled_texture(Dim::k2d, f32);
  auto* tex_2d_T = sampled_texture(Dim::k2d, T);
  auto* tex_2d_array_f32 = sampled_texture(Dim::k2dArray, f32);
  auto* tex_2d_array_T = sampled_texture(Dim::k2dArray, T);
  auto* tex_3d_f32 = sampled_texture(Dim::k3d, f32);
  auto* tex_3d_T = sampled_texture(Dim::k3d, T);
  auto* tex_cube_f32 = sampled_texture(Dim::kCube, f32);
  auto* tex_cube_T = sampled_texture(Dim::kCube, T);
  auto* tex_cube_array_f32 = sampled_texture(Dim::kCubeArray, f32);
  auto* tex_cube_array_T = sampled_texture(Dim::kCubeArray, T);
  auto* tex_ms_2d_T = multisampled_texture(Dim::k2d, T);
  auto* tex_ms_2d_array_T = multisampled_texture(Dim::k2dArray, T);
  auto* tex_depth_2d = depth_texture(Dim::k2d);
  auto* tex_depth_2d_array = depth_texture(Dim::k2dArray);
  auto* tex_depth_cube = depth_texture(Dim::kCube);
  auto* tex_depth_cube_array = depth_texture(Dim::kCubeArray);
  auto* tex_storage_1d_FT =
      storage_texture(Dim::k1d, OpenNumber::F, OpenType::T);
  auto* tex_storage_1d_array_FT =
      storage_texture(Dim::k1dArray, OpenNumber::F, OpenType::T);
  auto* tex_storage_2d_FT =
      storage_texture(Dim::k2d, OpenNumber::F, OpenType::T);
  auto* tex_storage_2d_array_FT =
      storage_texture(Dim::k2dArray, OpenNumber::F, OpenType::T);
  auto* tex_storage_3d_FT =
      storage_texture(Dim::k3d, OpenNumber::F, OpenType::T);
  auto* tex_storage_ro_1d_FT =
      access_control(ast::AccessControl::kReadOnly, tex_storage_1d_FT);
  auto* tex_storage_ro_1d_array_FT =
      access_control(ast::AccessControl::kReadOnly, tex_storage_1d_array_FT);
  auto* tex_storage_ro_2d_FT =
      access_control(ast::AccessControl::kReadOnly, tex_storage_2d_FT);
  auto* tex_storage_ro_2d_array_FT =
      access_control(ast::AccessControl::kReadOnly, tex_storage_2d_array_FT);
  auto* tex_storage_ro_3d_FT =
      access_control(ast::AccessControl::kReadOnly, tex_storage_3d_FT);
  auto* tex_storage_wo_1d_FT =
      access_control(ast::AccessControl::kWriteOnly, tex_storage_1d_FT);
  auto* tex_storage_wo_1d_array_FT =
      access_control(ast::AccessControl::kWriteOnly, tex_storage_1d_array_FT);
  auto* tex_storage_wo_2d_FT =
      access_control(ast::AccessControl::kWriteOnly, tex_storage_2d_FT);
  auto* tex_storage_wo_2d_array_FT =
      access_control(ast::AccessControl::kWriteOnly, tex_storage_2d_array_FT);
  auto* tex_storage_wo_3d_FT =
      access_control(ast::AccessControl::kWriteOnly, tex_storage_3d_FT);
  auto* sampler = this->sampler(type::SamplerKind::kSampler);
  auto* sampler_comparison =
      this->sampler(type::SamplerKind::kComparisonSampler);
  auto t = semantic::Parameter::Usage::kTexture;
  auto s = semantic::Parameter::Usage::kSampler;
  auto coords = semantic::Parameter::Usage::kCoords;
  auto array_index = semantic::Parameter::Usage::kArrayIndex;
  auto ddx = semantic::Parameter::Usage::kDdx;
  auto ddy = semantic::Parameter::Usage::kDdy;
  auto depth_ref = semantic::Parameter::Usage::kDepthRef;
  auto bias = semantic::Parameter::Usage::kBias;
  auto level = semantic::Parameter::Usage::kLevel;
  auto offset = semantic::Parameter::Usage::kOffset;
  auto value = semantic::Parameter::Usage::kValue;
  auto sample_index = semantic::Parameter::Usage::kSampleIndex;

  // clang-format off

  //       name                   return type  parameter types
  Register(I::kTextureDimensions, i32,      {{t, tex_1d_T},                              }); // NOLINT
  Register(I::kTextureDimensions, i32,      {{t, tex_1d_array_T},                        }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_2d_T},                              }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_2d_T},                {level, i32}, }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_2d_array_T},                        }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_2d_array_T},          {level, i32}, }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_3d_T},                              }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_3d_T},                {level, i32}, }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_cube_T},                            }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_cube_T},              {level, i32}, }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_cube_array_T},                      }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_cube_array_T},        {level, i32}, }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_ms_2d_T},                           }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_ms_2d_array_T},                     }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_depth_2d},                          }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_depth_2d},            {level, i32}, }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_depth_2d_array},                    }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_depth_2d_array},      {level, i32}, }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_depth_cube},                        }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_depth_cube},          {level, i32}, }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_depth_cube_array},                  }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_depth_cube_array},    {level, i32}, }); // NOLINT
  Register(I::kTextureDimensions, i32,      {{t, tex_storage_1d_FT},                     }); // NOLINT
  Register(I::kTextureDimensions, i32,      {{t, tex_storage_1d_array_FT},               }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_storage_2d_FT},                     }); // NOLINT
  Register(I::kTextureDimensions, vec2_i32, {{t, tex_storage_2d_array_FT},               }); // NOLINT
  Register(I::kTextureDimensions, vec3_i32, {{t, tex_storage_3d_FT},                     }); // NOLINT

  Register(I::kTextureNumLayers,  i32, {{t, tex_1d_array_T},          });
  Register(I::kTextureNumLayers,  i32, {{t, tex_2d_array_T},          });
  Register(I::kTextureNumLayers,  i32, {{t, tex_cube_array_T},        });
  Register(I::kTextureNumLayers,  i32, {{t, tex_ms_2d_array_T},       });
  Register(I::kTextureNumLayers,  i32, {{t, tex_depth_2d_array},      });
  Register(I::kTextureNumLayers,  i32, {{t, tex_depth_cube_array},    });
  Register(I::kTextureNumLayers,  i32, {{t, tex_storage_1d_array_FT}, });
  Register(I::kTextureNumLayers,  i32, {{t, tex_storage_2d_array_FT}, });

  Register(I::kTextureNumLevels,  i32, {{t, tex_2d_T},             });
  Register(I::kTextureNumLevels,  i32, {{t, tex_2d_array_T},       });
  Register(I::kTextureNumLevels,  i32, {{t, tex_3d_T},             });
  Register(I::kTextureNumLevels,  i32, {{t, tex_cube_T},           });
  Register(I::kTextureNumLevels,  i32, {{t, tex_cube_array_T},     });
  Register(I::kTextureNumLevels,  i32, {{t, tex_depth_2d},         });
  Register(I::kTextureNumLevels,  i32, {{t, tex_depth_2d_array},   });
  Register(I::kTextureNumLevels,  i32, {{t, tex_depth_cube},       });
  Register(I::kTextureNumLevels,  i32, {{t, tex_depth_cube_array}, });

  Register(I::kTextureNumSamples, i32, {{t, tex_ms_2d_T},       });
  Register(I::kTextureNumSamples, i32, {{t, tex_ms_2d_array_T}, });

  Register(I::kTextureSample, vec4_f32, {{t, tex_1d_f32},           {s, sampler}, {coords, f32},                                              }); // NOLINT
  Register(I::kTextureSample, vec4_f32, {{t, tex_1d_array_f32},     {s, sampler}, {coords, f32},      {array_index, i32},                     }); // NOLINT
  Register(I::kTextureSample, vec4_f32, {{t, tex_2d_f32},           {s, sampler}, {coords, vec2_f32},                                         }); // NOLINT
  Register(I::kTextureSample, vec4_f32, {{t, tex_2d_f32},           {s, sampler}, {coords, vec2_f32},                     {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSample, vec4_f32, {{t, tex_2d_array_f32},     {s, sampler}, {coords, vec2_f32}, {array_index, i32},                     }); // NOLINT
  Register(I::kTextureSample, vec4_f32, {{t, tex_2d_array_f32},     {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSample, vec4_f32, {{t, tex_3d_f32},           {s, sampler}, {coords, vec3_f32},                                         }); // NOLINT
  Register(I::kTextureSample, vec4_f32, {{t, tex_3d_f32},           {s, sampler}, {coords, vec3_f32},                     {offset, vec3_i32}, }); // NOLINT
  Register(I::kTextureSample, vec4_f32, {{t, tex_cube_f32},         {s, sampler}, {coords, vec3_f32},                                         }); // NOLINT
  Register(I::kTextureSample, vec4_f32, {{t, tex_cube_array_f32},   {s, sampler}, {coords, vec3_f32}, {array_index, i32},                     }); // NOLINT
  Register(I::kTextureSample, f32,      {{t, tex_depth_2d},         {s, sampler}, {coords, vec2_f32},                                         }); // NOLINT
  Register(I::kTextureSample, f32,      {{t, tex_depth_2d},         {s, sampler}, {coords, vec2_f32},                     {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSample, f32,      {{t, tex_depth_2d_array},   {s, sampler}, {coords, vec2_f32}, {array_index, i32},                     }); // NOLINT
  Register(I::kTextureSample, f32,      {{t, tex_depth_2d_array},   {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSample, f32,      {{t, tex_depth_cube},       {s, sampler}, {coords, vec3_f32},                                         }); // NOLINT
  Register(I::kTextureSample, f32,      {{t, tex_depth_cube_array}, {s, sampler}, {coords, vec3_f32}, {array_index, i32},                     }); // NOLINT

  Register(I::kTextureSampleBias, vec4_f32,    {{t, tex_2d_f32},           {s, sampler}, {coords, vec2_f32},                     {bias, f32},                     }); // NOLINT
  Register(I::kTextureSampleBias, vec4_f32,    {{t, tex_2d_f32},           {s, sampler}, {coords, vec2_f32},                     {bias, f32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSampleBias, vec4_f32,    {{t, tex_2d_array_f32},     {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {bias, f32},                     }); // NOLINT
  Register(I::kTextureSampleBias, vec4_f32,    {{t, tex_2d_array_f32},     {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {bias, f32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSampleBias, vec4_f32,    {{t, tex_3d_f32},           {s, sampler}, {coords, vec3_f32},                     {bias, f32},                     }); // NOLINT
  Register(I::kTextureSampleBias, vec4_f32,    {{t, tex_3d_f32},           {s, sampler}, {coords, vec3_f32},                     {bias, f32}, {offset, vec3_i32}, }); // NOLINT
  Register(I::kTextureSampleBias, vec4_f32,    {{t, tex_cube_f32},         {s, sampler}, {coords, vec3_f32},                     {bias, f32},                     }); // NOLINT
  Register(I::kTextureSampleBias, vec4_f32,    {{t, tex_cube_array_f32},   {s, sampler}, {coords, vec3_f32}, {array_index, i32}, {bias, f32},                     }); // NOLINT

  Register(I::kTextureSampleCompare, f32,      {{t, tex_depth_2d},         {s, sampler_comparison}, {coords, vec2_f32},                     {depth_ref, f32},                         }); // NOLINT
  Register(I::kTextureSampleCompare, f32,      {{t, tex_depth_2d},         {s, sampler_comparison}, {coords, vec2_f32},                     {depth_ref, f32}, {offset, vec2_i32},     }); // NOLINT
  Register(I::kTextureSampleCompare, f32,      {{t, tex_depth_2d_array},   {s, sampler_comparison}, {coords, vec2_f32}, {array_index, i32}, {depth_ref, f32},                         }); // NOLINT
  Register(I::kTextureSampleCompare, f32,      {{t, tex_depth_2d_array},   {s, sampler_comparison}, {coords, vec2_f32}, {array_index, i32}, {depth_ref, f32}, {offset, vec2_i32},     }); // NOLINT
  Register(I::kTextureSampleCompare, f32,      {{t, tex_depth_cube},       {s, sampler_comparison}, {coords, vec3_f32},                     {depth_ref, f32},                         }); // NOLINT
  Register(I::kTextureSampleCompare, f32,      {{t, tex_depth_cube_array}, {s, sampler_comparison}, {coords, vec3_f32}, {array_index, i32}, {depth_ref, f32},                         }); // NOLINT

  Register(I::kTextureSampleGrad, vec4_f32,      {{t, tex_2d_f32},         {s, sampler}, {coords, vec2_f32},                     {ddx, vec2_f32}, {ddy, vec2_f32},                     }); // NOLINT
  Register(I::kTextureSampleGrad, vec4_f32,      {{t, tex_2d_f32},         {s, sampler}, {coords, vec2_f32},                     {ddx, vec2_f32}, {ddy, vec2_f32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSampleGrad, vec4_f32,      {{t, tex_2d_array_f32},   {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {ddx, vec2_f32}, {ddy, vec2_f32},                     }); // NOLINT
  Register(I::kTextureSampleGrad, vec4_f32,      {{t, tex_2d_array_f32},   {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {ddx, vec2_f32}, {ddy, vec2_f32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSampleGrad, vec4_f32,      {{t, tex_3d_f32},         {s, sampler}, {coords, vec3_f32},                     {ddx, vec3_f32}, {ddy, vec3_f32},                     }); // NOLINT
  Register(I::kTextureSampleGrad, vec4_f32,      {{t, tex_3d_f32},         {s, sampler}, {coords, vec3_f32},                     {ddx, vec3_f32}, {ddy, vec3_f32}, {offset, vec3_i32}, }); // NOLINT
  Register(I::kTextureSampleGrad, vec4_f32,      {{t, tex_cube_f32},       {s, sampler}, {coords, vec3_f32},                     {ddx, vec3_f32}, {ddy, vec3_f32},                     }); // NOLINT
  Register(I::kTextureSampleGrad, vec4_f32,      {{t, tex_cube_array_f32}, {s, sampler}, {coords, vec3_f32}, {array_index, i32}, {ddx, vec3_f32}, {ddy, vec3_f32},                     }); // NOLINT

  Register(I::kTextureSampleLevel, vec4_f32,     {{t, tex_2d_f32},          {s, sampler}, {coords, vec2_f32},                     {level, f32},                     }); // NOLINT
  Register(I::kTextureSampleLevel, vec4_f32,     {{t, tex_2d_f32},          {s, sampler}, {coords, vec2_f32},                     {level, f32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSampleLevel, vec4_f32,     {{t, tex_2d_array_f32},    {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {level, f32},                     }); // NOLINT
  Register(I::kTextureSampleLevel, vec4_f32,     {{t, tex_2d_array_f32},    {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {level, f32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSampleLevel, vec4_f32,     {{t, tex_3d_f32},          {s, sampler}, {coords, vec3_f32},                     {level, f32},                     }); // NOLINT
  Register(I::kTextureSampleLevel, vec4_f32,     {{t, tex_3d_f32},          {s, sampler}, {coords, vec3_f32},                     {level, f32}, {offset, vec3_i32}, }); // NOLINT
  Register(I::kTextureSampleLevel, vec4_f32,     {{t, tex_cube_f32},        {s, sampler}, {coords, vec3_f32},                     {level, f32},                     }); // NOLINT
  Register(I::kTextureSampleLevel, vec4_f32,     {{t, tex_cube_array_f32},  {s, sampler}, {coords, vec3_f32}, {array_index, i32}, {level, f32},                     }); // NOLINT
  Register(I::kTextureSampleLevel, f32,          {{t, tex_depth_2d},        {s, sampler}, {coords, vec2_f32},                     {level, i32},                     }); // NOLINT
  Register(I::kTextureSampleLevel, f32,          {{t, tex_depth_2d},        {s, sampler}, {coords, vec2_f32},                     {level, i32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSampleLevel, f32,          {{t, tex_depth_2d_array},  {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {level, i32},                     }); // NOLINT
  Register(I::kTextureSampleLevel, f32,          {{t, tex_depth_2d_array},  {s, sampler}, {coords, vec2_f32}, {array_index, i32}, {level, i32}, {offset, vec2_i32}, }); // NOLINT
  Register(I::kTextureSampleLevel, f32,          {{t, tex_depth_cube},      {s, sampler}, {coords, vec3_f32},                     {level, i32},                     }); // NOLINT
  Register(I::kTextureSampleLevel, f32,          {{t, tex_depth_cube_array},{s, sampler}, {coords, vec3_f32}, {array_index, i32}, {level, i32},                     }); // NOLINT

  Register(I::kTextureStore, void_, {{t, tex_storage_wo_1d_FT},      {coords, i32},                          {value, vec4_T}, }); // NOLINT
  Register(I::kTextureStore, void_, {{t, tex_storage_wo_1d_array_FT},{coords, i32},      {array_index, i32}, {value, vec4_T}, }); // NOLINT
  Register(I::kTextureStore, void_, {{t, tex_storage_wo_2d_FT},      {coords, vec2_i32},                     {value, vec4_T}, }); // NOLINT
  Register(I::kTextureStore, void_, {{t, tex_storage_wo_2d_array_FT},{coords, vec2_i32}, {array_index, i32}, {value, vec4_T}, }); // NOLINT
  Register(I::kTextureStore, void_, {{t, tex_storage_wo_3d_FT},      {coords, vec3_i32},                     {value, vec4_T}, }); // NOLINT

  Register(I::kTextureLoad, vec4_T, {{t, tex_2d_T},               {coords, vec2_i32},                      {level, i32},                      }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_2d_array_T},         {coords, vec2_i32}, {array_index, i32},  {level, i32},                      }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_3d_T},               {coords, vec3_i32},                      {level, i32},                      }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_ms_2d_T},            {coords, vec2_i32},                                    {sample_index, i32}, }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_ms_2d_array_T},      {coords, vec2_i32}, {array_index, i32},                {sample_index, i32}, }); // NOLINT
  Register(I::kTextureLoad, f32,    {{t, tex_depth_2d},           {coords, vec2_i32},                      {level, i32},                      }); // NOLINT
  Register(I::kTextureLoad, f32,    {{t, tex_depth_2d_array},     {coords, vec2_i32}, {array_index, i32},  {level, i32},                      }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_storage_ro_1d_FT},      {coords, i32},                                                              }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_storage_ro_1d_array_FT},{coords, i32},      {array_index, i32},                                     }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_storage_ro_2d_FT},      {coords, vec2_i32},                                                         }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_storage_ro_2d_array_FT},{coords, vec2_i32}, {array_index, i32},                                     }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_storage_ro_3d_FT},      {coords, vec3_i32},                                                         }); // NOLINT

  // TODO(bclayton): Update the rest of tint to reflect the spec changes made in
  // https://github.com/gpuweb/gpuweb/pull/1301:

  // Overloads added in https://github.com/gpuweb/gpuweb/pull/1301
  Register(I::kTextureLoad, vec4_T, {{t, tex_1d_T},               {coords, i32},                           {level, i32},                      }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_1d_array_T},         {coords, i32},      {array_index, i32},  {level, i32},                      }); // NOLINT

  // Overloads removed in https://github.com/gpuweb/gpuweb/pull/1301
  Register(I::kTextureLoad, vec4_T, {{t, tex_1d_T},               {coords, i32},                                                              }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_1d_array_T},         {coords, i32},      {array_index, i32},                                     }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_2d_T},               {coords, vec2_i32},                                                         }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_2d_array_T},         {coords, vec2_i32}, {array_index, i32},                                     }); // NOLINT
  Register(I::kTextureLoad, vec4_T, {{t, tex_3d_T},               {coords, vec3_i32},                                                         }); // NOLINT
  Register(I::kTextureLoad, f32,    {{t, tex_depth_2d},           {coords, vec2_i32},                                                         }); // NOLINT
  Register(I::kTextureLoad, f32,    {{t, tex_depth_2d_array},     {coords, vec2_i32}, {array_index, i32},                                     }); // NOLINT

  // clang-format on
}

std::string Impl::Overload::str() const {
  std::stringstream ss;
  ss << type << "(";
  {
    bool first = true;
    for (auto param : parameters) {
      if (!first) {
        ss << ", ";
      }
      first = false;
      if (param.usage != semantic::Parameter::Usage::kNone) {
        ss << semantic::str(param.usage) << " : ";
      }
      ss << param.matcher->str();
    }
  }
  ss << ") -> ";
  ss << return_type->str();

  if (!open_type_matchers.empty()) {
    ss << "  where: ";

    for (uint32_t i = 0; i < static_cast<uint32_t>(OpenType::Count); i++) {
      auto open_type = static_cast<OpenType>(i);
      auto it = open_type_matchers.find(open_type);
      if (it != open_type_matchers.end()) {
        if (i > 0) {
          ss << ", ";
        }
        ss << tint::str(open_type) << " is " << it->second->str();
      }
    }
  }
  return ss.str();
}

IntrinsicTable::Result Impl::Lookup(
    ProgramBuilder& builder,
    semantic::IntrinsicType type,
    const std::vector<type::Type*>& args) const {
  // Candidate holds information about a mismatched overload that could be what
  // the user intended to call.
  struct Candidate {
    const Overload* overload;
    int score;
  };

  // The list of failed matches that had promise.
  std::vector<Candidate> candidates;

  // TODO(bclayton) Sort overloads_, or place them into a map keyed by intrinsic
  // type. This is horribly inefficient.
  for (auto& overload : overloads_) {
    int match_score = 0;
    if (auto* intrinsic = overload.Match(builder, type, args, match_score)) {
      return Result{intrinsic, ""};  // Match found
    }
    if (match_score > 0) {
      candidates.emplace_back(Candidate{&overload, match_score});
    }
  }

  // Sort the candidates with the most promising first
  std::stable_sort(
      candidates.begin(), candidates.end(),
      [](const Candidate& a, const Candidate& b) { return a.score > b.score; });

  // Generate an error message
  std::stringstream ss;
  ss << "no matching call to " << semantic::str(type) << "(";
  {
    bool first = true;
    for (auto* arg : args) {
      if (!first) {
        ss << ", ";
      }
      first = false;
      ss << arg->FriendlyName(builder.Symbols());
    }
  }
  ss << ")" << std::endl;

  if (!candidates.empty()) {
    ss << std::endl;
    ss << candidates.size() << " candidate function"
       << (candidates.size() > 1 ? "s:" : ":") << std::endl;
    for (auto& candidate : candidates) {
      ss << "  " << candidate.overload->str() << std::endl;
    }
  }

  return Result{nullptr, ss.str()};
}

semantic::Intrinsic* Impl::Overload::Match(ProgramBuilder& builder,
                                           semantic::IntrinsicType intrinsic,
                                           const std::vector<type::Type*>& args,
                                           int& match_score) const {
  if (type != intrinsic) {
    match_score = std::numeric_limits<int>::min();
    return nullptr;  // Incorrect function
  }

  // Penalize argument <-> parameter count mismatches
  match_score = 1000;
  match_score -= std::max(parameters.size(), args.size()) -
                 std::min(parameters.size(), args.size());

  bool matched = parameters.size() == args.size();

  Matcher::MatchState matcher_state;

  // Check that each of the parameters match.
  // This stage also populates the open_types and open_numbers.
  auto count = std::min(parameters.size(), args.size());
  for (size_t i = 0; i < count; i++) {
    assert(args[i]);
    auto* arg_ty = args[i];
    if (auto* ptr = arg_ty->As<type::Pointer>()) {
      if (!parameters[i].matcher->ExpectsPointer()) {
        // Argument is a pointer, but the matcher isn't expecting one.
        // Perform an implicit dereference.
        arg_ty = ptr->type();
      }
    }
    if (parameters[i].matcher->Match(matcher_state, arg_ty)) {
      // A correct parameter match is scored higher than number of parameters to
      // arguments.
      match_score += 2;
    } else {
      matched = false;
    }
  }
  if (!matched) {
    return nullptr;
  }

  // If any of the open-types are constrained, check that they match.
  for (auto matcher_it : open_type_matchers) {
    OpenType open_type = matcher_it.first;
    auto* matcher = matcher_it.second;
    auto type_it = matcher_state.open_types.find(open_type);
    if (type_it == matcher_state.open_types.end()) {
      // We have an overload that claims to have matched, but didn't actually
      // resolve the open type. This is a bug that needs fixing.
      assert(false);
      return nullptr;
    }
    auto* resolved_type = type_it->second;
    if (resolved_type == nullptr) {
      // We have an overload that claims to have matched, but has a nullptr
      // resolved open type. This is a bug that needs fixing.
      assert(false);
      return nullptr;
    }
    if (!matcher->Match(matcher_state, resolved_type)) {
      matched = false;
      continue;
    }
    match_score++;
  }
  if (!matched) {
    return nullptr;
  }

  // Overload matched!

  // Build the return type
  Builder::BuildState builder_state{builder.Types(), matcher_state.open_types,
                                    matcher_state.open_numbers};
  auto* ret = return_type->Build(builder_state);
  assert(ret);  // Build() must return a type

  // Build the semantic parameters
  semantic::ParameterList params;
  params.reserve(parameters.size());
  for (size_t i = 0; i < args.size(); i++) {
    auto& parameter = parameters[i];
    auto* ty = parameter.matcher->Build(builder_state);
    params.emplace_back(semantic::Parameter{ty, parameter.usage});
  }

  return builder.create<semantic::Intrinsic>(intrinsic, ret, params);
}

}  // namespace

std::unique_ptr<IntrinsicTable> IntrinsicTable::Create() {
  return std::make_unique<Impl>();
}

IntrinsicTable::~IntrinsicTable() = default;

}  // namespace tint
