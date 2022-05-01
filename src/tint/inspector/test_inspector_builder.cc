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

void InspectorBuilder::MakeEmptyBodyFunction(std::string name, ast::AttributeList attributes) {
    Func(name, ast::VariableList(), ty.void_(), ast::StatementList{Return()}, attributes);
}

void InspectorBuilder::MakeCallerBodyFunction(std::string caller,
                                              std::vector<std::string> callees,
                                              ast::AttributeList attributes) {
    ast::StatementList body;
    body.reserve(callees.size() + 1);
    for (auto callee : callees) {
        body.push_back(CallStmt(Call(callee)));
    }
    body.push_back(Return());

    Func(caller, ast::VariableList(), ty.void_(), body, attributes);
}

const ast::Struct* InspectorBuilder::MakeInOutStruct(
    std::string name,
    std::vector<std::tuple<std::string, uint32_t>> inout_vars) {
    ast::StructMemberList members;
    for (auto var : inout_vars) {
        std::string member_name;
        uint32_t location;
        std::tie(member_name, location) = var;
        members.push_back(Member(member_name, ty.u32(), {Location(location), Flat()}));
    }
    return Structure(name, members);
}

const ast::Function* InspectorBuilder::MakePlainGlobalReferenceBodyFunction(
    std::string func,
    std::string var,
    const ast::Type* type,
    ast::AttributeList attributes) {
    ast::StatementList stmts;
    stmts.emplace_back(Decl(Var("local_" + var, type)));
    stmts.emplace_back(Assign("local_" + var, var));
    stmts.emplace_back(Return());

    return Func(func, ast::VariableList(), ty.void_(), stmts, attributes);
}

bool InspectorBuilder::ContainsName(const std::vector<StageVariable>& vec,
                                    const std::string& name) {
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

const ast::Struct* InspectorBuilder::MakeStructType(const std::string& name,
                                                    std::vector<const ast::Type*> member_types) {
    ast::StructMemberList members;
    for (auto* type : member_types) {
        members.push_back(MakeStructMember(members.size(), type, {}));
    }
    return MakeStructTypeFromMembers(name, std::move(members));
}

const ast::Struct* InspectorBuilder::MakeStructTypeFromMembers(const std::string& name,
                                                               ast::StructMemberList members) {
    return Structure(name, std::move(members));
}

const ast::StructMember* InspectorBuilder::MakeStructMember(size_t index,
                                                            const ast::Type* type,
                                                            ast::AttributeList attributes) {
    return Member(StructMemberName(index, type), type, std::move(attributes));
}

const ast::Struct* InspectorBuilder::MakeUniformBufferType(
    const std::string& name,
    std::vector<const ast::Type*> member_types) {
    return MakeStructType(name, member_types);
}

std::function<const ast::TypeName*()> InspectorBuilder::MakeStorageBufferTypes(
    const std::string& name,
    std::vector<const ast::Type*> member_types) {
    MakeStructType(name, member_types);
    return [this, name] { return ty.type_name(name); };
}

void InspectorBuilder::AddUniformBuffer(const std::string& name,
                                        const ast::Type* type,
                                        uint32_t group,
                                        uint32_t binding) {
    Global(name, type, ast::StorageClass::kUniform,
           ast::AttributeList{
               create<ast::BindingAttribute>(binding),
               create<ast::GroupAttribute>(group),
           });
}

void InspectorBuilder::AddWorkgroupStorage(const std::string& name, const ast::Type* type) {
    Global(name, type, ast::StorageClass::kWorkgroup);
}

void InspectorBuilder::AddStorageBuffer(const std::string& name,
                                        const ast::Type* type,
                                        ast::Access access,
                                        uint32_t group,
                                        uint32_t binding) {
    Global(name, type, ast::StorageClass::kStorage, access,
           ast::AttributeList{
               create<ast::BindingAttribute>(binding),
               create<ast::GroupAttribute>(group),
           });
}

void InspectorBuilder::MakeStructVariableReferenceBodyFunction(
    std::string func_name,
    std::string struct_name,
    std::vector<std::tuple<size_t, const ast::Type*>> members) {
    ast::StatementList stmts;
    for (auto member : members) {
        size_t member_idx;
        const ast::Type* member_type;
        std::tie(member_idx, member_type) = member;
        std::string member_name = StructMemberName(member_idx, member_type);

        stmts.emplace_back(Decl(Var("local" + member_name, member_type)));
    }

    for (auto member : members) {
        size_t member_idx;
        const ast::Type* member_type;
        std::tie(member_idx, member_type) = member;
        std::string member_name = StructMemberName(member_idx, member_type);

        stmts.emplace_back(Assign("local" + member_name, MemberAccessor(struct_name, member_name)));
    }

    stmts.emplace_back(Return());

    Func(func_name, ast::VariableList(), ty.void_(), stmts, ast::AttributeList{});
}

void InspectorBuilder::AddSampler(const std::string& name, uint32_t group, uint32_t binding) {
    Global(name, sampler_type(),
           ast::AttributeList{
               create<ast::BindingAttribute>(binding),
               create<ast::GroupAttribute>(group),
           });
}

void InspectorBuilder::AddComparisonSampler(const std::string& name,
                                            uint32_t group,
                                            uint32_t binding) {
    Global(name, comparison_sampler_type(),
           ast::AttributeList{
               create<ast::BindingAttribute>(binding),
               create<ast::GroupAttribute>(group),
           });
}

void InspectorBuilder::AddResource(const std::string& name,
                                   const ast::Type* type,
                                   uint32_t group,
                                   uint32_t binding) {
    Global(name, type,
           ast::AttributeList{
               create<ast::BindingAttribute>(binding),
               create<ast::GroupAttribute>(group),
           });
}

void InspectorBuilder::AddGlobalVariable(const std::string& name, const ast::Type* type) {
    Global(name, type, ast::StorageClass::kPrivate);
}

const ast::Function* InspectorBuilder::MakeSamplerReferenceBodyFunction(
    const std::string& func_name,
    const std::string& texture_name,
    const std::string& sampler_name,
    const std::string& coords_name,
    const ast::Type* base_type,
    ast::AttributeList attributes) {
    std::string result_name = "sampler_result";

    ast::StatementList stmts;
    stmts.emplace_back(Decl(Var(result_name, ty.vec(base_type, 4))));

    stmts.emplace_back(
        Assign(result_name, Call("textureSample", texture_name, sampler_name, coords_name)));
    stmts.emplace_back(Return());

    return Func(func_name, ast::VariableList(), ty.void_(), stmts, attributes);
}

const ast::Function* InspectorBuilder::MakeSamplerReferenceBodyFunction(
    const std::string& func_name,
    const std::string& texture_name,
    const std::string& sampler_name,
    const std::string& coords_name,
    const std::string& array_index,
    const ast::Type* base_type,
    ast::AttributeList attributes) {
    std::string result_name = "sampler_result";

    ast::StatementList stmts;

    stmts.emplace_back(Decl(Var("sampler_result", ty.vec(base_type, 4))));

    stmts.emplace_back(Assign("sampler_result", Call("textureSample", texture_name, sampler_name,
                                                     coords_name, array_index)));
    stmts.emplace_back(Return());

    return Func(func_name, ast::VariableList(), ty.void_(), stmts, attributes);
}

const ast::Function* InspectorBuilder::MakeComparisonSamplerReferenceBodyFunction(
    const std::string& func_name,
    const std::string& texture_name,
    const std::string& sampler_name,
    const std::string& coords_name,
    const std::string& depth_name,
    const ast::Type* base_type,
    ast::AttributeList attributes) {
    std::string result_name = "sampler_result";

    ast::StatementList stmts;

    stmts.emplace_back(Decl(Var("sampler_result", base_type)));
    stmts.emplace_back(Assign("sampler_result", Call("textureSampleCompare", texture_name,
                                                     sampler_name, coords_name, depth_name)));
    stmts.emplace_back(Return());

    return Func(func_name, ast::VariableList(), ty.void_(), stmts, attributes);
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
            return create<ast::Vector>(scalar, 2);
        case ast::TextureDimension::k3d:
        case ast::TextureDimension::kCube:
        case ast::TextureDimension::kCubeArray:
            return create<ast::Vector>(scalar, 3);
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
    Global(name, type,
           ast::AttributeList{
               create<ast::BindingAttribute>(binding),
               create<ast::GroupAttribute>(group),
           });
}

const ast::Function* InspectorBuilder::MakeStorageTextureBodyFunction(
    const std::string& func_name,
    const std::string& st_name,
    const ast::Type* dim_type,
    ast::AttributeList attributes) {
    ast::StatementList stmts;

    stmts.emplace_back(Decl(Var("dim", dim_type)));
    stmts.emplace_back(Assign("dim", Call("textureDimensions", st_name)));
    stmts.emplace_back(Return());

    return Func(func_name, ast::VariableList(), ty.void_(), stmts, attributes);
}

std::function<const ast::Type*()> InspectorBuilder::GetTypeFunction(ComponentType component,
                                                                    CompositionType composition) {
    std::function<const ast::Type*()> func;
    switch (component) {
        case ComponentType::kFloat:
            func = [this]() -> const ast::Type* { return ty.f32(); };
            break;
        case ComponentType::kSInt:
            func = [this]() -> const ast::Type* { return ty.i32(); };
            break;
        case ComponentType::kUInt:
            func = [this]() -> const ast::Type* { return ty.u32(); };
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
