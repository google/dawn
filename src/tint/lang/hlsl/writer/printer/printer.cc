// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/hlsl/writer/printer/printer.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/constant/value.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/value.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/array_count.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/generator/text_generator.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/strconv/float_to_string.h"
#include "src/tint/utils/text/string.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::hlsl::writer {
namespace {

/// PIMPL class for the HLSL generator
class Printer : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param module the IR module to generate
    explicit Printer(core::ir::Module& module) : ir_(module) {}

    /// @returns the generated HLSL shader
    tint::Result<PrintResult> Generate() {
        auto valid = core::ir::ValidateAndDumpIfNeeded(ir_, "HLSL writer");
        if (valid != Success) {
            return std::move(valid.Failure());
        }

        // TOOD(dsinclair): EmitRootBlock

        // Emit functions.
        for (auto* func : ir_.DependencyOrderedFunctions()) {
            EmitFunction(func);
        }

        result_.hlsl = main_buffer_.String();
        return std::move(result_);
    }

  private:
    /// The result of printing the module.
    PrintResult result_;

    core::ir::Module& ir_;

    /// A hashmap of value to name
    Hashmap<const core::ir::Value*, std::string, 32> names_;
    /// Map of builtin structure to unique generated name
    std::unordered_map<const core::type::Struct*, std::string> builtin_struct_names_;

    /// The current function being emitted
    const core::ir::Function* current_function_ = nullptr;
    /// The current block being emitted
    const core::ir::Block* current_block_ = nullptr;

    void EmitFunction(const core::ir::Function* func) {
        TINT_SCOPED_ASSIGNMENT(current_function_, func);

        {
            auto out = Line();
            auto func_name = NameOf(func);

            // TODO(dsinclair): Pipeline stage and workgroup information

            EmitType(out, func->ReturnType());
            out << " " << func_name << "(";

            // TODO(dsinclair): Parameters

            out << ") {";
        }
        {
            const ScopedIndent si(current_buffer_);
            EmitBlock(func->Block());
        }

        Line() << "}";
    }

    void EmitBlock(const core::ir::Block* block) {
        TINT_SCOPED_ASSIGNMENT(current_block_, block);

        for (auto* inst : *block) {
            Switch(
                inst,                                               //
                [&](const core::ir::Var* v) { EmitVar(v); },        //
                [&](const core::ir::Let* i) { EmitLet(i); },        //
                [&](const core::ir::Return* i) { EmitReturn(i); },  //
                TINT_ICE_ON_NO_MATCH);
        }
    }

    void EmitVar(const core::ir::Var* var) {
        auto out = Line();

        // TODO(dsinclair): This isn't right, as some types contain their names
        EmitType(out, var->Result(0)->Type());
        out << " ";
        out << NameOf(var->Result(0));

        out << " = ";

        // TODO(dsinclair): Add transform to create a 0-initializer if one not present
        EmitValue(out, var->Initializer());
        out << ";";
    }

    void EmitLet(const core::ir::Let* l) {
        auto out = Line();

        // TODO(dsinclair): This isn't right, as some types contain their names.
        // TODO(dsinclair): Investigate using `const` here as well, the AST printer doesn't emit
        //                  const with a let, but we should be able to.
        EmitType(out, l->Result(0)->Type());
        out << " ";
        out << NameOf(l->Result(0));
        out << " = ";
        EmitValue(out, l->Value());
        out << ";";
    }

    void EmitReturn(const core::ir::Return* r) {
        // If this return has no arguments and the current block is for the function which is
        // being returned, skip the return.
        if (current_block_ == current_function_->Block() && r->Args().IsEmpty()) {
            return;
        }

        auto out = Line();
        out << "return";
        if (!r->Args().IsEmpty()) {
            out << " ";
            EmitValue(out, r->Args().Front());
        }
        out << ";";
    }

    void EmitValue(StringStream& out, const core::ir::Value* v) {
        Switch(
            v,                                                           //
            [&](const core::ir::Constant* c) { EmitConstant(out, c); },  //
            [&](const core::ir::InstructionResult* r) {
                Switch(
                    r->Instruction(),
                    [&](const core::ir::Let* l) { out << NameOf(l->Result(0)); },  //
                    TINT_ICE_ON_NO_MATCH);
            },
            TINT_ICE_ON_NO_MATCH);
    }

    void EmitConstant(StringStream& out, const core::ir::Constant* c) {
        EmitConstant(out, c->Value());
    }

    void EmitConstant(StringStream& out, const core::constant::Value* c) {
        Switch(
            c->Type(),  //
            [&](const core::type::Bool*) { out << (c->ValueAs<AInt>() ? "true" : "false"); },
            [&](const core::type::F16*) { EmitConstantF16(out, c); },
            [&](const core::type::F32*) { PrintF32(out, c->ValueAs<f32>()); },
            [&](const core::type::I32*) { out << c->ValueAs<i32>(); },
            [&](const core::type::U32*) { out << c->ValueAs<AInt>() << "u"; },
            [&](const core::type::Array* a) { EmitConstantArray(out, c, a); },
            [&](const core::type::Vector* v) { EmitConstantVector(out, c, v); },
            [&](const core::type::Matrix* m) { EmitConstantMatrix(out, c, m); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    void EmitConstantF16(StringStream& out, const core::constant::Value* c) {
        // Emit a f16 scalar with explicit float16_t type declaration.
        out << "float16_t";
        const ScopedParen sp(out);
        PrintF16(out, c->ValueAs<f16>());
    }

    void EmitConstantArray(StringStream& out,
                           const core::constant::Value* c,
                           const core::type::Array* a) {
        if (c->AllZero()) {
            out << "(";
            EmitType(out, a);
            out << ")0";
            return;
        }

        out << "{";

        auto count = a->ConstantCount();
        TINT_ASSERT(count.has_value() && count.value() > 0);

        for (size_t i = 0; i < count; i++) {
            if (i > 0) {
                out << ", ";
            }
            EmitConstant(out, c->Index(i));
        }

        out << "}";
    }

    void EmitConstantVector(StringStream& out,
                            const core::constant::Value* c,
                            const core::type::Vector* v) {
        if (auto* splat = c->As<core::constant::Splat>()) {
            {
                const ScopedParen sp(out);
                EmitConstant(out, splat->el);
            }
            out << ".";
            for (size_t i = 0; i < v->Width(); i++) {
                out << "x";
            }
            return;
        }

        EmitType(out, v);

        const ScopedParen sp(out);
        for (size_t i = 0; i < v->Width(); i++) {
            if (i > 0) {
                out << ", ";
            }
            EmitConstant(out, c->Index(i));
        }
    }

    void EmitConstantMatrix(StringStream& out,
                            const core::constant::Value* c,
                            const core::type::Matrix* m) {
        EmitType(out, m);

        const ScopedParen sp(out);
        for (size_t i = 0; i < m->columns(); i++) {
            if (i > 0) {
                out << ", ";
            }
            EmitConstant(out, c->Index(i));
        }
    }

    void EmitType(StringStream& out,
                  const core::type::Type* ty,
                  core::AddressSpace address_space = core::AddressSpace::kUndefined,
                  core::Access access = core::Access::kUndefined,
                  const std::string& name = "",
                  bool* name_printed = nullptr) {
        if (name_printed) {
            *name_printed = false;
        }

        switch (address_space) {
            case core::AddressSpace::kStorage:
                if (access != core::Access::kRead) {
                    out << "RW";
                }
                out << "ByteAddressBuffer";
                return;
            case core::AddressSpace::kUniform: {
                auto array_length = (ty->Size() + 15) / 16;
                out << "uint4 " << name << "[" << array_length << "]";
                if (name_printed) {
                    *name_printed = true;
                }
                return;
            }
            default:
                break;
        }

        Switch(
            ty,                                                   //
            [&](const core::type::Bool*) { out << "bool"; },      //
            [&](const core::type::F16*) { out << "float16_t"; },  //
            [&](const core::type::F32*) { out << "float"; },      //
            [&](const core::type::I32*) { out << "int"; },        //
            [&](const core::type::U32*) { out << "uint"; },       //
            [&](const core::type::Void*) { out << "void"; },      //

            [&](const core::type::Atomic* atomic) {
                EmitType(out, atomic->Type(), address_space, access, name);
            },
            [&](const core::type::Array* ary) { EmitArrayType(out, ary, address_space, access); },
            [&](const core::type::Vector* vec) { EmitVectorType(out, vec, address_space, access); },
            [&](const core::type::Matrix* mat) { EmitMatrixType(out, mat, address_space, access); },
            [&](const core::type::Struct* str) { out << StructName(str); },

            [&](const core::type::Pointer* p) {
                EmitType(out, p->StoreType(), p->AddressSpace(), p->Access());
            },
            [&](const core::type::Sampler* sampler) { EmitSamplerType(out, sampler); },
            [&](const core::type::Texture* tex) { EmitTextureType(out, tex); },
            TINT_ICE_ON_NO_MATCH);
    }

    void EmitArrayType(StringStream& out,
                       const core::type::Array* ary,
                       core::AddressSpace address_space,
                       core::Access access) {
        const core::type::Type* base_type = ary;
        std::vector<uint32_t> sizes;
        while (auto* arr = base_type->As<core::type::Array>()) {
            if (TINT_UNLIKELY(arr->Count()->Is<core::type::RuntimeArrayCount>())) {
                TINT_ICE() << "runtime arrays may only exist in storage buffers, which "
                              "should have "
                              "been transformed into a ByteAddressBuffer";
            }
            const auto count = arr->ConstantCount();
            TINT_ASSERT(count.has_value() && count.value() > 0);

            sizes.push_back(count.value());
            base_type = arr->ElemType();
        }
        EmitType(out, base_type, address_space, access);

        for (const uint32_t size : sizes) {
            out << "[" << size << "]";
        }
    }

    void EmitVectorType(StringStream& out,
                        const core::type::Vector* vec,
                        core::AddressSpace address_space,
                        core::Access access) {
        auto width = vec->Width();
        if (vec->type()->Is<core::type::F32>()) {
            out << "float" << width;
        } else if (vec->type()->Is<core::type::I32>()) {
            out << "int" << width;
        } else if (vec->type()->Is<core::type::U32>()) {
            out << "uint" << width;
        } else if (vec->type()->Is<core::type::Bool>()) {
            out << "bool" << width;
        } else {
            // For example, use "vector<float16_t, N>" for f16 vector.
            out << "vector<";
            EmitType(out, vec->type(), address_space, access);
            out << ", " << width << ">";
        }
    }

    void EmitMatrixType(StringStream& out,
                        const core::type::Matrix* mat,
                        core::AddressSpace address_space,
                        core::Access access) {
        if (mat->type()->Is<core::type::F16>()) {
            // Use matrix<type, N, M> for f16 matrix
            out << "matrix<";
            EmitType(out, mat->type(), address_space, access);
            out << ", " << mat->columns() << ", " << mat->rows() << ">";
            return;
        }

        EmitType(out, mat->type(), address_space, access);

        // Note: HLSL's matrices are declared as <type>NxM, where N is the
        // number of rows and M is the number of columns. Despite HLSL's
        // matrices being column-major by default, the index operator and
        // initializers actually operate on row-vectors, where as WGSL operates
        // on column vectors. To simplify everything we use the transpose of the
        // matrices. See:
        // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math#matrix-ordering
        out << mat->columns() << "x" << mat->rows();
    }

    void EmitTextureType(StringStream& out, const core::type::Texture* tex) {
        if (TINT_UNLIKELY(tex->Is<core::type::ExternalTexture>())) {
            TINT_ICE() << "Multiplanar external texture transform was not run.";
        }

        auto* storage = tex->As<core::type::StorageTexture>();
        auto* ms = tex->As<core::type::MultisampledTexture>();
        auto* depth_ms = tex->As<core::type::DepthMultisampledTexture>();
        auto* sampled = tex->As<core::type::SampledTexture>();

        if (storage && storage->access() != core::Access::kRead) {
            out << "RW";
        }
        out << "Texture";

        switch (tex->dim()) {
            case core::type::TextureDimension::k1d:
                out << "1D";
                break;
            case core::type::TextureDimension::k2d:
                out << ((ms || depth_ms) ? "2DMS" : "2D");
                break;
            case core::type::TextureDimension::k2dArray:
                out << ((ms || depth_ms) ? "2DMSArray" : "2DArray");
                break;
            case core::type::TextureDimension::k3d:
                out << "3D";
                break;
            case core::type::TextureDimension::kCube:
                out << "Cube";
                break;
            case core::type::TextureDimension::kCubeArray:
                out << "CubeArray";
                break;
            default:
                TINT_UNREACHABLE() << "unexpected TextureDimension " << tex->dim();
        }

        if (storage) {
            auto* component = ImageFormatToRWtextureType(storage->texel_format());
            if (TINT_UNLIKELY(!component)) {
                TINT_ICE() << "Unsupported StorageTexture TexelFormat: "
                           << static_cast<int>(storage->texel_format());
            }
            out << "<" << component << ">";
        } else if (depth_ms) {
            out << "<float4>";
        } else if (sampled || ms) {
            auto* subtype = sampled ? sampled->type() : ms->type();
            out << "<";
            if (subtype->Is<core::type::F32>()) {
                out << "float4";
            } else if (subtype->Is<core::type::I32>()) {
                out << "int4";
            } else if (TINT_LIKELY(subtype->Is<core::type::U32>())) {
                out << "uint4";
            } else {
                TINT_ICE() << "Unsupported multisampled texture type";
            }
            out << ">";
        }
    }

    void EmitSamplerType(StringStream& out, const core::type::Sampler* sampler) {
        out << "Sampler";
        if (sampler->IsComparison()) {
            out << "Comparison";
        }
        out << "State";
    }

    /// @returns the name of the given value, creating a new unique name if the value is unnamed in
    /// the module.
    std::string NameOf(const core::ir::Value* value) {
        return names_.GetOrAdd(value, [&] {
            auto sym = ir_.NameOf(value);
            return sym.IsValid() ? sym.Name() : UniqueIdentifier("v");
        });
    }

    /// @return a new, unique identifier with the given prefix.
    /// @param prefix optional prefix to apply to the generated identifier. If empty
    /// "tint_symbol" will be used.
    std::string UniqueIdentifier(const std::string& prefix /* = "" */) {
        return ir_.symbols.New(prefix).Name();
    }

    std::string StructName(const core::type::Struct* s) {
        auto name = s->Name().Name();
        if (HasPrefix(name, "__")) {
            name = tint::GetOrAdd(builtin_struct_names_, s,
                                  [&] { return UniqueIdentifier(name.substr(2)); });
        }
        return name;
    }

    void PrintF32(StringStream& out, float value) {
        if (std::isinf(value)) {
            out << "0.0f " << (value >= 0 ? "/* inf */" : "/* -inf */");
        } else if (std::isnan(value)) {
            out << "0.0f /* nan */";
        } else {
            out << tint::strconv::FloatToString(value) << "f";
        }
    }

    void PrintF16(StringStream& out, float value) {
        if (std::isinf(value)) {
            out << "0.0h " << (value >= 0 ? "/* inf */" : "/* -inf */");
        } else if (std::isnan(value)) {
            out << "0.0h /* nan */";
        } else {
            out << tint::strconv::FloatToString(value) << "h";
        }
    }

    const char* ImageFormatToRWtextureType(core::TexelFormat image_format) {
        switch (image_format) {
            case core::TexelFormat::kR8Unorm:
            case core::TexelFormat::kBgra8Unorm:
            case core::TexelFormat::kRgba8Unorm:
            case core::TexelFormat::kRgba8Snorm:
            case core::TexelFormat::kRgba16Float:
            case core::TexelFormat::kR32Float:
            case core::TexelFormat::kRg32Float:
            case core::TexelFormat::kRgba32Float:
                return "float4";
            case core::TexelFormat::kRgba8Uint:
            case core::TexelFormat::kRgba16Uint:
            case core::TexelFormat::kR32Uint:
            case core::TexelFormat::kRg32Uint:
            case core::TexelFormat::kRgba32Uint:
                return "uint4";
            case core::TexelFormat::kRgba8Sint:
            case core::TexelFormat::kRgba16Sint:
            case core::TexelFormat::kR32Sint:
            case core::TexelFormat::kRg32Sint:
            case core::TexelFormat::kRgba32Sint:
                return "int4";
            default:
                return nullptr;
        }
    }
};

}  // namespace

Result<PrintResult> Print(core::ir::Module& module) {
    return Printer{module}.Generate();
}

PrintResult::PrintResult() = default;

PrintResult::~PrintResult() = default;

PrintResult::PrintResult(const PrintResult&) = default;

PrintResult& PrintResult::operator=(const PrintResult&) = default;

}  // namespace tint::hlsl::writer
