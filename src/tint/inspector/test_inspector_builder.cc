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

#include "src/tint/inspector/test_inspector_builder.h"

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

namespace tint::inspector {

InspectorBuilder::InspectorBuilder() = default;
InspectorBuilder::~InspectorBuilder() = default;

void InspectorBuilder::MakeEmptyBodyFunction(std::string name,
                                             utils::VectorRef<const ast::Attribute*> attributes) {
    Func(name, utils::Empty, ty.void_(), utils::Vector{Return()}, attributes);
}

void InspectorBuilder::MakeCallerBodyFunction(std::string caller,
                                              utils::VectorRef<std::string> callees,
                                              utils::VectorRef<const ast::Attribute*> attributes) {
    utils::Vector<const ast::Statement*, 8> body;
    body.Reserve(callees.Length() + 1);
    for (auto callee : callees) {
        body.Push(CallStmt(Call(callee)));
    }
    body.Push(Return());

    Func(caller, utils::Empty, ty.void_(), body, attributes);
}

const ast::Struct* InspectorBuilder::MakeInOutStruct(std::string name,
                                                     utils::VectorRef<InOutInfo> inout_vars) {
    utils::Vector<const ast::StructMember*, 8> members;
    for (auto var : inout_vars) {
        std::string member_name;
        uint32_t location;
        std::tie(member_name, location) = var;
        members.Push(Member(member_name, ty.u32(),
                            utils::Vector{
                                Location(AInt(location)),
                                Flat(),
                            }));
    }
    return Structure(name, members);
}

const ast::Function* InspectorBuilder::MakePlainGlobalReferenceBodyFunction(
    std::string func,
    std::string var,
    const ast::Type* type,
    utils::VectorRef<const ast::Attribute*> attributes) {
    utils::Vector<const ast::Statement*, 3> stmts;
    stmts.Push(Decl(Var("local_" + var, type)));
    stmts.Push(Assign("local_" + var, var));
    stmts.Push(Return());
    return Func(func, utils::Empty, ty.void_(), std::move(stmts), std::move(attributes));
}

bool InspectorBuilder::ContainsName(utils::VectorRef<StageVariable> vec, const std::string& name) {
    for (auto& s : vec) {
        if (s.name == name) {
            return true;
        }
    }
    return false;
}

std::string InspectorBuilder::StructMemberName(size_t idx, const ast::Type* type) {
    return std::to_string(idx) + type->FriendlyName(Symbols());
}

const ast::Struct* InspectorBuilder::MakeStructType(
    const std::string& name,
    utils::VectorRef<const ast::Type*> member_types) {
    utils::Vector<const ast::StructMember*, 8> members;
    for (auto* type : member_types) {
        members.Push(MakeStructMember(members.Length(), type, {}));
    }
    return MakeStructTypeFromMembers(name, std::move(members));
}

const ast::Struct* InspectorBuilder::MakeStructTypeFromMembers(
    const std::string& name,
    utils::VectorRef<const ast::StructMember*> members) {
    return Structure(name, std::move(members));
}

const ast::StructMember* InspectorBuilder::MakeStructMember(
    size_t index,
    const ast::Type* type,
    utils::VectorRef<const ast::Attribute*> attributes) {
    return Member(StructMemberName(index, type), type, std::move(attributes));
}

const ast::Struct* InspectorBuilder::MakeUniformBufferType(
    const std::string& name,
    utils::VectorRef<const ast::Type*> member_types) {
    return MakeStructType(name, member_types);
}

std::function<const ast::TypeName*()> InspectorBuilder::MakeStorageBufferTypes(
    const std::string& name,
    utils::VectorRef<const ast::Type*> member_types) {
    MakeStructType(name, member_types);
    return [this, name] { return ty.type_name(name); };
}

void InspectorBuilder::AddUniformBuffer(const std::string& name,
                                        const ast::Type* type,
                                        uint32_t group,
                                        uint32_t binding) {
    GlobalVar(name, type, ast::AddressSpace::kUniform, Binding(AInt(binding)), Group(AInt(group)));
}

void InspectorBuilder::AddWorkgroupStorage(const std::string& name, const ast::Type* type) {
    GlobalVar(name, type, ast::AddressSpace::kWorkgroup);
}

void InspectorBuilder::AddStorageBuffer(const std::string& name,
                                        const ast::Type* type,
                                        ast::Access access,
                                        uint32_t group,
                                        uint32_t binding) {
    GlobalVar(name, type, ast::AddressSpace::kStorage, access, Binding(AInt(binding)),
              Group(AInt(group)));
}

void InspectorBuilder::MakeStructVariableReferenceBodyFunction(
    std::string func_name,
    std::string struct_name,
    utils::VectorRef<std::tuple<size_t, const ast::Type*>> members) {
    utils::Vector<const ast::Statement*, 8> stmts;
    for (auto member : members) {
        size_t member_idx;
        const ast::Type* member_type;
        std::tie(member_idx, member_type) = member;
        std::string member_name = StructMemberName(member_idx, member_type);

        stmts.Push(Decl(Var("local" + member_name, member_type)));
    }

    for (auto member : members) {
        size_t member_idx;
        const ast::Type* member_type;
        std::tie(member_idx, member_type) = member;
        std::string member_name = StructMemberName(member_idx, member_type);

        stmts.Push(Assign("local" + member_name, MemberAccessor(struct_name, member_name)));
    }

    stmts.Push(Return());

    Func(func_name, utils::Empty, ty.void_(), stmts);
}

void InspectorBuilder::AddSampler(const std::string& name, uint32_t group, uint32_t binding) {
    GlobalVar(name, sampler_type(), Binding(AInt(binding)), Group(AInt(group)));
}

void InspectorBuilder::AddComparisonSampler(const std::string& name,
                                            uint32_t group,
                                            uint32_t binding) {
    GlobalVar(name, comparison_sampler_type(), Binding(AInt(binding)), Group(AInt(group)));
}

void InspectorBuilder::AddResource(const std::string& name,
                                   const ast::Type* type,
                                   uint32_t group,
                                   uint32_t binding) {
    GlobalVar(name, type, Binding(AInt(binding)), Group(AInt(group)));
}

void InspectorBuilder::AddGlobalVariable(const std::string& name, const ast::Type* type) {
    GlobalVar(name, type, ast::AddressSpace::kPrivate);
}

const ast::Function* InspectorBuilder::MakeSamplerReferenceBodyFunction(
    const std::string& func_name,
    const std::string& texture_name,
    const std::string& sampler_name,
    const std::string& coords_name,
    const ast::Type* base_type,
    utils::VectorRef<const ast::Attribute*> attributes) {
    std::string result_name = "sampler_result";

    utils::Vector stmts{
        Decl(Var(result_name, ty.vec(base_type, 4))),
        Assign(result_name, Call("textureSample", texture_name, sampler_name, coords_name)),
        Return(),
    };
    return Func(func_name, utils::Empty, ty.void_(), std::move(stmts), std::move(attributes));
}

const ast::Function* InspectorBuilder::MakeSamplerReferenceBodyFunction(
    const std::string& func_name,
    const std::string& texture_name,
    const std::string& sampler_name,
    const std::string& coords_name,
    const std::string& array_index,
    const ast::Type* base_type,
    utils::VectorRef<const ast::Attribute*> attributes) {
    std::string result_name = "sampler_result";

    utils::Vector stmts{
        Decl(Var("sampler_result", ty.vec(base_type, 4))),
        Assign("sampler_result",
               Call("textureSample", texture_name, sampler_name, coords_name, array_index)),
        Return(),
    };
    return Func(func_name, utils::Empty, ty.void_(), std::move(stmts), std::move(attributes));
}

const ast::Function* InspectorBuilder::MakeComparisonSamplerReferenceBodyFunction(
    const std::string& func_name,
    const std::string& texture_name,
    const std::string& sampler_name,
    const std::string& coords_name,
    const std::string& depth_name,
    const ast::Type* base_type,
    utils::VectorRef<const ast::Attribute*> attributes) {
    std::string result_name = "sampler_result";

    utils::Vector stmts{
        Decl(Var("sampler_result", base_type)),
        Assign("sampler_result",
               Call("textureSampleCompare", texture_name, sampler_name, coords_name, depth_name)),
        Return(),
    };
    return Func(func_name, utils::Empty, ty.void_(), std::move(stmts), std::move(attributes));
}

const ast::Type* InspectorBuilder::GetBaseType(ResourceBinding::SampledKind sampled_kind) {
    switch (sampled_kind) {
        case ResourceBinding::SampledKind::kFloat:
            return ty.f32();
        case ResourceBinding::SampledKind::kSInt:
            return ty.i32();
        case ResourceBinding::SampledKind::kUInt:
            return ty.u32();
        default:
            return nullptr;
    }
}

const ast::Type* InspectorBuilder::GetCoordsType(ast::TextureDimension dim,
                                                 const ast::Type* scalar) {
    switch (dim) {
        case ast::TextureDimension::k1d:
            return scalar;
        case ast::TextureDimension::k2d:
        case ast::TextureDimension::k2dArray:
            return create<ast::Vector>(scalar, 2u);
        case ast::TextureDimension::k3d:
        case ast::TextureDimension::kCube:
        case ast::TextureDimension::kCubeArray:
            return create<ast::Vector>(scalar, 3u);
        default:
            [=]() { FAIL() << "Unsupported texture dimension: " << dim; }();
    }
    return nullptr;
}

const ast::Type* InspectorBuilder::MakeStorageTextureTypes(ast::TextureDimension dim,
                                                           ast::TexelFormat format) {
    return ty.storage_texture(dim, format, ast::Access::kWrite);
}

void InspectorBuilder::AddStorageTexture(const std::string& name,
                                         const ast::Type* type,
                                         uint32_t group,
                                         uint32_t binding) {
    GlobalVar(name, type, Binding(AInt(binding)), Group(AInt(group)));
}

const ast::Function* InspectorBuilder::MakeStorageTextureBodyFunction(
    const std::string& func_name,
    const std::string& st_name,
    const ast::Type* dim_type,
    utils::VectorRef<const ast::Attribute*> attributes) {
    utils::Vector stmts{
        Decl(Var("dim", dim_type)),
        Assign("dim", Call("textureDimensions", st_name)),
        Return(),
    };

    return Func(func_name, utils::Empty, ty.void_(), std::move(stmts), std::move(attributes));
}

std::function<const ast::Type*()> InspectorBuilder::GetTypeFunction(ComponentType component,
                                                                    CompositionType composition) {
    std::function<const ast::Type*()> func;
    switch (component) {
        case ComponentType::kF32:
            func = [this]() -> const ast::Type* { return ty.f32(); };
            break;
        case ComponentType::kI32:
            func = [this]() -> const ast::Type* { return ty.i32(); };
            break;
        case ComponentType::kU32:
            func = [this]() -> const ast::Type* { return ty.u32(); };
            break;
        case ComponentType::kF16:
            func = [this]() -> const ast::Type* { return ty.f16(); };
            break;
        case ComponentType::kUnknown:
            return []() -> const ast::Type* { return nullptr; };
    }

    uint32_t n;
    switch (composition) {
        case CompositionType::kScalar:
            return func;
        case CompositionType::kVec2:
            n = 2;
            break;
        case CompositionType::kVec3:
            n = 3;
            break;
        case CompositionType::kVec4:
            n = 4;
            break;
        default:
            return []() -> ast::Type* { return nullptr; };
    }

    return [this, func, n]() -> const ast::Type* { return ty.vec(func(), n); };
}

Inspector& InspectorBuilder::Build() {
    if (inspector_) {
        return *inspector_;
    }
    program_ = std::make_unique<Program>(std::move(*this));
    [&]() {
        ASSERT_TRUE(program_->IsValid()) << diag::Formatter().format(program_->Diagnostics());
    }();
    inspector_ = std::make_unique<Inspector>(program_.get());
    return *inspector_;
}

}  // namespace tint::inspector
