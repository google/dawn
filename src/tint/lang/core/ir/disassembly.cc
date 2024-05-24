// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/core/ir/disassembly.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string_view>

#include "src//tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/binary_op.h"
#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/loop.h"
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
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/styled_text.h"
#include "src/tint/utils/text/text_style.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::core::ir {
namespace {

static constexpr auto StylePlain = style::Plain;
static constexpr auto StyleAttribute = style::Attribute + style::NoQuote;
static constexpr auto StyleCode = style::Code + style::NoQuote;
static constexpr auto StyleComment = style::Comment + style::NoQuote;
static constexpr auto StyleEnum = style::Enum + style::NoQuote;
static constexpr auto StyleError = style::Error + style::NoQuote;
static constexpr auto StyleFunction = style::Function + style::NoQuote;
static constexpr auto StyleInstruction = style::Instruction + style::NoQuote;
static constexpr auto StyleKeyword = style::Keyword + style::NoQuote;
static constexpr auto StyleLabel = style::Label + style::NoQuote;
static constexpr auto StyleLiteral = style::Literal + style::NoQuote;
static constexpr auto StyleType = style::Type + style::NoQuote;
static constexpr auto StyleVariable = style::Variable + style::NoQuote;

class ScopedIndent {
  public:
    explicit ScopedIndent(uint32_t& indent) : indent_(indent) { indent_ += 2; }

    ~ScopedIndent() { indent_ -= 2; }

  private:
    uint32_t& indent_;
};

}  // namespace

Disassembly::Disassembly(Disassembly&&) = default;

Disassembly::Disassembly(const Module& mod, std::string_view file_name) : mod_(mod) {
    Disassemble();
    file_ = std::make_shared<Source::File>(std::string(file_name), Plain());

    auto set_source_file = [&](auto& map) {
        for (auto& it : map) {
            it.value.file = file_.get();
        }
    };
    set_source_file(block_to_src_);
    set_source_file(block_param_to_src_);
    set_source_file(instruction_to_src_);
    set_source_file(operand_to_src_);
    set_source_file(result_to_src_);
    set_source_file(function_to_src_);
    set_source_file(function_param_to_src_);
}

Disassembly::~Disassembly() = default;

void Disassembly::Disassemble() {
    TINT_DEFER(out_ << StylePlain);
    out_.Clear();
    out_ << StyleCode;

    for (auto* ty : mod_.Types()) {
        if (auto* str = ty->As<core::type::Struct>()) {
            EmitStructDecl(str);
        }
    }

    if (!mod_.root_block->IsEmpty()) {
        EmitBlock(mod_.root_block, "root");
        EmitLine();
    }

    for (auto& func : mod_.functions) {
        EmitFunction(func);
    }
}

StyledText& Disassembly::Indent() {
    for (uint32_t i = 0; i < indent_size_; i++) {
        out_ << " ";
    }
    return out_;
}

void Disassembly::EmitLine() {
    out_ << "\n";
    current_output_line_ += 1;
    current_output_start_pos_ = static_cast<uint32_t>(out_.Length());
}

Source::Location Disassembly::MakeCurrentLocation() {
    return Source::Location{
        current_output_line_,
        static_cast<uint32_t>(out_.Length()) - current_output_start_pos_ + 1,
    };
}

void Disassembly::EmitBlock(const Block* blk, std::string_view comment /* = "" */) {
    Indent();

    SourceMarker sm(this);
    out_ << NameOf(blk);
    if (auto* merge = blk->As<MultiInBlock>()) {
        if (!merge->Params().IsEmpty()) {
            out_ << " (";
            for (auto* p : merge->Params()) {
                if (p != merge->Params().Front()) {
                    out_ << ", ";
                }
                {
                    SourceMarker psm(this);
                    EmitValue(p);
                    psm.Store(p);
                }
                out_ << ":" << StyleType(p->Type()->FriendlyName());
            }
            out_ << ")";
        }
    }
    sm.Store(blk);

    out_ << ": {";
    if (!comment.empty()) {
        out_ << "  " << StyleComment("# ", comment);
    }

    EmitLine();
    {
        ScopedIndent si(indent_size_);
        for (auto* inst : *blk) {
            Indent();
            EmitInstruction(inst);
        }
    }
    Indent() << "}";

    EmitLine();
}

void Disassembly::EmitBindingPoint(BindingPoint p) {
    out_ << StyleAttribute("@binding_point") << "(" << StyleLiteral(p.group) << ", "
         << StyleLiteral(p.binding) << ")";
}

void Disassembly::EmitLocation(Location loc) {
    out_ << StyleAttribute("@location") << "(" << loc.value << ")";
    if (loc.interpolation.has_value()) {
        out_ << ", " << StyleAttribute("@interpolate") << "(";
        out_ << StyleEnum(loc.interpolation->type);
        if (loc.interpolation->sampling != core::InterpolationSampling::kUndefined) {
            out_ << ", ";
            out_ << StyleEnum(loc.interpolation->sampling);
        }
        out_ << ")";
    }
}

void Disassembly::EmitParamAttributes(const FunctionParam* p) {
    if (!p->Invariant() && !p->Location().has_value() && !p->BindingPoint().has_value() &&
        !p->Builtin().has_value()) {
        return;
    }

    out_ << " [";

    bool need_comma = false;
    auto comma = [&] {
        if (need_comma) {
            out_ << ", ";
        }
    };

    if (p->Invariant()) {
        comma();
        out_ << StyleAttribute("@invariant");
        need_comma = true;
    }
    if (p->Location().has_value()) {
        EmitLocation(p->Location().value());
        need_comma = true;
    }
    if (p->BindingPoint().has_value()) {
        comma();
        EmitBindingPoint(p->BindingPoint().value());
        need_comma = true;
    }
    if (p->Builtin().has_value()) {
        comma();
        out_ << StyleAttribute("@", p->Builtin().value());
        need_comma = true;
    }
    out_ << "]";
}

void Disassembly::EmitReturnAttributes(const Function* func) {
    if (!func->ReturnInvariant() && !func->ReturnLocation().has_value() &&
        !func->ReturnBuiltin().has_value()) {
        return;
    }

    out_ << " [";

    bool need_comma = false;
    auto comma = [&] {
        if (need_comma) {
            out_ << ", ";
        }
    };
    if (func->ReturnInvariant()) {
        comma();
        out_ << StyleAttribute("@invariant");
        need_comma = true;
    }
    if (func->ReturnLocation().has_value()) {
        comma();
        EmitLocation(func->ReturnLocation().value());
        need_comma = true;
    }
    if (func->ReturnBuiltin().has_value()) {
        comma();
        out_ << StyleAttribute("@", func->ReturnBuiltin().value());
        need_comma = true;
    }
    out_ << "]";
}

void Disassembly::EmitFunction(const Function* func) {
    in_function_ = true;

    auto fn_id = NameOf(func);
    {
        SourceMarker sm(this);
        Indent() << fn_id;
        sm.Store(func);
    }
    out_ << " =";

    if (func->Stage() != Function::PipelineStage::kUndefined) {
        out_ << " " << StyleAttribute("@", func->Stage());
    }
    if (func->WorkgroupSize()) {
        auto arr = func->WorkgroupSize().value();
        out_ << " " << StyleAttribute("@workgroup_size") << "(" << StyleLiteral(arr[0]) << ", "
             << StyleLiteral(arr[1]) << ", " << StyleLiteral(arr[2]) << ")";
    }

    out_ << " " << StyleKeyword("func") << "(";

    for (auto* p : func->Params()) {
        if (p != func->Params().Front()) {
            out_ << ", ";
        }
        SourceMarker sm(this);
        out_ << NameOf(p) << ":" << StyleType(p->Type() ? p->Type()->FriendlyName() : "undef");
        sm.Store(p);

        EmitParamAttributes(p);
    }
    out_ << "):" << StyleType(func->ReturnType()->FriendlyName());

    EmitReturnAttributes(func);

    out_ << " {";

    {  // Add a comment if the function IDs or parameter IDs doesn't match their name
        Vector<std::string, 4> names;
        if (auto name = mod_.NameOf(func); name.IsValid()) {
            if ("%" + name.Name() != fn_id.Plain()) {
                names.Push(fn_id.Plain() + ": '" + name.Name() + "'");
            }
        }
        for (auto* p : func->Params()) {
            if (auto name = mod_.NameOf(p); name.IsValid()) {
                auto id = NameOf(p);
                if ("%" + name.Name() != id.Plain()) {
                    names.Push(id.Plain() + ": '" + name.Name() + "'");
                }
            }
        }
        if (!names.IsEmpty()) {
            out_ << "  " << StyleComment("# ", tint::Join(names, ", "));
        }
    }

    EmitLine();

    {
        ScopedIndent si(indent_size_);
        EmitBlock(func->Block());
    }
    Indent() << "}";
    EmitLine();
}

void Disassembly::EmitValueWithType(const Instruction* val) {
    SourceMarker sm(this);
    EmitValueWithType(val->Result(0));
    sm.StoreResult(IndexedValue{val, 0});
}

void Disassembly::EmitValueWithType(const Value* val) {
    EmitValue(val);
    if (val) {
        out_ << ":" << StyleType(val->Type()->FriendlyName());
    }
}

void Disassembly::EmitValue(const Value* val) {
    if (!val) {
        out_ << StyleLiteral("undef");
        return;
    }
    tint::Switch(
        val,
        [&](const ir::Constant* constant) {
            std::function<void(const core::constant::Value*)> emit =
                [&](const core::constant::Value* c) {
                    tint::Switch(
                        c,
                        [&](const core::constant::Scalar<AFloat>* scalar) {
                            out_ << StyleLiteral(scalar->ValueAs<AFloat>().value);
                        },
                        [&](const core::constant::Scalar<AInt>* scalar) {
                            out_ << StyleLiteral(scalar->ValueAs<AInt>().value);
                        },
                        [&](const core::constant::Scalar<i32>* scalar) {
                            out_ << StyleLiteral(scalar->ValueAs<i32>().value, "i");
                        },
                        [&](const core::constant::Scalar<u32>* scalar) {
                            out_ << StyleLiteral(scalar->ValueAs<u32>().value, "u");
                        },
                        [&](const core::constant::Scalar<f32>* scalar) {
                            out_ << StyleLiteral(scalar->ValueAs<f32>().value, "f");
                        },
                        [&](const core::constant::Scalar<f16>* scalar) {
                            out_ << StyleLiteral(scalar->ValueAs<f16>().value, "h");
                        },
                        [&](const core::constant::Scalar<bool>* scalar) {
                            out_ << StyleLiteral((scalar->ValueAs<bool>() ? "true" : "false"));
                        },
                        [&](const core::constant::Splat* splat) {
                            out_ << StyleType(splat->Type()->FriendlyName()) << "(";
                            emit(splat->Index(0));
                            out_ << ")";
                        },
                        [&](const core::constant::Composite* composite) {
                            out_ << StyleType(composite->Type()->FriendlyName()) << "(";
                            bool need_comma = false;
                            for (const auto* elem : composite->elements) {
                                if (need_comma) {
                                    out_ << ", ";
                                }
                                emit(elem);
                                need_comma = true;
                            }
                            out_ << ")";
                        });
                };
            emit(constant->Value());
        },
        [&](Default) { out_ << NameOf(val); });
}

void Disassembly::EmitInstructionName(const Instruction* inst) {
    SourceMarker sm(this);
    out_ << StyleInstruction(inst->FriendlyName());
    sm.Store(inst);
}

void Disassembly::EmitInstruction(const Instruction* inst) {
    TINT_DEFER(EmitLine());

    if (!inst->Alive()) {
        SourceMarker sm(this);
        out_ << StyleError("<destroyed ", inst->TypeInfo().name, " ", inst, ">");
        sm.Store(inst);
        return;
    }
    tint::Switch(
        inst,                                     //
        [&](const Switch* s) { EmitSwitch(s); },  //
        [&](const If* i) { EmitIf(i); },          //
        [&](const Loop* l) { EmitLoop(l); },      //
        [&](const Binary* b) { EmitBinary(b); },  //
        [&](const Unary* u) { EmitUnary(u); },    //
        [&](const Discard* d) { EmitInstructionName(d); },
        [&](const Store* s) {
            EmitInstructionName(s);
            out_ << " ";
            EmitOperand(s, Store::kToOperandOffset);
            out_ << ", ";
            EmitOperand(s, Store::kFromOperandOffset);
        },
        [&](const StoreVectorElement* s) {
            EmitInstructionName(s);
            out_ << " ";
            EmitOperandList(s);
        },
        [&](const UserCall* uc) {
            EmitValueWithType(uc);
            out_ << " = ";
            EmitInstructionName(uc);
            out_ << " ";
            EmitOperand(uc, UserCall::kFunctionOperandOffset);
            if (!uc->Args().IsEmpty()) {
                out_ << ", ";
            }
            EmitOperandList(uc, UserCall::kArgsOperandOffset);
        },
        [&](const Var* v) {
            EmitValueWithType(v);
            out_ << " = ";
            EmitInstructionName(v);
            if (v->Initializer()) {
                out_ << ", ";
                EmitOperand(v, Var::kInitializerOperandOffset);
            }
            if (v->BindingPoint().has_value()) {
                out_ << " ";
                EmitBindingPoint(v->BindingPoint().value());
            }
            if (v->Attributes().invariant) {
                out_ << " " << StyleAttribute("@invariant");
            }
            if (v->Attributes().location.has_value()) {
                out_ << " " << StyleAttribute("@location") << "("
                     << v->Attributes().location.value() << ")";
            }
            if (v->Attributes().blend_src.has_value()) {
                out_ << " " << StyleAttribute("@blend_src") << "("
                     << v->Attributes().blend_src.value() << ")";
            }
            if (v->Attributes().interpolation.has_value()) {
                auto& interp = v->Attributes().interpolation.value();
                out_ << " " << StyleAttribute("@interpolate") << "(" << interp.type;
                if (interp.sampling != core::InterpolationSampling::kUndefined) {
                    out_ << ", " << interp.sampling;
                }
                out_ << ")";
            }
            if (v->Attributes().builtin.has_value()) {
                out_ << " " << StyleAttribute("@builtin") << "(" << v->Attributes().builtin.value()
                     << ")";
            }
        },
        [&](const Swizzle* s) {
            EmitValueWithType(s);
            out_ << " = ";
            EmitInstructionName(s);
            out_ << " ";
            EmitValue(s->Object());
            out_ << ", ";
            for (auto idx : s->Indices()) {
                switch (idx) {
                    case 0:
                        out_ << "x";
                        break;
                    case 1:
                        out_ << "y";
                        break;
                    case 2:
                        out_ << "z";
                        break;
                    case 3:
                        out_ << "w";
                        break;
                }
            }
        },
        [&](const Terminator* b) { EmitTerminator(b); },
        [&](Default) {
            EmitValueWithType(inst);
            out_ << " = ";
            EmitInstructionName(inst);
            if (!inst->Operands().IsEmpty()) {
                out_ << " ";
                EmitOperandList(inst);
            }
        });

    {  // Add a comment if the result IDs don't match their names
        Vector<std::string, 4> names;
        for (auto* result : inst->Results()) {
            if (result) {
                if (auto name = mod_.NameOf(result); name.IsValid()) {
                    auto id = NameOf(result).Plain();
                    if ("%" + name.Name() != id) {
                        names.Push(id + ": '" + name.Name() + "'");
                    }
                }
            }
        }
        if (!names.IsEmpty()) {
            out_ << "  # " << tint::Join(names, ", ");
        }
    }
}

void Disassembly::EmitOperand(const Instruction* inst, size_t index) {
    SourceMarker marker(this);
    EmitValue(inst->Operands()[index]);
    marker.Store(IndexedValue{inst, static_cast<uint32_t>(index)});
}

void Disassembly::EmitOperandList(const Instruction* inst, size_t start_index /* = 0 */) {
    for (size_t i = start_index, n = inst->Operands().Length(); i < n; i++) {
        if (i != start_index) {
            out_ << ", ";
        }
        EmitOperand(inst, i);
    }
}

void Disassembly::EmitOperandList(const Instruction* inst, size_t start_index, size_t count) {
    size_t n = std::min(count, inst->Operands().Length());
    for (size_t i = start_index; i < n; i++) {
        if (i != start_index) {
            out_ << ", ";
        }
        EmitOperand(inst, i);
    }
}

void Disassembly::EmitIf(const If* if_) {
    SourceMarker sm(this);
    if (auto results = if_->Results(); !results.IsEmpty()) {
        for (size_t i = 0; i < results.Length(); ++i) {
            if (i > 0) {
                out_ << ", ";
            }
            SourceMarker rs(this);
            EmitValueWithType(results[i]);
            rs.StoreResult(IndexedValue{if_, i});
        }
        out_ << " = ";
    }
    out_ << StyleInstruction("if") << " ";
    EmitOperand(if_, If::kConditionOperandOffset);

    bool has_false = !if_->False()->IsEmpty();

    out_ << " [" << StyleKeyword("t") << ": " << NameOf(if_->True());
    if (has_false) {
        out_ << ", " << StyleKeyword("f") << ": " << NameOf(if_->False());
    }
    out_ << "]";
    sm.Store(if_);

    out_ << " {  " << StyleComment("# ", NameOf(if_));
    EmitLine();

    // True block is assumed to have instructions
    {
        ScopedIndent si(indent_size_);
        EmitBlock(if_->True(), "true");
    }

    if (has_false) {
        ScopedIndent si(indent_size_);
        EmitBlock(if_->False(), "false");
    } else if (auto results = if_->Results(); !results.IsEmpty()) {
        ScopedIndent si(indent_size_);
        Indent();
        out_ << StyleComment("# implicit false block: exit_if undef");
        for (size_t v = 1; v < if_->Results().Length(); v++) {
            out_ << StyleComment(", undef");
        }
        EmitLine();
    }

    Indent();
    out_ << "}";
}

void Disassembly::EmitLoop(const Loop* l) {
    SourceMarker sm(this);
    if (auto results = l->Results(); !results.IsEmpty()) {
        for (size_t i = 0; i < results.Length(); ++i) {
            if (i > 0) {
                out_ << ", ";
            }
            SourceMarker rs(this);
            EmitValueWithType(results[i]);
            rs.StoreResult(IndexedValue{l, i});
        }
        out_ << " = ";
    }
    out_ << StyleInstruction("loop") << " [";

    if (!l->Initializer()->IsEmpty()) {
        out_ << StyleKeyword("i") << ": " << NameOf(l->Initializer());
        out_ << ", ";
    }

    out_ << StyleKeyword("b") << ": " << NameOf(l->Body());

    if (!l->Continuing()->IsEmpty()) {
        out_ << ", ";
        out_ << StyleKeyword("c") << ": " << NameOf(l->Continuing());
    }

    out_ << "]";
    sm.Store(l);

    out_ << " {  " << StyleComment("# ", NameOf(l));
    EmitLine();

    if (!l->Initializer()->IsEmpty()) {
        ScopedIndent si(indent_size_);
        EmitBlock(l->Initializer(), "initializer");
    }

    // Loop is assumed to always have a body
    {
        ScopedIndent si(indent_size_);
        EmitBlock(l->Body(), "body");
    }

    if (!l->Continuing()->IsEmpty()) {
        ScopedIndent si(indent_size_);
        EmitBlock(l->Continuing(), "continuing");
    }

    Indent();
    out_ << "}";
}

void Disassembly::EmitSwitch(const Switch* s) {
    SourceMarker sm(this);
    if (auto results = s->Results(); !results.IsEmpty()) {
        for (size_t i = 0; i < results.Length(); ++i) {
            if (i > 0) {
                out_ << ", ";
            }
            SourceMarker rs(this);
            EmitValueWithType(results[i]);
            rs.StoreResult(IndexedValue{s, i});
        }
        out_ << " = ";
    }
    out_ << StyleInstruction("switch") << " ";
    EmitValue(s->Condition());
    out_ << " [";
    for (auto& c : s->Cases()) {
        if (&c != &s->Cases().Front()) {
            out_ << ", ";
        }
        out_ << "c: (";
        for (auto& selector : c.selectors) {
            if (&selector != &c.selectors.Front()) {
                out_ << " ";
            }

            if (selector.IsDefault()) {
                out_ << StyleKeyword("default");
            } else {
                EmitValue(selector.val);
            }
        }
        out_ << ", " << NameOf(c.block) << ")";
    }
    out_ << "]";
    sm.Store(s);

    out_ << " {  " << StyleComment("# ", NameOf(s));
    EmitLine();

    for (auto& c : s->Cases()) {
        ScopedIndent si(indent_size_);
        EmitBlock(c.block, "case");
    }

    Indent();
    out_ << "}";
}

void Disassembly::EmitTerminator(const Terminator* term) {
    SourceMarker sm(this);
    auto args_offset = tint::Switch<std::optional<size_t>>(
        term,
        [&](const ir::Return*) {
            out_ << StyleInstruction("ret");
            return ir::Return::kArgsOperandOffset;
        },
        [&](const ir::Continue*) {
            out_ << StyleInstruction("continue");
            return ir::Continue::kArgsOperandOffset;
        },
        [&](const ir::ExitIf*) {
            out_ << StyleInstruction("exit_if");
            return ir::ExitIf::kArgsOperandOffset;
        },
        [&](const ir::ExitSwitch*) {
            out_ << StyleInstruction("exit_switch");
            return ir::ExitSwitch::kArgsOperandOffset;
        },
        [&](const ir::ExitLoop*) {
            out_ << StyleInstruction("exit_loop");
            return ir::ExitLoop::kArgsOperandOffset;
        },
        [&](const ir::NextIteration*) {
            out_ << StyleInstruction("next_iteration");
            return ir::NextIteration::kArgsOperandOffset;
        },
        [&](const ir::Unreachable*) {
            out_ << StyleInstruction("unreachable");
            return std::nullopt;
        },
        [&](const ir::BreakIf* bi) {
            out_ << StyleInstruction("break_if");
            out_ << " ";
            EmitValue(bi->Condition());
            auto next_iter_values = bi->NextIterValues();
            auto exit_values = bi->ExitValues();
            if (!next_iter_values.IsEmpty()) {
                out_ << " " << StyleLabel("next_iteration") << ": [ ";
                EmitOperandList(bi, ir::BreakIf::kArgsOperandOffset, next_iter_values.Length());
                out_ << " ]";
            }
            if (!exit_values.IsEmpty()) {
                out_ << " " << StyleLabel("exit_loop") << ": [ ";
                EmitOperandList(bi, ir::BreakIf::kArgsOperandOffset + next_iter_values.Length());
                out_ << " ]";
            }
            return std::nullopt;
        },
        [&](const ir::TerminateInvocation*) {
            out_ << StyleInstruction("terminate_invocation");
            return std::nullopt;
        },
        [&](Default) {
            out_ << StyleError("unknown terminator ", term->TypeInfo().name);
            return std::nullopt;
        });

    if (args_offset && !term->Args().IsEmpty()) {
        out_ << " ";
        EmitOperandList(term, *args_offset);
    }

    sm.Store(term);

    tint::Switch(
        term,  //
        [&](const ir::BreakIf* bi) {
            out_ << "  "
                 << StyleComment("# -> [t: exit_loop ", NameOf(bi->Loop()),
                                 ", f: ", NameOf(bi->Loop()->Body()), "]");
        },
        [&](const ir::Continue* c) {
            out_ << "  " << StyleComment("# -> ", NameOf(c->Loop()->Continuing()));
        },                                                                                  //
        [&](const ir::ExitIf* e) { out_ << "  " << StyleComment("# ", NameOf(e->If())); },  //
        [&](const ir::ExitSwitch* e) {
            out_ << "  " << StyleComment("# ", NameOf(e->Switch()));
        },                                                                                      //
        [&](const ir::ExitLoop* e) { out_ << "  " << StyleComment("# ", NameOf(e->Loop())); },  //
        [&](const ir::NextIteration* ni) {
            out_ << "  " << StyleComment("# -> ", NameOf(ni->Loop()->Body()));
        });
}

void Disassembly::EmitBinary(const Binary* b) {
    SourceMarker sm(this);
    EmitValueWithType(b);
    out_ << " = " << NameOf(b->Op()) << " ";
    EmitOperandList(b);

    sm.Store(b);
}

void Disassembly::EmitUnary(const Unary* u) {
    SourceMarker sm(this);
    EmitValueWithType(u);
    out_ << " = " << NameOf(u->Op()) << " ";
    EmitOperandList(u);

    sm.Store(u);
}

void Disassembly::EmitStructDecl(const core::type::Struct* str) {
    out_ << StyleType(str->Name().Name()) << " = " << StyleKeyword("struct") << " "
         << StyleAttribute("@align") << "(" << StyleLiteral(str->Align()) << ")";
    if (str->StructFlags().Contains(core::type::StructFlag::kBlock)) {
        out_ << ", " << StyleAttribute("@block");
    }
    out_ << " {";
    EmitLine();
    for (auto* member : str->Members()) {
        out_ << "  " << StyleVariable(member->Name().Name()) << ":"
             << StyleType(member->Type()->FriendlyName());
        out_ << " " << StyleAttribute("@offset") << "(" << StyleLiteral(member->Offset()) << ")";
        if (member->Attributes().invariant) {
            out_ << ", " << StyleAttribute("@invariant");
        }
        if (member->Attributes().location.has_value()) {
            out_ << ", " << StyleAttribute("@location") << "("
                 << StyleLiteral(member->Attributes().location.value()) << ")";
        }
        if (member->Attributes().interpolation.has_value()) {
            auto& interp = member->Attributes().interpolation.value();
            out_ << ", " << StyleAttribute("@interpolate") << "(" << StyleEnum(interp.type);
            if (interp.sampling != core::InterpolationSampling::kUndefined) {
                out_ << ", " << StyleEnum(interp.sampling);
            }
            out_ << ")";
        }
        if (member->Attributes().builtin.has_value()) {
            out_ << ", " << StyleAttribute("@builtin") << "("
                 << StyleLiteral(member->Attributes().builtin.value()) << ")";
        }
        EmitLine();
    }
    out_ << "}";
    EmitLine();
    EmitLine();
}

StyledText Disassembly::NameOf(const Block* node) {
    TINT_ASSERT(node);
    auto id = block_ids_.GetOrAdd(node, [&] { return block_ids_.Count(); });
    return StyledText{} << StyleLabel("$B", id);
}

StyledText Disassembly::NameOf(const Value* value) {
    TINT_ASSERT(value);
    auto id = value_ids_.GetOrAdd(value, [&] {
        if (auto sym = mod_.NameOf(value)) {
            if (ids_.Add(sym.Name())) {
                return sym.Name();
            }
            auto prefix = sym.Name() + "_";
            for (size_t i = 1;; i++) {
                auto name = prefix + std::to_string(i);
                if (ids_.Add(name)) {
                    return name;
                }
            }
        }
        return std::to_string(value_ids_.Count());
    });

    auto style = tint::Switch(
        value,                                           //
        [&](const Function*) { return StyleFunction; },  //
        [&](const InstructionResult*) { return StyleVariable; });
    return StyledText{} << style("%", id);
}

StyledText Disassembly::NameOf(const If* inst) {
    if (!inst) {
        return StyledText{} << StyleError("undef");
    }

    auto name = if_names_.GetOrAdd(inst, [&] { return "if_" + std::to_string(if_names_.Count()); });
    return StyledText{} << StyleInstruction(name);
}

StyledText Disassembly::NameOf(const Loop* inst) {
    if (!inst) {
        return StyledText{} << StyleError("undef");
    }

    auto name =
        loop_names_.GetOrAdd(inst, [&] { return "loop_" + std::to_string(loop_names_.Count()); });
    return StyledText{} << StyleInstruction(name);
}

StyledText Disassembly::NameOf(const Switch* inst) {
    if (!inst) {
        return StyledText{} << StyleError("undef");
    }

    auto name = switch_names_.GetOrAdd(
        inst, [&] { return "switch_" + std::to_string(switch_names_.Count()); });
    return StyledText{} << StyleInstruction(name);
}

StyledText Disassembly::NameOf(BinaryOp op) {
    switch (op) {
        case BinaryOp::kAdd:
            return StyledText{} << StyleInstruction("add");
        case BinaryOp::kSubtract:
            return StyledText{} << StyleInstruction("sub");
        case BinaryOp::kMultiply:
            return StyledText{} << StyleInstruction("mul");
        case BinaryOp::kDivide:
            return StyledText{} << StyleInstruction("div");
        case BinaryOp::kModulo:
            return StyledText{} << StyleInstruction("mod");
        case BinaryOp::kAnd:
            return StyledText{} << StyleInstruction("and");
        case BinaryOp::kOr:
            return StyledText{} << StyleInstruction("or");
        case BinaryOp::kXor:
            return StyledText{} << StyleInstruction("xor");
        case BinaryOp::kEqual:
            return StyledText{} << StyleInstruction("eq");
        case BinaryOp::kNotEqual:
            return StyledText{} << StyleInstruction("neq");
        case BinaryOp::kLessThan:
            return StyledText{} << StyleInstruction("lt");
        case BinaryOp::kGreaterThan:
            return StyledText{} << StyleInstruction("gt");
        case BinaryOp::kLessThanEqual:
            return StyledText{} << StyleInstruction("lte");
        case BinaryOp::kGreaterThanEqual:
            return StyledText{} << StyleInstruction("gte");
        case BinaryOp::kShiftLeft:
            return StyledText{} << StyleInstruction("shl");
        case BinaryOp::kShiftRight:
            return StyledText{} << StyleInstruction("shr");
        case BinaryOp::kLogicalAnd:
            return StyledText{} << StyleInstruction("logical-and");
        case BinaryOp::kLogicalOr:
            return StyledText{} << StyleInstruction("logical-or");
    }
    TINT_UNREACHABLE() << op;
}

StyledText Disassembly::NameOf(UnaryOp op) {
    switch (op) {
        case UnaryOp::kComplement:
            return StyledText{} << StyleInstruction("complement");
        case UnaryOp::kNegation:
            return StyledText{} << StyleInstruction("negation");
        case UnaryOp::kAddressOf:
            return StyledText{} << StyleInstruction("ref-to-ptr");
        case UnaryOp::kIndirection:
            return StyledText{} << StyleInstruction("ptr-to-ref");
        case UnaryOp::kNot:
            return StyledText{} << StyleInstruction("not");
    }
    TINT_UNREACHABLE() << op;
}

}  // namespace tint::core::ir
