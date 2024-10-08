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

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/core_unary.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/multi_in_block.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/texel_format.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/lang/glsl/ir/builtin_call.h"
#include "src/tint/lang/glsl/ir/member_builtin_call.h"
#include "src/tint/lang/glsl/ir/ternary.h"
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
constexpr const char* kOESSampleVariables = "GL_OES_sample_variables";
constexpr const char* kEXTBlendFuncExtended = "GL_EXT_blend_func_extended";
constexpr const char* kEXTTextureShadowLod = "GL_EXT_texture_shadow_lod";

enum class LayoutFormat : uint8_t {
    kStd140,
    kStd430,
};

/// PIMPL class for the MSL generator
class Printer : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    /// @param version the GLSL version information
    Printer(core::ir::Module& module, const Version& version) : ir_(module), version_(version) {}

    /// @returns the generated GLSL shader
    tint::Result<std::string> Generate() {
        auto valid = core::ir::ValidateAndDumpIfNeeded(
            ir_, "GLSL writer",
            core::ir::Capabilities{core::ir::Capability::kAllowHandleVarsWithoutBindings});
        if (valid != Success) {
            return std::move(valid.Failure());
        }

        {
            TINT_SCOPED_ASSIGNMENT(current_buffer_, &header_buffer_);

            auto out = Line();
            out << "#version " << version_.major_version << version_.minor_version << "0";
            if (version_.IsES()) {
                out << " es";
            }
        }

        EmitRootBlock();

        // Emit functions.
        for (auto& func : ir_.DependencyOrderedFunctions()) {
            EmitFunction(func);
        }

        StringStream ss;
        auto header = header_buffer_.String();
        if (!header.empty()) {
            ss << header << "\n";
        }

        auto preamble = preamble_buffer_.String();
        if (!preamble.empty()) {
            ss << preamble << "\n";
        }
        ss << main_buffer_.String();

        return ss.str();
    }

  private:
    core::ir::Module& ir_;

    const Version& version_;

    /// The buffer holding header text
    TextBuffer header_buffer_;

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

    /// Block to emit for a continuing
    std::function<void()> emit_continuing_;

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

    void EmitRootBlock() {
        TINT_SCOPED_ASSIGNMENT(current_block_, ir_.root_block);

        for (auto* inst : *ir_.root_block) {
            tint::Switch(
                inst,  //
                [&](core::ir::Var* v) { EmitGlobalVar(v); },

                TINT_ICE_ON_NO_MATCH);
        }
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

            EmitType(out, func->ReturnType());
            out << " ";

            // Fragment shaders need a precision statement
            if (func->Stage() == core::ir::Function::PipelineStage::kFragment) {
                auto pre = Line(&header_buffer_);
                pre << "precision highp float;\n";
                pre << "precision highp int;";
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
                inst,  //
                // TerminateInvocation must come before Call.
                [&](const core::ir::TerminateInvocation*) { EmitDiscard(); },  //

                [&](const core::ir::BreakIf* i) { EmitBreakIf(i); },                        //
                [&](const core::ir::Call* i) { EmitCallStmt(i); },                          //
                [&](const core::ir::Continue*) { EmitContinue(); },                         //
                [&](const core::ir::ExitIf*) { /* do nothing handled by transform */ },     //
                [&](const core::ir::ExitLoop*) { EmitExitLoop(); },                         //
                [&](const core::ir::ExitSwitch*) { EmitExitSwitch(); },                     //
                [&](const core::ir::If* i) { EmitIf(i); },                                  //
                [&](const core::ir::Let* i) { EmitLet(i); },                                //
                [&](const core::ir::Loop* l) { EmitLoop(l); },                              //
                [&](const core::ir::Return* r) { EmitReturn(r); },                          //
                [&](const core::ir::Store* s) { EmitStore(s); },                            //
                [&](const core::ir::StoreVectorElement* s) { EmitStoreVectorElement(s); },  //
                [&](const core::ir::Switch* i) { EmitSwitch(i); },                          //
                [&](const core::ir::Unreachable*) { EmitUnreachable(); },                   //
                [&](const core::ir::Var* v) { EmitVar(Line(), v); },                        //

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

    void EmitStoreVectorElement(const core::ir::StoreVectorElement* l) {
        auto out = Line();

        EmitValue(out, l->To());
        out << "[";
        EmitValue(out, l->Index());
        out << "] = ";
        EmitValue(out, l->Value());
        out << ";";
    }

    void IdxToComponent(StringStream& out, uint32_t idx) {
        switch (idx) {
            case 0:
                out << "x";
                break;
            case 1:
                out << "y";
                break;
            case 2:
                out << "z";
                break;
            case 3:
                out << "w";
                break;
            default:
                TINT_UNREACHABLE() << "invalid index for component";
        }
    }

    void EmitLoadVectorElement(StringStream& out, const core::ir::LoadVectorElement* l) {
        EmitValue(out, l->From());

        if (auto* cnst = l->Index()->As<core::ir::Constant>()) {
            out << ".";
            IdxToComponent(out, cnst->Value()->ValueAs<uint32_t>());
        } else {
            out << "[";
            EmitValue(out, l->Index());
            out << "]";
        }
    }

    void EmitSwizzle(StringStream& out, const core::ir::Swizzle* swizzle) {
        EmitValue(out, swizzle->Object());
        out << ".";
        for (const auto i : swizzle->Indices()) {
            IdxToComponent(out, i);
        }
    }

    void EmitDiscard() { Line() << "discard;"; }

    void EmitContinue() {
        if (emit_continuing_) {
            emit_continuing_();
        }
        Line() << "continue;";
    }

    void EmitExitLoop() { Line() << "break;"; }

    void EmitLoop(const core::ir::Loop* l) {
        // Note, we can't just emit the continuing inside a conditional at the top of the loop
        // because any variable declared in the block must be visible to the continuing.
        //
        // loop {
        //   var a = 3;
        //   continue {
        //     let y = a;
        //   }
        // }

        auto emit_continuing = [&] {
            Line() << "{";
            {
                const ScopedIndent si(current_buffer_);
                EmitBlock(l->Continuing());
            }
            Line() << "}";
        };
        TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);

        Line() << "{";
        {
            ScopedIndent init(current_buffer_);
            EmitBlock(l->Initializer());

            Line() << "while(true) {";
            {
                ScopedIndent si(current_buffer_);
                EmitBlock(l->Body());
            }
            Line() << "}";
        }
        Line() << "}";
    }

    /// Emit an if instruction
    /// @param if_ the if instruction
    void EmitIf(const core::ir::If* if_) {
        {
            auto out = Line();
            out << "if (";
            EmitValue(out, if_->Condition());
            out << ") {";
        }

        {
            const ScopedIndent si(current_buffer_);
            EmitBlock(if_->True());
        }

        if (if_->False() && !if_->False()->IsEmpty()) {
            Line() << "} else {";

            const ScopedIndent si(current_buffer_);
            EmitBlock(if_->False());
        }

        Line() << "}";
    }

    void EmitBreakIf(const core::ir::BreakIf* b) {
        auto out = Line();
        out << "if (";
        EmitValue(out, b->Condition());
        out << ") { break; }";
    }

    void EmitExitSwitch() { Line() << "break;"; }

    void EmitSwitch(const core::ir::Switch* s) {
        {
            auto out = Line();
            out << "switch(";
            EmitValue(out, s->Condition());
            out << ") {";
        }
        {
            const ScopedIndent blk(current_buffer_);
            for (auto& case_ : s->Cases()) {
                for (auto& sel : case_.selectors) {
                    if (sel.IsDefault()) {
                        Line() << "default:";
                    } else {
                        auto out = Line();
                        out << "case ";
                        EmitValue(out, sel.val);
                        out << ":";
                    }
                }
                Line() << "{";
                {
                    const ScopedIndent ci(current_buffer_);
                    EmitBlock(case_.block);
                }
                Line() << "}";
            }
        }
        Line() << "}";
    }

    /// Emit an access instruction
    void EmitAccess(StringStream& out, const core::ir::Access* a) {
        EmitValue(out, a->Object());

        auto* current_type = a->Object()->Type()->UnwrapPtr();
        for (auto* index : a->Indices()) {
            TINT_ASSERT(current_type);
            Switch(
                current_type,  //
                [&](const core::type::Struct* s) {
                    auto* c = index->As<core::ir::Constant>();
                    auto* member = s->Members()[c->Value()->ValueAs<uint32_t>()];
                    out << "." << member->Name().Name();
                    current_type = member->Type();
                },
                [&](Default) {
                    out << "[";
                    EmitValue(out, index);
                    out << "]";
                    current_type = current_type->Element(0);
                });
        }
    }

    void EmitLet(const core::ir::Let* l) {
        TINT_ASSERT(!l->Result(0)->Type()->Is<core::type::Pointer>());

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

        TINT_SCOPED_ASSIGNMENT(current_buffer_, &header_buffer_);

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
            [&](const core::type::Atomic* a) { EmitType(out, a->Type(), name, name_printed); },
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
            [&](const core::type::Texture* t) { EmitTextureType(out, t); },

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
        std::stringstream args;
        const core::type::Type* ty = ary;
        while (auto* arr = ty->As<core::type::Array>()) {
            if (arr->Count()->Is<core::type::RuntimeArrayCount>()) {
                args << "[]";
            } else {
                auto count = arr->ConstantCount();
                TINT_ASSERT(count.has_value());

                args << "[" << count.value() << "]";
            }
            ty = arr->ElemType();
        }

        EmitType(out, ty);
        if (!name.empty()) {
            out << " " << name;
            if (name_printed) {
                *name_printed = true;
            }
        }
        out << args.str();
    }

    void EmitTextureType(StringStream& out, const core::type::Texture* t) {
        TINT_ASSERT(!t->Is<core::type::ExternalTexture>());

        auto* storage = t->As<core::type::StorageTexture>();
        auto* sampled = t->As<core::type::SampledTexture>();
        auto* ms = t->As<core::type::MultisampledTexture>();
        auto* depth_ms = t->As<core::type::DepthMultisampledTexture>();

        out << "highp ";

        if (storage) {
            switch (storage->Access()) {
                case core::Access::kRead:
                    out << "readonly ";
                    break;
                case core::Access::kWrite:
                    out << "writeonly ";
                    break;
                case core::Access::kReadWrite: {
                    if (version_.IsES()) {
                        // ESSL 3.1 SPEC (chapter 4.9, Memory Access Qualifiers):
                        // Except for image variables qualified with the format qualifiers r32f,
                        // r32i, and r32ui, image variables must specify either memory qualifier
                        // readonly or the memory qualifier writeonly.
                        switch (storage->TexelFormat()) {
                            case core::TexelFormat::kR32Float:
                            case core::TexelFormat::kR32Sint:
                            case core::TexelFormat::kR32Uint:
                                break;
                            default:
                                TINT_UNREACHABLE() << "invalid texel format for read-write";
                        }
                    }
                    break;
                }
                default:
                    TINT_UNREACHABLE() << "invalid storage access";
            }
        }
        auto* subtype = sampled   ? sampled->Type()
                        : storage ? storage->Type()
                        : ms      ? ms->Type()
                                  : nullptr;

        if (subtype) {
            tint::Switch(
                subtype,                         //
                [&](const core::type::F32*) {},  //
                [&](const core::type::I32*) { out << "i"; },
                [&](const core::type::U32*) { out << "u"; },  //
                TINT_ICE_ON_NO_MATCH);
        }

        out << (storage ? "image" : "sampler");

        switch (t->Dim()) {
            case core::type::TextureDimension::k2d:
                out << "2D";
                if (ms || depth_ms) {
                    out << "MS";
                }
                break;
            case core::type::TextureDimension::k2dArray:
                out << "2D";
                if (ms) {
                    out << "MS";
                }
                out << "Array";
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
                TINT_UNREACHABLE() << "unknown texture dimension: " << t->Dim();
        }
        if (t->Is<core::type::DepthTexture>()) {
            out << "Shadow";
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
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        auto space = ptr->AddressSpace();

        EmitTypeAndName(out, var->Result(0)->Type(), NameOf(var->Result(0)));
        if (var->Initializer()) {
            out << " = ";
            EmitValue(out, var->Initializer());
        } else if (space == core::AddressSpace::kPrivate ||
                   space == core::AddressSpace::kFunction) {
            TINT_ASSERT(ptr);
            out << " = ";
            EmitZeroValue(out, ptr->UnwrapPtr());
        }
        out << ";";
    }

    void EmitGlobalVar(core::ir::Var* var) {
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        auto space = ptr->AddressSpace();

        switch (space) {
            case core::AddressSpace::kStorage:
                EmitStorageVar(var);
                break;
            case core::AddressSpace::kUniform:
                EmitUniformVar(var);
                break;
            case core::AddressSpace::kWorkgroup:
                EmitWorkgroupVar(var);
                break;
            case core::AddressSpace::kHandle:
                EmitHandleVar(var);
                break;
            case core::AddressSpace::kPushConstant:
                EmitPushConstantVar(var);
                break;
            case core::AddressSpace::kIn:
            case core::AddressSpace::kOut:
                EmitIOVar(var);
                break;
            case core::AddressSpace::kPixelLocal:
                TINT_UNREACHABLE() << "PixelLocal not supported";
            default: {
                auto out = Line();
                EmitVar(out, var);
                break;
            }
        }
    }

    void EmitStorageVar(core::ir::Var* var) {
        const auto& bp = var->BindingPoint();
        TINT_ASSERT(bp.has_value());

        EmitLayoutBinding(Line(), bp.value(), std::nullopt, {LayoutFormat::kStd430});

        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        EmitVarStruct("buffer", NameOf(var->Result(0)), "ssbo",
                      ptr->UnwrapPtr()->As<core::type::Struct>());
    }

    void EmitUniformVar(core::ir::Var* var) {
        const auto& bp = var->BindingPoint();
        TINT_ASSERT(bp.has_value());

        EmitLayoutBinding(Line(), bp.value(), std::nullopt, {LayoutFormat::kStd140});

        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();
        EmitVarStruct("uniform", NameOf(var->Result(0)), "ubo",
                      ptr->UnwrapPtr()->As<core::type::Struct>());
    }

    void EmitWorkgroupVar(core::ir::Var* var) {
        auto out = Line();
        out << "shared ";
        EmitVar(out, var);
    }

    void EmitHandleVar(core::ir::Var* var) {
        auto* ptr = var->Result(0)->Type()->As<core::type::Pointer>();

        // GLSL ignores sampler variables.
        if (ptr->UnwrapPtr()->Is<core::type::Sampler>()) {
            return;
        }

        auto out = Line();
        if (auto* storage = ptr->UnwrapPtr()->As<core::type::StorageTexture>()) {
            const auto& bp = var->BindingPoint();

            TINT_ASSERT(bp.has_value());
            EmitLayoutBinding(out, bp.value(), {storage->TexelFormat()}, std::nullopt);
            out << " ";
        }

        EmitVar(out, var);
    }

    void EmitPushConstantVar(core::ir::Var* var) {
        auto out = Line();
        EmitLayoutLocation(out, {0}, std::nullopt);
        EmitVar(out, var);
    }

    void EmitIOVar(core::ir::Var* var) {
        auto& attrs = var->Attributes();

        if (attrs.builtin.has_value()) {
            if (version_.IsES() && (attrs.builtin == tint::core::BuiltinValue::kSampleIndex ||
                                    attrs.builtin == tint::core::BuiltinValue::kSampleMask)) {
                EmitExtension(kOESSampleVariables);
            }

            // Do not emit builtin (gl_) variables.
            return;
        }

        auto out = Line();
        EmitLayoutLocation(out, attrs.location, attrs.blend_src);
        if (attrs.interpolation.has_value()) {
            EmitInterpolation(out, attrs.interpolation.value());
        }
        EmitVar(out, var);
    }

    void EmitVarStruct(std::string_view kind,
                       std::string_view name,
                       std::string_view type_suffix,
                       const core::type::Struct* str) {
        TINT_ASSERT(str);

        Line() << kind << " " << UniqueIdentifier(StructName(str)) << "_" << type_suffix << " {";

        {
            ScopedIndent si(current_buffer_);

            for (auto* mem : str->Members()) {
                auto out = Line();
                EmitTypeAndName(out, mem->Type(), mem->Name().Name());
                out << ";";
            }
        }

        Line() << "} " << name << ";";
    }

    void EmitLayoutLocation(StringStream& out,
                            std::optional<uint32_t> location,
                            std::optional<uint32_t> blend_src) {
        if (location.has_value()) {
            out << "layout(location = " << location.value();
            if (blend_src.has_value()) {
                EmitExtension(kEXTBlendFuncExtended);

                out << ", index = " << blend_src.value();
            }
            out << ") ";
        }
    }

    void EmitLayoutBinding(StringStream& out,
                           const tint::BindingPoint& bp,
                           std::optional<core::TexelFormat> texel_format,
                           std::optional<LayoutFormat> layout_format) {
        TINT_ASSERT(!(texel_format.has_value() && layout_format.has_value()));

        out << "layout(binding = " << bp.binding;

        if (layout_format.has_value()) {
            out << ", ";
            switch (layout_format.value()) {
                case LayoutFormat::kStd140:
                    out << "std140";
                    break;
                case LayoutFormat::kStd430:
                    out << "std430";
                    break;
            }
        }

        if (texel_format.has_value()) {
            out << ", ";
            switch (texel_format.value()) {
                case core::TexelFormat::kBgra8Unorm:
                    TINT_ICE() << "bgra8unorm should have been polyfilled to rgba8unorm";
                case core::TexelFormat::kR32Uint:
                    out << "r32ui";
                    break;
                case core::TexelFormat::kR32Sint:
                    out << "r32i";
                    break;
                case core::TexelFormat::kR32Float:
                    out << "r32f";
                    break;
                case core::TexelFormat::kRgba8Unorm:
                    out << "rgba8";
                    break;
                case core::TexelFormat::kRgba8Snorm:
                    out << "rgba8_snorm";
                    break;
                case core::TexelFormat::kRgba8Uint:
                    out << "rgba8ui";
                    break;
                case core::TexelFormat::kRgba8Sint:
                    out << "rgba8i";
                    break;
                case core::TexelFormat::kRg32Uint:
                    out << "rg32ui";
                    break;
                case core::TexelFormat::kRg32Sint:
                    out << "rg32i";
                    break;
                case core::TexelFormat::kRg32Float:
                    out << "rg32f";
                    break;
                case core::TexelFormat::kRgba16Uint:
                    out << "rgba16ui";
                    break;
                case core::TexelFormat::kRgba16Sint:
                    out << "rgba16i";
                    break;
                case core::TexelFormat::kRgba16Float:
                    out << "rgba16f";
                    break;
                case core::TexelFormat::kRgba32Uint:
                    out << "rgba32ui";
                    break;
                case core::TexelFormat::kRgba32Sint:
                    out << "rgba32i";
                    break;
                case core::TexelFormat::kRgba32Float:
                    out << "rgba32f";
                    break;
                case core::TexelFormat::kR8Unorm:
                    out << "r8";
                    break;
                case core::TexelFormat::kUndefined:
                    TINT_UNREACHABLE() << "invalid texel format";
            }
        }
        out << ")";
    }

    void EmitInterpolation(StringStream& out, const core::Interpolation& interp) {
        switch (interp.type) {
            case core::InterpolationType::kPerspective:
            case core::InterpolationType::kLinear:
            case core::InterpolationType::kUndefined:
                break;
            case core::InterpolationType::kFlat:
                out << "flat ";
                break;
        }

        switch (interp.sampling) {
            case core::InterpolationSampling::kCentroid:
                out << "centroid ";
                break;
            case core::InterpolationSampling::kSample:
            case core::InterpolationSampling::kCenter:
            case core::InterpolationSampling::kFirst:
            case core::InterpolationSampling::kEither:
            case core::InterpolationSampling::kUndefined:
                break;
        }
    }

    /// Emits the zero value for the given type
    /// @param out the stream to emit too
    /// @param ty the type
    void EmitZeroValue(StringStream& out, const core::type::Type* ty) {
        EmitConstant(out, ir_.constant_values.Zero(ty));
    }

    void EmitValue(StringStream& out, const core::ir::Value* v) {
        tint::Switch(
            v,  //
            [&](const core::ir::Constant* c) { EmitConstant(out, c); },
            [&](const core::ir::InstructionResult* r) {
                tint::Switch(
                    r->Instruction(),  //
                    [&](const core::ir::Access* a) { EmitAccess(out, a); },
                    [&](const core::ir::Construct* c) { EmitConstruct(out, c); },
                    [&](const core::ir::Convert* c) { EmitConvert(out, c); },  //
                    [&](const core::ir::CoreBinary* b) { EmitBinary(out, b); },
                    [&](const core::ir::CoreBuiltinCall* c) { EmitCoreBuiltinCall(out, c); },
                    [&](const core::ir::CoreUnary* u) { EmitUnary(out, u); },
                    [&](const core::ir::Let* l) { out << NameOf(l->Result(0)); },
                    [&](const core::ir::Load* l) { EmitLoad(out, l); },
                    [&](const core::ir::LoadVectorElement* l) { EmitLoadVectorElement(out, l); },
                    [&](const core::ir::Store* s) { EmitStore(s); },
                    [&](const core::ir::Swizzle* s) { EmitSwizzle(out, s); },  //
                    [&](const core::ir::UserCall* c) { EmitUserCall(out, c); },
                    [&](const core::ir::Var* var) { out << NameOf(var->Result(0)); },

                    [&](const glsl::ir::BuiltinCall* c) { EmitGlslBuiltinCall(out, c); },  //
                    [&](const glsl::ir::MemberBuiltinCall* mbc) {
                        EmitGlslMemberBuiltinCall(out, mbc);
                    },
                    [&](const glsl::ir::Ternary* t) { EmitTernary(out, t); },  //

                    TINT_ICE_ON_NO_MATCH);
            },
            [&](const core::ir::FunctionParam* p) { out << NameOf(p); },  //

            TINT_ICE_ON_NO_MATCH);
    }

    void EmitGlslMemberBuiltinCall(StringStream& out, const glsl::ir::MemberBuiltinCall* c) {
        EmitValue(out, c->Object());
        out << "." << c->Func() << "(";

        bool needs_comma = false;
        for (const auto* arg : c->Args()) {
            if (needs_comma) {
                out << ", ";
            }
            EmitValue(out, arg);
            needs_comma = true;
        }
        out << ")";
    }

    bool RequiresEXTTextureShadowLod(glsl::BuiltinFn fn) {
        return fn == glsl::BuiltinFn::kExtTextureLod || fn == glsl::BuiltinFn::kExtTextureLodOffset;
    }

    glsl::BuiltinFn EXTToNonEXT(glsl::BuiltinFn fn) {
        switch (fn) {
            case glsl::BuiltinFn::kExtTextureLod:
                return glsl::BuiltinFn::kTextureLod;
            case glsl::BuiltinFn::kExtTextureLodOffset:
                return glsl::BuiltinFn::kTextureLodOffset;
            default:
                TINT_UNREACHABLE() << "invalid function for conversion: " << fn;
        }
    }

    void EmitGlslBuiltinCall(StringStream& out, const glsl::ir::BuiltinCall* c) {
        // The atomic subtract is an add in GLSL. If the value is a u32, it just negates the u32 and
        // GLSL handles it. We don't have u32 negation in the IR, so fake it in the printer.
        if (c->Func() == glsl::BuiltinFn::kAtomicSub) {
            out << "atomicAdd";
            {
                ScopedParen sp(out);

                EmitValue(out, c->Args()[0]);
                out << ", -";
                {
                    ScopedParen argSP(out);
                    EmitValue(out, c->Args()[1]);
                }
            }
            return;
        }

        auto fn = c->Func();

        if (RequiresEXTTextureShadowLod(fn)) {
            EmitExtension(kEXTTextureShadowLod);
            fn = EXTToNonEXT(fn);
        }

        out << fn << "(";
        bool needs_comma = false;
        for (const auto* arg : c->Args()) {
            if (needs_comma) {
                out << ", ";
            }
            EmitValue(out, arg);
            needs_comma = true;
        }
        out << ")";
    }

    void EmitTernary(StringStream& out, const glsl::ir::Ternary* t) {
        out << "((";
        EmitValue(out, t->Cmp());
        out << ") ? (";
        EmitValue(out, t->True());
        out << ") : (";
        EmitValue(out, t->False());
        out << "))";
        return;
    }

    /// Emit a convert instruction
    void EmitConvert(StringStream& out, const core::ir::Convert* c) {
        EmitType(out, c->Result(0)->Type());
        out << "(";
        EmitValue(out, c->Operand(0));
        out << ")";
    }

    /// Emit a constructor
    void EmitConstruct(StringStream& out, const core::ir::Construct* c) {
        if (c->Args().IsEmpty()) {
            EmitZeroValue(out, c->Result(0)->Type());
            return;
        }

        auto emit_args = [&]() {
            out << "(";

            size_t i = 0;
            for (auto* arg : c->Args()) {
                if (i > 0) {
                    out << ", ";
                }
                EmitValue(out, arg);
                i++;
            }
            out << ")";
        };

        Switch(
            c->Result(0)->Type(),
            [&](const core::type::Struct* struct_ty) {
                EmitStructType(struct_ty);
                out << StructName(struct_ty);
                emit_args();
            },
            [&](Default) {
                EmitType(out, c->Result(0)->Type());
                emit_args();
            });
    }

    /// Emit Load
    /// @param out the output stream to write to
    /// @param load the load
    void EmitLoad(StringStream& out, const core::ir::Load* load) { EmitValue(out, load->From()); }

    /// Emit a store
    void EmitStore(const core::ir::Store* s) {
        auto out = Line();

        EmitValue(out, s->To());
        out << " = ";
        EmitValue(out, s->From());
        out << ";";
    }

    void EmitUnary(StringStream& out, const core::ir::CoreUnary* u) {
        switch (u->Op()) {
            case core::UnaryOp::kNegation:
                out << "-";
                break;
            case core::UnaryOp::kComplement:
                out << "~";
                break;
            case core::UnaryOp::kNot:
                if (u->Val()->Type()->Is<core::type::Scalar>()) {
                    out << "!";
                } else {
                    out << "not";
                }
                break;
            default:
                TINT_UNIMPLEMENTED() << u->Op();
        }
        out << "(";
        EmitValue(out, u->Val());
        out << ")";
    }

    /// Emit a binary instruction
    /// @param b the binary instruction
    void EmitBinary(StringStream& out, const core::ir::CoreBinary* b) {
        auto kind = [&] {
            switch (b->Op()) {
                case core::BinaryOp::kAdd:
                    return "+";
                case core::BinaryOp::kSubtract:
                    return "-";
                case core::BinaryOp::kMultiply:
                    return "*";
                case core::BinaryOp::kDivide:
                    return "/";
                case core::BinaryOp::kModulo:
                    return "%";
                case core::BinaryOp::kAnd:
                    return "&";
                case core::BinaryOp::kOr:
                    return "|";
                case core::BinaryOp::kXor:
                    return "^";
                case core::BinaryOp::kEqual:
                    return "==";
                case core::BinaryOp::kNotEqual:
                    return "!=";
                case core::BinaryOp::kLessThan:
                    return "<";
                case core::BinaryOp::kGreaterThan:
                    return ">";
                case core::BinaryOp::kLessThanEqual:
                    return "<=";
                case core::BinaryOp::kGreaterThanEqual:
                    return ">=";
                case core::BinaryOp::kShiftLeft:
                    return "<<";
                case core::BinaryOp::kShiftRight:
                    return ">>";
                case core::BinaryOp::kLogicalAnd:
                case core::BinaryOp::kLogicalOr:
                    // These should have been replaced by if statements as GLSL is not
                    // short-circuting.
                    TINT_UNREACHABLE() << "logical and/or should not be present";
            }
            return "<error>";
        };

        ScopedParen sp(out);
        EmitValue(out, b->LHS());
        out << " " << kind() << " ";
        EmitValue(out, b->RHS());
    }

    void EmitCoreBuiltinCall(StringStream& out, const core::ir::CoreBuiltinCall* c) {
        EmitCoreBuiltinName(out, c->Func());

        ScopedParen sp(out);
        size_t i = 0;
        for (const auto* arg : c->Args()) {
            if (i > 0) {
                out << ", ";
            }
            ++i;

            EmitValue(out, arg);
        }
    }

    void EmitCoreBuiltinName(StringStream& out, core::BuiltinFn func) {
        switch (func) {
            case core::BuiltinFn::kAbs:
            case core::BuiltinFn::kAcos:
            case core::BuiltinFn::kAcosh:
            case core::BuiltinFn::kAsin:
            case core::BuiltinFn::kAsinh:
            case core::BuiltinFn::kAtan:
            case core::BuiltinFn::kAtanh:
            case core::BuiltinFn::kAtomicAdd:
            case core::BuiltinFn::kAtomicAnd:
            case core::BuiltinFn::kAtomicExchange:
            case core::BuiltinFn::kAtomicMax:
            case core::BuiltinFn::kAtomicMin:
            case core::BuiltinFn::kAtomicOr:
            case core::BuiltinFn::kAtomicXor:
            case core::BuiltinFn::kCeil:
            case core::BuiltinFn::kClamp:
            case core::BuiltinFn::kCos:
            case core::BuiltinFn::kCosh:
            case core::BuiltinFn::kCross:
            case core::BuiltinFn::kDegrees:
            case core::BuiltinFn::kDeterminant:
            case core::BuiltinFn::kDistance:
            case core::BuiltinFn::kExp:
            case core::BuiltinFn::kExp2:
            case core::BuiltinFn::kFloor:
            case core::BuiltinFn::kFrexp:
            case core::BuiltinFn::kLdexp:
            case core::BuiltinFn::kLength:
            case core::BuiltinFn::kLog:
            case core::BuiltinFn::kLog2:
            case core::BuiltinFn::kMax:
            case core::BuiltinFn::kMin:
            case core::BuiltinFn::kNormalize:
            case core::BuiltinFn::kPow:
            case core::BuiltinFn::kRadians:
            case core::BuiltinFn::kReflect:
            case core::BuiltinFn::kRefract:
            case core::BuiltinFn::kRound:
            case core::BuiltinFn::kSign:
            case core::BuiltinFn::kSin:
            case core::BuiltinFn::kSinh:
            case core::BuiltinFn::kSqrt:
            case core::BuiltinFn::kStep:
            case core::BuiltinFn::kTan:
            case core::BuiltinFn::kTanh:
            case core::BuiltinFn::kTranspose:
            case core::BuiltinFn::kTrunc:
                out << func;
                break;
            case core::BuiltinFn::kAtan2:
                out << "atan";
                break;
            case core::BuiltinFn::kAtomicStore:
                // GLSL does not have an atomicStore, so we emulate it with
                // atomicExchange.
                out << "atomicExchange";
                break;
            case core::BuiltinFn::kDpdx:
                out << "dFdx";
                break;
            case core::BuiltinFn::kDpdxCoarse:
                out << "dFdx";
                if (version_.IsDesktop()) {
                    out << "Coarse";
                }
                break;
            case core::BuiltinFn::kDpdxFine:
                out << "dFdx";
                if (version_.IsDesktop()) {
                    out << "Fine";
                }
                break;
            case core::BuiltinFn::kDpdy:
                out << "dFdy";
                break;
            case core::BuiltinFn::kDpdyCoarse:
                out << "dFdy";
                if (version_.IsDesktop()) {
                    out << "Coarse";
                }
                break;
            case core::BuiltinFn::kDpdyFine:
                out << "dFdy";
                if (version_.IsDesktop()) {
                    out << "Fine";
                }
                break;
            case core::BuiltinFn::kFaceForward:
                out << "faceforward";
                break;
            case core::BuiltinFn::kFract:
                out << "fract";
                break;
            case core::BuiltinFn::kFma:
                out << "fma";
                break;
            case core::BuiltinFn::kFwidth:
            case core::BuiltinFn::kFwidthCoarse:
            case core::BuiltinFn::kFwidthFine:
                out << "fwidth";
                break;
            case core::BuiltinFn::kInverseSqrt:
                out << "inversesqrt";
                break;
            case core::BuiltinFn::kMix:
                out << "mix";
                break;
            case core::BuiltinFn::kPack2X16Float:
                out << "packHalf2x16";
                break;
            case core::BuiltinFn::kPack2X16Snorm:
                out << "packSnorm2x16";
                break;
            case core::BuiltinFn::kPack2X16Unorm:
                out << "packUnorm2x16";
                break;
            case core::BuiltinFn::kPack4X8Snorm:
                out << "packSnorm4x8";
                break;
            case core::BuiltinFn::kPack4X8Unorm:
                out << "packUnorm4x8";
                break;
            case core::BuiltinFn::kReverseBits:
                out << "bitfieldReverse";
                break;
            case core::BuiltinFn::kSmoothstep:
                out << "smoothstep";
                break;
            case core::BuiltinFn::kUnpack2X16Float:
                out << "unpackHalf2x16";
                break;
            case core::BuiltinFn::kUnpack2X16Snorm:
                out << "unpackSnorm2x16";
                break;
            case core::BuiltinFn::kUnpack2X16Unorm:
                out << "unpackUnorm2x16";
                break;
            case core::BuiltinFn::kUnpack4X8Snorm:
                out << "unpackSnorm4x8";
                break;
            case core::BuiltinFn::kUnpack4X8Unorm:
                out << "unpackUnorm4x8";
                break;
            default:
                TINT_UNREACHABLE() << "unhandled core builtin: " << func;
        }
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
    return Printer{module, version}.Generate();
}

}  // namespace tint::glsl::writer
