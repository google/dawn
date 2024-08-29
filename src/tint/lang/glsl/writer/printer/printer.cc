// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/glsl/writer/printer/printer.h"

#include <string>
#include <utility>

#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/core_unary.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/glsl/writer/common/printer_support.h"
#include "src/tint/lang/glsl/writer/common/version.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/generator/text_generator.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::glsl::writer {
namespace {

constexpr const char* kAMDGpuShaderHalfFloat = "GL_AMD_gpu_shader_half_float";

/// PIMPL class for the MSL generator
class Printer : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    explicit Printer(core::ir::Module& module) : ir_(module) {}

    /// @param version the GLSL version information
    /// @returns the generated GLSL shader
    tint::Result<std::string> Generate(const Version& version) {
        auto valid = core::ir::ValidateAndDumpIfNeeded(ir_, "GLSL writer");
        if (valid != Success) {
            return std::move(valid.Failure());
        }

        {
            TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);

            auto out = Line();
            out << "#version " << version.major_version << version.minor_version << "0";
            if (version.IsES()) {
                out << " es";
            }
        }

        // Emit module-scope declarations.
        EmitBlock(ir_.root_block);

        // Emit functions.
        for (auto& func : ir_.functions) {
            EmitFunction(func);
        }

        StringStream ss;
        ss << preamble_buffer_.String() << '\n' << main_buffer_.String();
        return ss.str();
    }

  private:
    core::ir::Module& ir_;

    /// The buffer holding preamble text
    TextBuffer preamble_buffer_;

    /// The current function being emitted
    const core::ir::Function* current_function_ = nullptr;
    /// The current block being emitted
    const core::ir::Block* current_block_ = nullptr;

    Hashset<std::string, 4> emitted_extensions_;

    /// A hashmap of value to name
    Hashmap<const core::ir::Value*, std::string, 32> names_;

    /// Map of builtin structure to unique generated name
    Hashmap<const core::type::Struct*, std::string, 4> builtin_struct_names_;

    // The set of emitted structs
    Hashset<const core::type::Struct*, 4> emitted_structs_;

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

    /// @param s the structure
    /// @returns the name of the structure, taking special care of builtin structures that start
    /// with double underscores. If the structure is a builtin, then the returned name will be a
    /// unique name without the leading underscores.
    std::string StructName(const core::type::Struct* s) {
        auto name = s->Name().Name();
        if (HasPrefix(name, "__")) {
            name =
                builtin_struct_names_.GetOrAdd(s, [&] { return UniqueIdentifier(name.substr(2)); });
        }
        return name;
    }

    /// Emit the function
    /// @param func the function to emit
    void EmitFunction(const core::ir::Function* func) {
        TINT_SCOPED_ASSIGNMENT(current_function_, func);

        {
            auto out = Line();

            if (func->Stage() == core::ir::Function::PipelineStage::kCompute) {
                auto wg_opt = func->WorkgroupSize();
                TINT_ASSERT(wg_opt.has_value());

                auto& wg = wg_opt.value();
                Line() << "layout(local_size_x = " << wg[0] << ", local_size_y = " << wg[1]
                       << ", local_size_z = " << wg[2] << ") in;";
            }

            // TODO(dsinclair): Handle return type attributes

            EmitType(out, func->ReturnType());
            out << " ";

            // Fragment shaders need a precision statement
            if (func->Stage() == core::ir::Function::PipelineStage::kFragment) {
                auto pre = Line(&preamble_buffer_);
                pre << "precision highp float;\n";
                pre << "precision highp int;\n";
            }

            // Switch the entry point name to `main`. This makes the assumption that single entry
            // point is always run for GLSL, which is has to be, there can be only one entry point.
            // So, we swap the entry point name to `main` which is required for GLSL.
            if (func->Stage() != core::ir::Function::PipelineStage::kUndefined) {
                out << "main";
            } else {
                out << ir_.NameOf(func).Name();
            }

            out << "(";

            size_t i = 0;
            for (auto* param : func->Params()) {
                if (i > 0) {
                    out << ", ";
                }
                ++i;

                const core::type::Type* type = param->Type();
                if (auto* ptr = type->As<core::type::Pointer>()) {
                    // Transform pointer parameters in to `inout` parameters.
                    out << "inout ";
                    type = ptr->StoreType();
                }
                EmitTypeAndName(out, type, NameOf(param));
            }

            out << ") {";
        }
        {
            ScopedIndent si(current_buffer_);
            EmitBlock(func->Block());
        }

        Line() << "}";
    }

    /// Emit a block
    /// @param block the block to emit
    void EmitBlock(const core::ir::Block* block) {
        TINT_SCOPED_ASSIGNMENT(current_block_, block);

        for (auto* inst : *block) {
            tint::Switch(
                inst,                                                      //
                [&](const core::ir::Call* i) { EmitCallStmt(i); },         //
                [&](const core::ir::Let* i) { EmitLet(i); },               //
                [&](const core::ir::Return* r) { EmitReturn(r); },         //
                [&](const core::ir::Unreachable*) { EmitUnreachable(); },  //
                [&](const core::ir::Var* v) { EmitVar(Line(), v); },       //

                [&](const core::ir::NextIteration*) { /* do nothing */ },                //
                [&](const core::ir::ExitIf*) { /* do nothing handled by transform */ },  //
                                                                                         //
                [&](const core::ir::Access*) { /* inlined */ },                          //
                [&](const core::ir::Bitcast*) { /* inlined */ },                         //
                [&](const core::ir::Construct*) { /* inlined */ },                       //
                [&](const core::ir::CoreBinary*) { /* inlined */ },                      //
                [&](const core::ir::CoreUnary*) { /* inlined */ },                       //
                [&](const core::ir::Load*) { /* inlined */ },                            //
                [&](const core::ir::LoadVectorElement*) { /* inlined */ },               //
                [&](const core::ir::Swizzle*) { /* inlined */ },                         //
                TINT_ICE_ON_NO_MATCH);
        }
    }

    void EmitLet(const core::ir::Let* l) {
        auto out = Line();

        // TODO(dsinclair): Investigate using `const` here as well, the AST printer doesn't emit
        //                  const with a let, but we should be able to.
        EmitTypeAndName(out, l->Result(0)->Type(), NameOf(l->Result(0)));
        out << " = ";
        EmitValue(out, l->Value());
        out << ";";
    }

    void EmitCallStmt(const core::ir::Call* c) {
        if (!c->Result(0)->IsUsed()) {
            auto out = Line();
            EmitValue(out, c->Result(0));
            out << ";";
        }
    }

    void EmitExtension(std::string name) {
        if (emitted_extensions_.Contains(name)) {
            return;
        }
        emitted_extensions_.Add(name);

        TINT_SCOPED_ASSIGNMENT(current_buffer_, &preamble_buffer_);

        Line() << "#extension " << name << ": require";
    }

    void EmitTypeAndName(StringStream& out, const core::type::Type* type, const std::string& name) {
        bool name_printed = false;
        EmitType(out, type, name, &name_printed);

        if (!name.empty() && !name_printed) {
            out << " " << name;
        }
    }

    /// Emit a type
    /// @param out the stream to emit too
    /// @param type the type to emit
    void EmitType(StringStream& out,
                  const core::type::Type* type,
                  [[maybe_unused]] const std::string& name = "",
                  bool* name_printed = nullptr) {
        if (name_printed) {
            *name_printed = false;
        }

        if (auto* ptr = type->As<core::type::MemoryView>()) {
            switch (ptr->AddressSpace()) {
                case core::AddressSpace::kIn: {
                    out << "in ";
                    break;
                }
                case core::AddressSpace::kOut: {
                    out << "out ";
                    break;
                }
                case core::AddressSpace::kUniform:
                case core::AddressSpace::kPushConstant:
                case core::AddressSpace::kHandle: {
                    out << "uniform ";
                    break;
                }
                default:
                    break;
            }
        }

        tint::Switch(
            type,  //
            [&](const core::type::Array* ary) { EmitArrayType(out, ary, name, name_printed); },
            [&](const core::type::Bool*) { out << "bool"; },
            [&](const core::type::I32*) { out << "int"; },
            [&](const core::type::U32*) { out << "uint"; },
            [&](const core::type::Void*) { out << "void"; },
            [&](const core::type::F32*) { out << "float"; },
            [&](const core::type::F16*) {
                EmitExtension(kAMDGpuShaderHalfFloat);
                out << "float16_t";
            },
            [&](const core::type::Pointer* p) {
                EmitType(out, p->StoreType(), name, name_printed);
            },
            [&](const core::type::Vector* v) { EmitVectorType(out, v); },
            [&](const core::type::Matrix* m) { EmitMatrixType(out, m); },
            [&](const core::type::Struct* s) {
                EmitStructType(s);
                out << StructName(s);
            },

            // TODO(dsinclair): Handle remaining types
            TINT_ICE_ON_NO_MATCH);
    }

    void EmitStructType(const core::type::Struct* str) {
        if (!emitted_structs_.Add(str)) {
            return;
        }

        // This does not append directly to the preamble because a struct may require other
        // structs to get emitted before it. So, the struct emits into a temporary text buffer, then
        // anything it depends on will emit to the preamble first, and then it copies the text
        // buffer into the preamble.
        TextBuffer str_buf;
        Line(&str_buf) << "\n" << "struct " << StructName(str) << " {";

        str_buf.IncrementIndent();

        for (auto* mem : str->Members()) {
            auto out = Line(&str_buf);
            EmitTypeAndName(out, mem->Type(), mem->Name().Name());
            out << ";";
        }

        str_buf.DecrementIndent();
        Line(&str_buf) << "};";

        preamble_buffer_.Append(str_buf);
    }

    void EmitVectorType(StringStream& out, const core::type::Vector* v) {
        tint::Switch(
            v->Type(),                       //
            [&](const core::type::F32*) {},  //
            [&](const core::type::F16*) {
                EmitExtension(kAMDGpuShaderHalfFloat);
                out << "f16";
            },
            [&](const core::type::I32*) { out << "i"; },
            [&](const core::type::U32*) { out << "u"; },
            [&](const core::type::Bool*) { out << "b"; },  //
            TINT_ICE_ON_NO_MATCH);

        out << "vec" << v->Width();
    }

    void EmitMatrixType(StringStream& out, const core::type::Matrix* m) {
        if (m->Type()->Is<core::type::F16>()) {
            EmitExtension(kAMDGpuShaderHalfFloat);
            out << "f16";
        }
        out << "mat" << m->Columns();
        if (m->Rows() != m->Columns()) {
            out << "x" << m->Rows();
        }
    }

    void EmitArrayType(StringStream& out,
                       const core::type::Array* ary,
                       const std::string& name,
                       bool* name_printed) {
        EmitType(out, ary->DeepestElement());
        if (!name.empty()) {
            out << " " << name;
            if (name_printed) {
                *name_printed = true;
            }
        }

        const core::type::Type* ty = ary;
        while (auto* arr = ty->As<core::type::Array>()) {
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                out << "[]";
            } else {
                auto count = arr->ConstantCount();
                TINT_ASSERT(count.has_value());

                out << "[" << count.value() << "]";
            }
            ty = arr->ElemType();
        }
    }

    /// Emit a return instruction
    /// @param r the return instruction
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

    void EmitVar(StringStream& out, const core::ir::Var* var) {
        EmitTypeAndName(out, var->Result(0)->Type(), NameOf(var->Result(0)));
        out << " = ";

        if (var->Initializer()) {
            EmitValue(out, var->Initializer());
        } else {
            auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
            TINT_ASSERT(ptr);

            EmitZeroValue(out, ptr->UnwrapPtr());
        }
        out << ";";
    }

    /// Emits the zero value for the given type
    /// @param out the stream to emit too
    /// @param ty the type
    void EmitZeroValue(StringStream& out, const core::type::Type* ty) {
        EmitConstant(out, ir_.constant_values.Zero(ty));
    }

    void EmitValue(StringStream& out, const core::ir::Value* v) {
        tint::Switch(
            v,                                                           //
            [&](const core::ir::Constant* c) { EmitConstant(out, c); },  //
            [&](const core::ir::InstructionResult* r) {
                tint::Switch(
                    r->Instruction(),                                                  //
                    [&](const core::ir::UserCall* c) { EmitUserCall(out, c); },        //
                    [&](const core::ir::Var* var) { out << NameOf(var->Result(0)); },  //
                    TINT_ICE_ON_NO_MATCH);
            },
            [&](const core::ir::FunctionParam* p) { out << NameOf(p); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Emits a user call instruction
    void EmitUserCall(StringStream& out, const core::ir::UserCall* c) {
        out << NameOf(c->Target()) << "(";
        size_t i = 0;
        for (const auto* arg : c->Args()) {
            if (i > 0) {
                out << ", ";
            }
            ++i;

            EmitValue(out, arg);
        }
        out << ")";
    }

    void EmitConstant(StringStream& out, const core::ir::Constant* c) {
        EmitConstant(out, c->Value());
    }

    void EmitConstant(StringStream& out, const core::constant::Value* c) {
        tint::Switch(
            c->Type(),  //
            [&](const core::type::Array* ary) { EmitConstantArray(out, ary, c); },
            [&](const core::type::Bool*) { out << (c->ValueAs<AInt>() ? "true" : "false"); },
            [&](const core::type::I32*) { PrintI32(out, c->ValueAs<i32>()); },
            [&](const core::type::U32*) { out << c->ValueAs<AInt>() << "u"; },
            [&](const core::type::F32*) { PrintF32(out, c->ValueAs<f32>()); },
            [&](const core::type::F16*) { PrintF16(out, c->ValueAs<f16>()); },
            [&](const core::type::Vector* v) { EmitConstantVector(out, v, c); },
            [&](const core::type::Matrix* m) { EmitConstantMatrix(out, m, c); },
            [&](const core::type::Struct* s) { EmitConstantStruct(out, s, c); },

            // TODO(dsinclair): Emit remaining constant types
            TINT_ICE_ON_NO_MATCH);
    }

    void EmitConstantStruct(StringStream& out,
                            const core::type::Struct* s,
                            const core::constant::Value* c) {
        EmitType(out, s);
        ScopedParen sp(out);

        for (size_t i = 0; i < s->Members().Length(); ++i) {
            if (i > 0) {
                out << ", ";
            }
            EmitConstant(out, c->Index(i));
        }
    }

    void EmitConstantVector(StringStream& out,
                            const core::type::Vector* v,
                            const core::constant::Value* c) {
        EmitType(out, v);

        ScopedParen sp(out);

        if (auto* splat = c->As<core::constant::Splat>()) {
            EmitConstant(out, splat->el);
            return;
        }

        for (size_t i = 0; i < v->Width(); ++i) {
            if (i > 0) {
                out << ", ";
            }
            EmitConstant(out, c->Index(i));
        }
    }

    void EmitConstantMatrix(StringStream& out,
                            const core::type::Matrix* m,
                            const core::constant::Value* c) {
        EmitType(out, m);
        ScopedParen sp(out);

        for (size_t col_idx = 0; col_idx < m->Columns(); ++col_idx) {
            if (col_idx > 0) {
                out << ", ";
            }
            EmitConstant(out, c->Index(col_idx));
        }
    }

    void EmitConstantArray(StringStream& out,
                           const core::type::Array* ary,
                           const core::constant::Value* c) {
        EmitType(out, ary);
        ScopedParen sp(out);

        auto count = ary->ConstantCount();
        TINT_ASSERT(count.has_value());

        for (size_t i = 0; i < count; ++i) {
            if (i > 0) {
                out << ", ";
            }
            EmitConstant(out, c->Index(i));
        }
    }

    /// Emit an unreachable instruction
    void EmitUnreachable() { Line() << "/* unreachable */"; }
};

}  // namespace

Result<std::string> Print(core::ir::Module& module, const Version& version) {
    return Printer{module}.Generate(version);
}

}  // namespace tint::glsl::writer
