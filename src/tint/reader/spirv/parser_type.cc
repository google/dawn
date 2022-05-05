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

#include "src/tint/reader/spirv/parser_type.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/unique_allocator.h"

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
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Sampler);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Texture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::DepthTexture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::DepthMultisampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::MultisampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::SampledTexture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::StorageTexture);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Named);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Alias);
TINT_INSTANTIATE_TYPEINFO(tint::reader::spirv::Struct);

namespace tint::reader::spirv {

namespace {
struct PointerHasher {
    size_t operator()(const Pointer& t) const { return utils::Hash(t.type, t.storage_class); }
};

struct ReferenceHasher {
    size_t operator()(const Reference& t) const { return utils::Hash(t.type, t.storage_class); }
};

struct VectorHasher {
    size_t operator()(const Vector& t) const { return utils::Hash(t.type, t.size); }
};

struct MatrixHasher {
    size_t operator()(const Matrix& t) const { return utils::Hash(t.type, t.columns, t.rows); }
};

struct ArrayHasher {
    size_t operator()(const Array& t) const { return utils::Hash(t.type, t.size, t.stride); }
};

struct AliasHasher {
    size_t operator()(const Alias& t) const { return utils::Hash(t.name); }
};

struct StructHasher {
    size_t operator()(const Struct& t) const { return utils::Hash(t.name); }
};

struct SamplerHasher {
    size_t operator()(const Sampler& s) const { return utils::Hash(s.kind); }
};

struct DepthTextureHasher {
    size_t operator()(const DepthTexture& t) const { return utils::Hash(t.dims); }
};

struct DepthMultisampledTextureHasher {
    size_t operator()(const DepthMultisampledTexture& t) const { return utils::Hash(t.dims); }
};

struct MultisampledTextureHasher {
    size_t operator()(const MultisampledTexture& t) const { return utils::Hash(t.dims, t.type); }
};

struct SampledTextureHasher {
    size_t operator()(const SampledTexture& t) const { return utils::Hash(t.dims, t.type); }
};

struct StorageTextureHasher {
    size_t operator()(const StorageTexture& t) const {
        return utils::Hash(t.dims, t.format, t.access);
    }
};
}  // namespace

// Equality operators
//! @cond Doxygen_Suppress
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
static bool operator==(const Named& a, const Named& b) {
    return a.name == b.name;
}
static bool operator==(const Sampler& a, const Sampler& b) {
    return a.kind == b.kind;
}
static bool operator==(const DepthTexture& a, const DepthTexture& b) {
    return a.dims == b.dims;
}
static bool operator==(const DepthMultisampledTexture& a, const DepthMultisampledTexture& b) {
    return a.dims == b.dims;
}
static bool operator==(const MultisampledTexture& a, const MultisampledTexture& b) {
    return a.dims == b.dims && a.type == b.type;
}
static bool operator==(const SampledTexture& a, const SampledTexture& b) {
    return a.dims == b.dims && a.type == b.type;
}
static bool operator==(const StorageTexture& a, const StorageTexture& b) {
    return a.dims == b.dims && a.format == b.format;
}
//! @endcond

const ast::Type* Void::Build(ProgramBuilder& b) const {
    return b.ty.void_();
}

const ast::Type* Bool::Build(ProgramBuilder& b) const {
    return b.ty.bool_();
}

const ast::Type* U32::Build(ProgramBuilder& b) const {
    return b.ty.u32();
}

const ast::Type* F32::Build(ProgramBuilder& b) const {
    return b.ty.f32();
}

const ast::Type* I32::Build(ProgramBuilder& b) const {
    return b.ty.i32();
}

Pointer::Pointer(const Type* t, ast::StorageClass s) : type(t), storage_class(s) {}
Pointer::Pointer(const Pointer&) = default;

const ast::Type* Pointer::Build(ProgramBuilder& b) const {
    return b.ty.pointer(type->Build(b), storage_class);
}

Reference::Reference(const Type* t, ast::StorageClass s) : type(t), storage_class(s) {}
Reference::Reference(const Reference&) = default;

const ast::Type* Reference::Build(ProgramBuilder& b) const {
    return type->Build(b);
}

Vector::Vector(const Type* t, uint32_t s) : type(t), size(s) {}
Vector::Vector(const Vector&) = default;

const ast::Type* Vector::Build(ProgramBuilder& b) const {
    return b.ty.vec(type->Build(b), size);
}

Matrix::Matrix(const Type* t, uint32_t c, uint32_t r) : type(t), columns(c), rows(r) {}
Matrix::Matrix(const Matrix&) = default;

const ast::Type* Matrix::Build(ProgramBuilder& b) const {
    return b.ty.mat(type->Build(b), columns, rows);
}

Array::Array(const Type* t, uint32_t sz, uint32_t st) : type(t), size(sz), stride(st) {}
Array::Array(const Array&) = default;

const ast::Type* Array::Build(ProgramBuilder& b) const {
    if (size > 0) {
        return b.ty.array(type->Build(b), u32(size), stride);
    } else {
        return b.ty.array(type->Build(b), nullptr, stride);
    }
}

Sampler::Sampler(ast::SamplerKind k) : kind(k) {}
Sampler::Sampler(const Sampler&) = default;

const ast::Type* Sampler::Build(ProgramBuilder& b) const {
    return b.ty.sampler(kind);
}

Texture::Texture(ast::TextureDimension d) : dims(d) {}
Texture::Texture(const Texture&) = default;

DepthTexture::DepthTexture(ast::TextureDimension d) : Base(d) {}
DepthTexture::DepthTexture(const DepthTexture&) = default;

const ast::Type* DepthTexture::Build(ProgramBuilder& b) const {
    return b.ty.depth_texture(dims);
}

DepthMultisampledTexture::DepthMultisampledTexture(ast::TextureDimension d) : Base(d) {}
DepthMultisampledTexture::DepthMultisampledTexture(const DepthMultisampledTexture&) = default;

const ast::Type* DepthMultisampledTexture::Build(ProgramBuilder& b) const {
    return b.ty.depth_multisampled_texture(dims);
}

MultisampledTexture::MultisampledTexture(ast::TextureDimension d, const Type* t)
    : Base(d), type(t) {}
MultisampledTexture::MultisampledTexture(const MultisampledTexture&) = default;

const ast::Type* MultisampledTexture::Build(ProgramBuilder& b) const {
    return b.ty.multisampled_texture(dims, type->Build(b));
}

SampledTexture::SampledTexture(ast::TextureDimension d, const Type* t) : Base(d), type(t) {}
SampledTexture::SampledTexture(const SampledTexture&) = default;

const ast::Type* SampledTexture::Build(ProgramBuilder& b) const {
    return b.ty.sampled_texture(dims, type->Build(b));
}

StorageTexture::StorageTexture(ast::TextureDimension d, ast::TexelFormat f, ast::Access a)
    : Base(d), format(f), access(a) {}
StorageTexture::StorageTexture(const StorageTexture&) = default;

const ast::Type* StorageTexture::Build(ProgramBuilder& b) const {
    return b.ty.storage_texture(dims, format, access);
}

Named::Named(Symbol n) : name(n) {}
Named::Named(const Named&) = default;
Named::~Named() = default;

Alias::Alias(Symbol n, const Type* ty) : Base(n), type(ty) {}
Alias::Alias(const Alias&) = default;

const ast::Type* Alias::Build(ProgramBuilder& b) const {
    return b.ty.type_name(name);
}

Struct::Struct(Symbol n, TypeList m) : Base(n), members(std::move(m)) {}
Struct::Struct(const Struct&) = default;
Struct::~Struct() = default;

const ast::Type* Struct::Build(ProgramBuilder& b) const {
    return b.ty.type_name(name);
}

/// The PIMPL state of the Types object.
struct TypeManager::State {
    /// The allocator of primitive types
    utils::BlockAllocator<Type> allocator_;
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
    /// Unique Pointer instances
    utils::UniqueAllocator<spirv::Pointer, PointerHasher> pointers_;
    /// Unique Reference instances
    utils::UniqueAllocator<spirv::Reference, ReferenceHasher> references_;
    /// Unique Vector instances
    utils::UniqueAllocator<spirv::Vector, VectorHasher> vectors_;
    /// Unique Matrix instances
    utils::UniqueAllocator<spirv::Matrix, MatrixHasher> matrices_;
    /// Unique Array instances
    utils::UniqueAllocator<spirv::Array, ArrayHasher> arrays_;
    /// Unique Alias instances
    utils::UniqueAllocator<spirv::Alias, AliasHasher> aliases_;
    /// Unique Struct instances
    utils::UniqueAllocator<spirv::Struct, StructHasher> structs_;
    /// Unique Sampler instances
    utils::UniqueAllocator<spirv::Sampler, SamplerHasher> samplers_;
    /// Unique DepthTexture instances
    utils::UniqueAllocator<spirv::DepthTexture, DepthTextureHasher> depth_textures_;
    /// Unique DepthMultisampledTexture instances
    utils::UniqueAllocator<spirv::DepthMultisampledTexture, DepthMultisampledTextureHasher>
        depth_multisampled_textures_;
    /// Unique MultisampledTexture instances
    utils::UniqueAllocator<spirv::MultisampledTexture, MultisampledTextureHasher>
        multisampled_textures_;
    /// Unique SampledTexture instances
    utils::UniqueAllocator<spirv::SampledTexture, SampledTextureHasher> sampled_textures_;
    /// Unique StorageTexture instances
    utils::UniqueAllocator<spirv::StorageTexture, StorageTextureHasher> storage_textures_;
};

const Type* Type::UnwrapPtr() const {
    const Type* type = this;
    while (auto* ptr = type->As<Pointer>()) {
        type = ptr->type;
    }
    return type;
}

const Type* Type::UnwrapRef() const {
    const Type* type = this;
    while (auto* ptr = type->As<Reference>()) {
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

const Type* Type::UnwrapAll() const {
    auto* type = this;
    while (true) {
        if (auto* alias = type->As<Alias>()) {
            type = alias->type;
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
    return Is([](const Vector* v) { return v->type->IsFloatScalar(); });
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
    return Is([](const Vector* v) { return v->type->Is<I32>(); });
}

bool Type::IsSignedScalarOrVector() const {
    return Is<I32>() || IsSignedIntegerVector();
}

bool Type::IsUnsignedIntegerVector() const {
    return Is([](const Vector* v) { return v->type->Is<U32>(); });
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

const spirv::Pointer* TypeManager::Pointer(const Type* el, ast::StorageClass sc) {
    return state->pointers_.Get(el, sc);
}

const spirv::Reference* TypeManager::Reference(const Type* el, ast::StorageClass sc) {
    return state->references_.Get(el, sc);
}

const spirv::Vector* TypeManager::Vector(const Type* el, uint32_t size) {
    return state->vectors_.Get(el, size);
}

const spirv::Matrix* TypeManager::Matrix(const Type* el, uint32_t columns, uint32_t rows) {
    return state->matrices_.Get(el, columns, rows);
}

const spirv::Array* TypeManager::Array(const Type* el, uint32_t size, uint32_t stride) {
    return state->arrays_.Get(el, size, stride);
}

const spirv::Alias* TypeManager::Alias(Symbol name, const Type* ty) {
    return state->aliases_.Get(name, ty);
}

const spirv::Struct* TypeManager::Struct(Symbol name, TypeList members) {
    return state->structs_.Get(name, std::move(members));
}

const spirv::Sampler* TypeManager::Sampler(ast::SamplerKind kind) {
    return state->samplers_.Get(kind);
}

const spirv::DepthTexture* TypeManager::DepthTexture(ast::TextureDimension dims) {
    return state->depth_textures_.Get(dims);
}

const spirv::DepthMultisampledTexture* TypeManager::DepthMultisampledTexture(
    ast::TextureDimension dims) {
    return state->depth_multisampled_textures_.Get(dims);
}

const spirv::MultisampledTexture* TypeManager::MultisampledTexture(ast::TextureDimension dims,
                                                                   const Type* ty) {
    return state->multisampled_textures_.Get(dims, ty);
}

const spirv::SampledTexture* TypeManager::SampledTexture(ast::TextureDimension dims,
                                                         const Type* ty) {
    return state->sampled_textures_.Get(dims, ty);
}

const spirv::StorageTexture* TypeManager::StorageTexture(ast::TextureDimension dims,
                                                         ast::TexelFormat fmt,
                                                         ast::Access access) {
    return state->storage_textures_.Get(dims, fmt, access);
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
    ss << "ptr<" << std::string(ast::ToString(storage_class)) << ", " << type->String() + ">";
    return ss.str();
}

std::string Reference::String() const {
    std::stringstream ss;
    ss << "ref<" + std::string(ast::ToString(storage_class)) << ", " << type->String() << ">";
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

std::string DepthMultisampledTexture::String() const {
    std::stringstream ss;
    ss << "depth_multisampled_" << dims;
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
    ss << "texture_storage_" << dims << "<" << format << ", " << access << ">";
    return ss.str();
}

std::string Named::String() const {
    return name.to_str();
}
#endif  // NDEBUG

}  // namespace tint::reader::spirv
