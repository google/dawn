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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or stateied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/reader/spirv/parser_type.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "src/program_builder.h"
#include "src/utils/get_or_create.h"
#include "src/utils/hash.h"

TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Type);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Void);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Bool);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::U32);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::F32);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::I32);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Pointer);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Reference);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Vector);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Matrix);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Array);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::AccessControl);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Sampler);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Texture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::DepthTexture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::MultisampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::SampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::StorageTexture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Named);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Alias);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Struct);

namespace tint {
namespace reader {
namespace spirv {

namespace {
struct PointerHasher {
  size_t operator()(const Pointer& t) const {
    return utils::Hash(t.type, t.storage_class);
  }
};

struct ReferenceHasher {
  size_t operator()(const Reference& t) const {
    return utils::Hash(t.type, t.storage_class);
  }
};

struct VectorHasher {
  size_t operator()(const Vector& t) const {
    return utils::Hash(t.type, t.size);
  }
};

struct MatrixHasher {
  size_t operator()(const Matrix& t) const {
    return utils::Hash(t.type, t.columns, t.rows);
  }
};

struct ArrayHasher {
  size_t operator()(const Array& t) const {
    return utils::Hash(t.type, t.size, t.stride);
  }
};

struct AccessControlHasher {
  size_t operator()(const AccessControl& t) const {
    return utils::Hash(t.type, t.access);
  }
};

struct MultisampledTextureHasher {
  size_t operator()(const MultisampledTexture& t) const {
    return utils::Hash(t.dims, t.type);
  }
};

struct SampledTextureHasher {
  size_t operator()(const SampledTexture& t) const {
    return utils::Hash(t.dims, t.type);
  }
};

struct StorageTextureHasher {
  size_t operator()(const StorageTexture& t) const {
    return utils::Hash(t.dims, t.format);
  }
};
}  // namespace

static bool operator==(const Pointer& a, const Pointer& b) {
  return a.type == b.type && a.storage_class == b.storage_class;
}

static bool operator==(const Reference& a, const Reference& b) {
  return a.type == b.type && a.storage_class == b.storage_class;
}

static bool operator==(const Vector& a, const Vector& b) {
  return a.type == b.type && a.size == b.size;
}

static bool operator==(const Matrix& a, const Matrix& b) {
  return a.type == b.type && a.columns == b.columns && a.rows == b.rows;
}

static bool operator==(const Array& a, const Array& b) {
  return a.type == b.type && a.size == b.size && a.stride == b.stride;
}

static bool operator==(const AccessControl& a, const AccessControl& b) {
  return a.type == b.type && a.access == b.access;
}

static bool operator==(const MultisampledTexture& a,
                       const MultisampledTexture& b) {
  return a.dims == b.dims && a.type == b.type;
}

static bool operator==(const SampledTexture& a, const SampledTexture& b) {
  return a.dims == b.dims && a.type == b.type;
}

static bool operator==(const StorageTexture& a, const StorageTexture& b) {
  return a.dims == b.dims && a.format == b.format;
}

ast::Type* Void::Build(ProgramBuilder& b) const {
  return b.ty.void_();
}

ast::Type* Bool::Build(ProgramBuilder& b) const {
  return b.ty.bool_();
}

ast::Type* U32::Build(ProgramBuilder& b) const {
  return b.ty.u32();
}

ast::Type* F32::Build(ProgramBuilder& b) const {
  return b.ty.f32();
}

ast::Type* I32::Build(ProgramBuilder& b) const {
  return b.ty.i32();
}

Pointer::Pointer(const Type* t, ast::StorageClass s)
    : type(t), storage_class(s) {}
Pointer::Pointer(const Pointer&) = default;

ast::Type* Pointer::Build(ProgramBuilder& b) const {
  return b.ty.pointer(type->Build(b), storage_class);
}

Reference::Reference(const Type* t, ast::StorageClass s)
    : type(t), storage_class(s) {}
Reference::Reference(const Reference&) = default;

ast::Type* Reference::Build(ProgramBuilder& b) const {
  return type->Build(b);
}

Vector::Vector(const Type* t, uint32_t s) : type(t), size(s) {}
Vector::Vector(const Vector&) = default;

ast::Type* Vector::Build(ProgramBuilder& b) const {
  return b.ty.vec(type->Build(b), size);
}

Matrix::Matrix(const Type* t, uint32_t c, uint32_t r)
    : type(t), columns(c), rows(r) {}
Matrix::Matrix(const Matrix&) = default;

ast::Type* Matrix::Build(ProgramBuilder& b) const {
  return b.ty.mat(type->Build(b), columns, rows);
}

Array::Array(const Type* t, uint32_t sz, uint32_t st)
    : type(t), size(sz), stride(st) {}
Array::Array(const Array&) = default;

ast::Type* Array::Build(ProgramBuilder& b) const {
  return b.ty.array(type->Build(b), size, stride);
}

AccessControl::AccessControl(const Type* t, ast::AccessControl::Access a)
    : type(t), access(a) {}
AccessControl::AccessControl(const AccessControl&) = default;

ast::Type* AccessControl::Build(ProgramBuilder& b) const {
  return b.ty.access(access, type->Build(b));
}

Sampler::Sampler(ast::SamplerKind k) : kind(k) {}
Sampler::Sampler(const Sampler&) = default;

ast::Type* Sampler::Build(ProgramBuilder& b) const {
  return b.ty.sampler(kind);
}

Texture::Texture(ast::TextureDimension d) : dims(d) {}
Texture::Texture(const Texture&) = default;

DepthTexture::DepthTexture(ast::TextureDimension d) : Base(d) {}
DepthTexture::DepthTexture(const DepthTexture&) = default;

ast::Type* DepthTexture::Build(ProgramBuilder& b) const {
  return b.ty.depth_texture(dims);
}

MultisampledTexture::MultisampledTexture(ast::TextureDimension d, const Type* t)
    : Base(d), type(t) {}
MultisampledTexture::MultisampledTexture(const MultisampledTexture&) = default;

ast::Type* MultisampledTexture::Build(ProgramBuilder& b) const {
  return b.ty.multisampled_texture(dims, type->Build(b));
}

SampledTexture::SampledTexture(ast::TextureDimension d, const Type* t)
    : Base(d), type(t) {}
SampledTexture::SampledTexture(const SampledTexture&) = default;

ast::Type* SampledTexture::Build(ProgramBuilder& b) const {
  return b.ty.sampled_texture(dims, type->Build(b));
}

StorageTexture::StorageTexture(ast::TextureDimension d, ast::ImageFormat f)
    : Base(d), format(f) {}
StorageTexture::StorageTexture(const StorageTexture&) = default;

ast::Type* StorageTexture::Build(ProgramBuilder& b) const {
  return b.ty.storage_texture(dims, format);
}

Named::Named(Symbol n) : name(n) {}
Named::Named(const Named&) = default;
Named::~Named() = default;

Alias::Alias(Symbol n, const Type* ty) : Base(n), type(ty) {}
Alias::Alias(const Alias&) = default;

ast::Type* Alias::Build(ProgramBuilder& b) const {
  return b.ty.type_name(name);
}

Struct::Struct(Symbol n, TypeList m) : Base(n), members(std::move(m)) {}
Struct::Struct(const Struct&) = default;
Struct::~Struct() = default;

ast::Type* Struct::Build(ProgramBuilder& b) const {
  return b.ty.type_name(name);
}

/// The PIMPL state of the Types object.
struct TypeManager::State {
  /// The allocator of types
  BlockAllocator<Type> allocator_;
  /// The lazily-created Void type
  spirv::Void const* void_ = nullptr;
  /// The lazily-created Bool type
  spirv::Bool const* bool_ = nullptr;
  /// The lazily-created U32 type
  spirv::U32 const* u32_ = nullptr;
  /// The lazily-created F32 type
  spirv::F32 const* f32_ = nullptr;
  /// The lazily-created I32 type
  spirv::I32 const* i32_ = nullptr;
  /// Map of Pointer to the returned Pointer type instance
  std::unordered_map<spirv::Pointer, const spirv::Pointer*, PointerHasher>
      pointers_;
  /// Map of Reference to the returned Reference type instance
  std::unordered_map<spirv::Reference, const spirv::Reference*, ReferenceHasher>
      references_;
  /// Map of Vector to the returned Vector type instance
  std::unordered_map<spirv::Vector, const spirv::Vector*, VectorHasher>
      vectors_;
  /// Map of Matrix to the returned Matrix type instance
  std::unordered_map<spirv::Matrix, const spirv::Matrix*, MatrixHasher>
      matrices_;
  /// Map of Array to the returned Array type instance
  std::unordered_map<spirv::Array, const spirv::Array*, ArrayHasher> arrays_;
  /// Map of AccessControl to the returned AccessControl type instance
  std::unordered_map<spirv::AccessControl,
                     const spirv::AccessControl*,
                     AccessControlHasher>
      access_controls_;
  /// Map of type name to returned Alias instance
  std::unordered_map<Symbol, const spirv::Alias*> aliases_;
  /// Map of type name to returned Struct instance
  std::unordered_map<Symbol, const spirv::Struct*> structs_;
  /// Map of ast::SamplerKind to returned Sampler instance
  std::unordered_map<ast::SamplerKind, const spirv::Sampler*> samplers_;
  /// Map of ast::TextureDimension to returned DepthTexture instance
  std::unordered_map<ast::TextureDimension, const spirv::DepthTexture*>
      depth_textures_;
  /// Map of MultisampledTexture to the returned MultisampledTexture type
  /// instance
  std::unordered_map<spirv::MultisampledTexture,
                     const spirv::MultisampledTexture*,
                     MultisampledTextureHasher>
      multisampled_textures_;
  /// Map of SampledTexture to the returned SampledTexture type instance
  std::unordered_map<spirv::SampledTexture,
                     const spirv::SampledTexture*,
                     SampledTextureHasher>
      sampled_textures_;
  /// Map of StorageTexture to the returned StorageTexture type instance
  std::unordered_map<spirv::StorageTexture,
                     const spirv::StorageTexture*,
                     StorageTextureHasher>
      storage_textures_;
};

const Type* Type::UnwrapPtr() const {
  const Type* type = this;
  while (auto* ptr = type->As<Pointer>()) {
    type = ptr->type;
  }
  return type;
}

const Type* Type::UnwrapAlias() const {
  const Type* type = this;
  while (auto* alias = type->As<Alias>()) {
    type = alias->type;
  }
  return type;
}

const Type* Type::UnwrapAliasAndAccess() const {
  auto* type = this;
  while (true) {
    if (auto* alias = type->As<Alias>()) {
      type = alias->type;
    } else if (auto* access = type->As<AccessControl>()) {
      type = access->type;
    } else {
      break;
    }
  }
  return type;
}

const Type* Type::UnwrapAll() const {
  auto* type = this;
  while (true) {
    if (auto* alias = type->As<Alias>()) {
      type = alias->type;
    } else if (auto* access = type->As<AccessControl>()) {
      type = access->type;
    } else if (auto* ptr = type->As<Pointer>()) {
      type = ptr->type;
    } else {
      break;
    }
  }
  return type;
}

bool Type::IsFloatScalar() const {
  return Is<F32>();
}

bool Type::IsFloatScalarOrVector() const {
  return IsFloatScalar() || IsFloatVector();
}

bool Type::IsFloatVector() const {
  return Is<Vector>([](const Vector* v) { return v->type->IsFloatScalar(); });
}

bool Type::IsIntegerScalar() const {
  return IsAnyOf<U32, I32>();
}

bool Type::IsIntegerScalarOrVector() const {
  return IsUnsignedScalarOrVector() || IsSignedScalarOrVector();
}

bool Type::IsScalar() const {
  return IsAnyOf<F32, U32, I32, Bool>();
}

bool Type::IsSignedIntegerVector() const {
  return Is<Vector>([](const Vector* v) { return v->type->Is<I32>(); });
}

bool Type::IsSignedScalarOrVector() const {
  return Is<I32>() || IsSignedIntegerVector();
}

bool Type::IsUnsignedIntegerVector() const {
  return Is<Vector>([](const Vector* v) { return v->type->Is<U32>(); });
}

bool Type::IsUnsignedScalarOrVector() const {
  return Is<U32>() || IsUnsignedIntegerVector();
}

TypeManager::TypeManager() {
  state = std::make_unique<State>();
}

TypeManager::~TypeManager() = default;

const spirv::Void* TypeManager::Void() {
  if (!state->void_) {
    state->void_ = state->allocator_.Create<spirv::Void>();
  }
  return state->void_;
}

const spirv::Bool* TypeManager::Bool() {
  if (!state->bool_) {
    state->bool_ = state->allocator_.Create<spirv::Bool>();
  }
  return state->bool_;
}

const spirv::U32* TypeManager::U32() {
  if (!state->u32_) {
    state->u32_ = state->allocator_.Create<spirv::U32>();
  }
  return state->u32_;
}

const spirv::F32* TypeManager::F32() {
  if (!state->f32_) {
    state->f32_ = state->allocator_.Create<spirv::F32>();
  }
  return state->f32_;
}

const spirv::I32* TypeManager::I32() {
  if (!state->i32_) {
    state->i32_ = state->allocator_.Create<spirv::I32>();
  }
  return state->i32_;
}

const spirv::Pointer* TypeManager::Pointer(const Type* el,
                                           ast::StorageClass sc) {
  return utils::GetOrCreate(state->pointers_, spirv::Pointer(el, sc), [&] {
    return state->allocator_.Create<spirv::Pointer>(el, sc);
  });
}

const spirv::Reference* TypeManager::Reference(const Type* el,
                                               ast::StorageClass sc) {
  return utils::GetOrCreate(state->references_, spirv::Reference(el, sc), [&] {
    return state->allocator_.Create<spirv::Reference>(el, sc);
  });
}

const spirv::Vector* TypeManager::Vector(const Type* el, uint32_t size) {
  return utils::GetOrCreate(state->vectors_, spirv::Vector(el, size), [&] {
    return state->allocator_.Create<spirv::Vector>(el, size);
  });
}

const spirv::Matrix* TypeManager::Matrix(const Type* el,
                                         uint32_t columns,
                                         uint32_t rows) {
  return utils::GetOrCreate(
      state->matrices_, spirv::Matrix(el, columns, rows), [&] {
        return state->allocator_.Create<spirv::Matrix>(el, columns, rows);
      });
}

const spirv::Array* TypeManager::Array(const Type* el,
                                       uint32_t size,
                                       uint32_t stride) {
  return utils::GetOrCreate(
      state->arrays_, spirv::Array(el, size, stride),
      [&] { return state->allocator_.Create<spirv::Array>(el, size, stride); });
}

const spirv::AccessControl* TypeManager::AccessControl(
    const Type* ty,
    ast::AccessControl::Access ac) {
  return utils::GetOrCreate(
      state->access_controls_, spirv::AccessControl(ty, ac),
      [&] { return state->allocator_.Create<spirv::AccessControl>(ty, ac); });
}

const spirv::Alias* TypeManager::Alias(Symbol name, const Type* ty) {
  return utils::GetOrCreate(state->aliases_, name, [&] {
    return state->allocator_.Create<spirv::Alias>(name, ty);
  });
}

const spirv::Struct* TypeManager::Struct(Symbol name, TypeList members) {
  return utils::GetOrCreate(state->structs_, name, [&] {
    return state->allocator_.Create<spirv::Struct>(name, std::move(members));
  });
}

const spirv::Sampler* TypeManager::Sampler(ast::SamplerKind kind) {
  return utils::GetOrCreate(state->samplers_, kind, [&] {
    return state->allocator_.Create<spirv::Sampler>(kind);
  });
}

const spirv::DepthTexture* TypeManager::DepthTexture(
    ast::TextureDimension dims) {
  return utils::GetOrCreate(state->depth_textures_, dims, [&] {
    return state->allocator_.Create<spirv::DepthTexture>(dims);
  });
}

const spirv::MultisampledTexture* TypeManager::MultisampledTexture(
    ast::TextureDimension dims,
    const Type* ty) {
  return utils::GetOrCreate(
      state->multisampled_textures_, spirv::MultisampledTexture(dims, ty), [&] {
        return state->allocator_.Create<spirv::MultisampledTexture>(dims, ty);
      });
}

const spirv::SampledTexture* TypeManager::SampledTexture(
    ast::TextureDimension dims,
    const Type* ty) {
  return utils::GetOrCreate(
      state->sampled_textures_, spirv::SampledTexture(dims, ty), [&] {
        return state->allocator_.Create<spirv::SampledTexture>(dims, ty);
      });
}

const spirv::StorageTexture* TypeManager::StorageTexture(
    ast::TextureDimension dims,
    ast::ImageFormat fmt) {
  return utils::GetOrCreate(
      state->storage_textures_, spirv::StorageTexture(dims, fmt), [&] {
        return state->allocator_.Create<spirv::StorageTexture>(dims, fmt);
      });
}

// Debug String() methods for Type classes. Only enabled in debug builds.
#ifndef NDEBUG
std::string Void::String() const {
  return "void";
}

std::string Bool::String() const {
  return "bool";
}

std::string U32::String() const {
  return "u32";
}

std::string F32::String() const {
  return "f32";
}

std::string I32::String() const {
  return "i32";
}

std::string Pointer::String() const {
  std::stringstream ss;
  ss << "ptr<" << std::string(ast::str(storage_class)) << ", "
     << type->String() + ">";
  return ss.str();
}

std::string Reference::String() const {
  std::stringstream ss;
  ss << "ref<" + std::string(ast::str(storage_class)) << ", " << type->String()
     << ">";
  return ss.str();
}

std::string Vector::String() const {
  std::stringstream ss;
  ss << "vec" << size << "<" << type->String() << ">";
  return ss.str();
}

std::string Matrix::String() const {
  std::stringstream ss;
  ss << "mat" << columns << "x" << rows << "<" << type->String() << ">";
  return ss.str();
}

std::string Array::String() const {
  std::stringstream ss;
  ss << "array<" << type->String() << ", " << size << ", " << stride << ">";
  return ss.str();
}

std::string AccessControl::String() const {
  std::stringstream ss;
  ss << "[[access(" << access << ")]] " << type->String();
  return ss.str();
}

std::string Sampler::String() const {
  switch (kind) {
    case ast::SamplerKind::kSampler:
      return "sampler";
    case ast::SamplerKind::kComparisonSampler:
      return "sampler_comparison";
  }
  return "<unknown sampler>";
}

std::string DepthTexture::String() const {
  std::stringstream ss;
  ss << "depth_" << dims;
  return ss.str();
}

std::string MultisampledTexture::String() const {
  std::stringstream ss;
  ss << "texture_multisampled_" << dims << "<" << type << ">";
  return ss.str();
}

std::string SampledTexture::String() const {
  std::stringstream ss;
  ss << "texture_" << dims << "<" << type << ">";
  return ss.str();
}

std::string StorageTexture::String() const {
  std::stringstream ss;
  ss << "texture_storage_" << dims << "<" << format << ">";
  return ss.str();
}

std::string Named::String() const {
  return name.to_str();
}
#endif  // NDEBUG

}  // namespace spirv
}  // namespace reader
}  // namespace tint
