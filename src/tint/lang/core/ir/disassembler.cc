// Copyright 2022 The Tint Authors.
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

#include "src/tint/lang/core/ir/disassembler.h"

#include "src//tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/constant/composite.h"
#include "src/tint/lang/core/constant/scalar.h"
#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/bitcast.h"
#include "src/tint/lang/core/ir/block.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/intrinsic_call.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
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
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"

namespace tint::ir {
namespace {

class ScopedIndent {
  public:
    explicit ScopedIndent(uint32_t& indent) : indent_(indent) { indent_ += 2; }

    ~ScopedIndent() { indent_ -= 2; }

  private:
    uint32_t& indent_;
};

}  // namespace

Disassembler::Disassembler(Module& mod) : mod_(mod) {}

Disassembler::~Disassembler() = default;

StringStream& Disassembler::Indent() {
    for (uint32_t i = 0; i < indent_size_; i++) {
        out_ << " ";
    }
    return out_;
}

void Disassembler::EmitLine() {
    out_ << std::endl;
    current_output_line_ += 1;
    current_output_start_pos_ = out_.tellp();
}

size_t Disassembler::IdOf(Block* node) {
    TINT_ASSERT(node);
    return block_ids_.GetOrCreate(node, [&] { return block_ids_.Count(); });
}

std::string Disassembler::IdOf(Value* value) {
    TINT_ASSERT(value);
    return value_ids_.GetOrCreate(value, [&] {
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
}

std::string Disassembler::NameOf(If* inst) {
    if (!inst) {
        return "undef";
    }

    return if_names_.GetOrCreate(inst, [&] { return "if_" + std::to_string(if_names_.Count()); });
}

std::string Disassembler::NameOf(Loop* inst) {
    if (!inst) {
        return "undef";
    }

    return loop_names_.GetOrCreate(inst,
                                   [&] { return "loop_" + std::to_string(loop_names_.Count()); });
}

std::string Disassembler::NameOf(Switch* inst) {
    if (!inst) {
        return "undef";
    }

    return switch_names_.GetOrCreate(
        inst, [&] { return "switch_" + std::to_string(switch_names_.Count()); });
}

Source::Location Disassembler::MakeCurrentLocation() {
    return Source::Location{current_output_line_, out_.tellp() - current_output_start_pos_ + 1};
}

std::string Disassembler::Disassemble() {
    for (auto* ty : mod_.Types()) {
        if (auto* str = ty->As<type::Struct>()) {
            EmitStructDecl(str);
        }
    }

    if (mod_.root_block) {
        EmitBlock(mod_.root_block, "root");
        EmitLine();
    }

    for (auto* func : mod_.functions) {
        EmitFunction(func);
    }
    return out_.str();
}

void Disassembler::EmitBlock(Block* blk, std::string_view comment /* = "" */) {
    Indent();

    SourceMarker sm(this);
    out_ << "%b" << IdOf(blk) << " = block";
    if (auto* merge = blk->As<MultiInBlock>()) {
        if (!merge->Params().IsEmpty()) {
            out_ << " (";
            EmitValueList(merge->Params().Slice());
            out_ << ")";
        }
    }
    sm.Store(blk);

    out_ << " {";
    if (!comment.empty()) {
        out_ << "  # " << comment;
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

void Disassembler::EmitBindingPoint(BindingPoint p) {
    out_ << "@binding_point(" << p.group << ", " << p.binding << ")";
}

void Disassembler::EmitLocation(Location loc) {
    out_ << "@location(" << loc.value << ")";
    if (loc.interpolation.has_value()) {
        out_ << ", @interpolate(";
        out_ << loc.interpolation->type;
        if (loc.interpolation->sampling != core::InterpolationSampling::kUndefined) {
            out_ << ", ";
            out_ << loc.interpolation->sampling;
        }
        out_ << ")";
    }
}

void Disassembler::EmitParamAttributes(FunctionParam* p) {
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
        out_ << "@invariant";
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
        out_ << "@" << p->Builtin().value();
        need_comma = true;
    }
    out_ << "]";
}

void Disassembler::EmitReturnAttributes(Function* func) {
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
        out_ << "@invariant";
        need_comma = true;
    }
    if (func->ReturnLocation().has_value()) {
        comma();
        EmitLocation(func->ReturnLocation().value());
        need_comma = true;
    }
    if (func->ReturnBuiltin().has_value()) {
        comma();
        out_ << "@" << func->ReturnBuiltin().value();
        need_comma = true;
    }
    out_ << "]";
}

void Disassembler::EmitFunction(Function* func) {
    in_function_ = true;

    std::string fn_id = IdOf(func);
    Indent() << "%" << fn_id << " =";

    if (func->Stage() != Function::PipelineStage::kUndefined) {
        out_ << " @" << func->Stage();
    }
    if (func->WorkgroupSize()) {
        auto arr = func->WorkgroupSize().value();
        out_ << " @workgroup_size(" << arr[0] << ", " << arr[1] << ", " << arr[2] << ")";
    }

    out_ << " func(";

    for (auto* p : func->Params()) {
        if (p != func->Params().Front()) {
            out_ << ", ";
        }
        out_ << "%" << IdOf(p) << ":" << p->Type()->FriendlyName();

        EmitParamAttributes(p);
    }
    out_ << "):" << func->ReturnType()->FriendlyName();

    EmitReturnAttributes(func);

    out_ << " -> %b" << IdOf(func->Block()) << " {";

    {  // Add a comment if the function IDs or parameter IDs doesn't match their name
        Vector<std::string, 4> names;
        if (auto name = mod_.NameOf(func); name.IsValid()) {
            if (name.NameView() != fn_id) {
                names.Push("%" + std::string(fn_id) + ": '" + name.Name() + "'");
            }
        }
        for (auto* p : func->Params()) {
            if (auto name = mod_.NameOf(p); name.IsValid()) {
                auto id = IdOf(p);
                if (name.NameView() != id) {
                    names.Push("%" + std::string(id) + ": '" + name.Name() + "'");
                }
            }
        }
        if (!names.IsEmpty()) {
            out_ << "  # " << tint::Join(names, ", ");
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

void Disassembler::EmitValueWithType(Instruction* val) {
    SourceMarker sm(this);
    if (val->Result()) {
        EmitValueWithType(val->Result());
    } else {
        out_ << "undef";
    }
    sm.StoreResult(Usage{val, 0});
}

void Disassembler::EmitValueWithType(Value* val) {
    if (!val) {
        out_ << "undef";
        return;
    }

    EmitValue(val);
    out_ << ":" << val->Type()->FriendlyName();
}

void Disassembler::EmitValue(Value* val) {
    tint::Switch(
        val,
        [&](ir::Constant* constant) {
            std::function<void(const constant::Value*)> emit = [&](const constant::Value* c) {
                tint::Switch(
                    c,
                    [&](const constant::Scalar<AFloat>* scalar) {
                        out_ << scalar->ValueAs<AFloat>().value;
                    },
                    [&](const constant::Scalar<AInt>* scalar) {
                        out_ << scalar->ValueAs<AInt>().value;
                    },
                    [&](const constant::Scalar<i32>* scalar) {
                        out_ << scalar->ValueAs<i32>().value << "i";
                    },
                    [&](const constant::Scalar<u32>* scalar) {
                        out_ << scalar->ValueAs<u32>().value << "u";
                    },
                    [&](const constant::Scalar<f32>* scalar) {
                        out_ << scalar->ValueAs<f32>().value << "f";
                    },
                    [&](const constant::Scalar<f16>* scalar) {
                        out_ << scalar->ValueAs<f16>().value << "h";
                    },
                    [&](const constant::Scalar<bool>* scalar) {
                        out_ << (scalar->ValueAs<bool>() ? "true" : "false");
                    },
                    [&](const constant::Splat* splat) {
                        out_ << splat->Type()->FriendlyName() << "(";
                        emit(splat->Index(0));
                        out_ << ")";
                    },
                    [&](const constant::Composite* composite) {
                        out_ << composite->Type()->FriendlyName() << "(";
                        for (const auto* elem : composite->elements) {
                            if (elem != composite->elements[0]) {
                                out_ << ", ";
                            }
                            emit(elem);
                        }
                        out_ << ")";
                    });
            };
            emit(constant->Value());
        },
        [&](ir::InstructionResult* rv) { out_ << "%" << IdOf(rv); },
        [&](ir::BlockParam* p) { out_ << "%" << IdOf(p) << ":" << p->Type()->FriendlyName(); },
        [&](ir::FunctionParam* p) { out_ << "%" << IdOf(p); },
        [&](Default) {
            if (val == nullptr) {
                out_ << "undef";
            } else {
                out_ << "Unknown value: " << val->TypeInfo().name;
            }
        });
}

void Disassembler::EmitInstructionName(std::string_view name, Instruction* inst) {
    SourceMarker sm(this);
    out_ << name;
    sm.Store(inst);
}

void Disassembler::EmitInstruction(Instruction* inst) {
    TINT_DEFER(EmitLine());

    if (!inst->Alive()) {
        SourceMarker sm(this);
        out_ << "<destroyed " << inst->TypeInfo().name << " " << tint::ToString(inst) << ">";
        sm.Store(inst);
        return;
    }
    tint::Switch(
        inst,                               //
        [&](Switch* s) { EmitSwitch(s); },  //
        [&](If* i) { EmitIf(i); },          //
        [&](Loop* l) { EmitLoop(l); },      //
        [&](Binary* b) { EmitBinary(b); },  //
        [&](Unary* u) { EmitUnary(u); },    //
        [&](Bitcast* b) {
            EmitValueWithType(b);
            out_ << " = ";
            EmitInstructionName("bitcast", b);
            out_ << " ";
            EmitOperandList(b);
        },
        [&](Discard* d) { EmitInstructionName("discard", d); },
        [&](CoreBuiltinCall* b) {
            EmitValueWithType(b);
            out_ << " = ";
            EmitInstructionName(core::str(b->Func()), b);
            out_ << " ";
            EmitOperandList(b);
        },
        [&](Construct* c) {
            EmitValueWithType(c);
            out_ << " = ";
            EmitInstructionName("construct", c);
            if (!c->Operands().IsEmpty()) {
                out_ << " ";
                EmitOperandList(c);
            }
        },
        [&](Convert* c) {
            EmitValueWithType(c);
            out_ << " = ";
            EmitInstructionName("convert", c);
            out_ << " ";
            EmitOperandList(c);
        },
        [&](IntrinsicCall* i) {
            EmitValueWithType(i);
            out_ << " = ";
            EmitInstructionName(tint::ToString(i->Kind()), i);
            out_ << " ";
            EmitOperandList(i);
        },
        [&](Load* l) {
            EmitValueWithType(l);
            out_ << " = ";
            EmitInstructionName("load", l);
            out_ << " ";
            EmitValue(l->From());
        },
        [&](Store* s) {
            EmitInstructionName("store", s);
            out_ << " ";
            EmitValue(s->To());
            out_ << ", ";
            EmitValue(s->From());
        },
        [&](LoadVectorElement* l) {
            EmitValueWithType(l);
            out_ << " = ";
            EmitInstructionName("load_vector_element", l);
            out_ << " ";
            EmitOperandList(l);
        },
        [&](StoreVectorElement* s) {
            EmitInstructionName("store_vector_element", s);
            out_ << " ";
            EmitOperandList(s);
        },
        [&](UserCall* uc) {
            EmitValueWithType(uc);
            out_ << " = ";
            EmitInstructionName("call", uc);
            out_ << " %" << IdOf(uc->Func());
            if (!uc->Args().IsEmpty()) {
                out_ << ", ";
            }
            EmitOperandList(uc, UserCall::kArgsOperandOffset);
        },
        [&](Var* v) {
            EmitValueWithType(v);
            out_ << " = ";
            EmitInstructionName("var", v);
            if (v->Initializer()) {
                out_ << ", ";
                EmitOperand(v, Var::kInitializerOperandOffset);
            }
            if (v->BindingPoint().has_value()) {
                out_ << " ";
                EmitBindingPoint(v->BindingPoint().value());
            }
        },
        [&](Let* l) {
            EmitValueWithType(l);
            out_ << " = ";
            EmitInstructionName("let", l);
            out_ << " ";
            EmitOperandList(l);
        },
        [&](Access* a) {
            EmitValueWithType(a);
            out_ << " = ";
            EmitInstructionName("access", a);
            out_ << " ";
            EmitOperandList(a);
        },
        [&](Swizzle* s) {
            EmitValueWithType(s);
            out_ << " = ";
            EmitInstructionName("swizzle", s);
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
        [&](Terminator* b) { EmitTerminator(b); },
        [&](Default) { out_ << "Unknown instruction: " << inst->TypeInfo().name; });

    {  // Add a comment if the result IDs don't match their names
        Vector<std::string, 4> names;
        for (auto* result : inst->Results()) {
            if (result) {
                if (auto name = mod_.NameOf(result); name.IsValid()) {
                    auto id = IdOf(result);
                    if (name.NameView() != id) {
                        names.Push("%" + std::string(id) + ": '" + name.Name() + "'");
                    }
                }
            }
        }
        if (!names.IsEmpty()) {
            out_ << "  # " << tint::Join(names, ", ");
        }
    }
}

void Disassembler::EmitOperand(Instruction* inst, size_t index) {
    SourceMarker condMarker(this);
    EmitValue(inst->Operands()[index]);
    condMarker.Store(Usage{inst, static_cast<uint32_t>(index)});
}

void Disassembler::EmitOperandList(Instruction* inst, size_t start_index /* = 0 */) {
    for (size_t i = start_index, n = inst->Operands().Length(); i < n; i++) {
        if (i != start_index) {
            out_ << ", ";
        }
        EmitOperand(inst, i);
    }
}

void Disassembler::EmitIf(If* if_) {
    SourceMarker sm(this);
    if (if_->HasResults()) {
        auto res = if_->Results();
        for (size_t i = 0; i < res.Length(); ++i) {
            if (i > 0) {
                out_ << ", ";
            }
            SourceMarker rs(this);
            EmitValueWithType(res[i]);
            rs.StoreResult(Usage{if_, i});
        }
        out_ << " = ";
    }
    out_ << "if ";
    EmitOperand(if_, If::kConditionOperandOffset);

    bool has_false = !if_->False()->IsEmpty();

    out_ << " [t: %b" << IdOf(if_->True());
    if (has_false) {
        out_ << ", f: %b" << IdOf(if_->False());
    }
    out_ << "]";
    sm.Store(if_);

    out_ << " {  # " << NameOf(if_);
    EmitLine();

    // True block is assumed to have instructions
    {
        ScopedIndent si(indent_size_);
        EmitBlock(if_->True(), "true");
    }

    if (has_false) {
        ScopedIndent si(indent_size_);
        EmitBlock(if_->False(), "false");
    } else if (if_->HasResults()) {
        ScopedIndent si(indent_size_);
        Indent();
        out_ << "# implicit false block: exit_if undef";
        for (size_t v = 1; v < if_->Results().Length(); v++) {
            out_ << ", undef";
        }
        EmitLine();
    }

    Indent();
    out_ << "}";
}

void Disassembler::EmitLoop(Loop* l) {
    Vector<std::string, 3> parts;
    if (!l->Initializer()->IsEmpty()) {
        parts.Push("i: %b" + std::to_string(IdOf(l->Initializer())));
    }
    parts.Push("b: %b" + std::to_string(IdOf(l->Body())));

    if (!l->Continuing()->IsEmpty()) {
        parts.Push("c: %b" + std::to_string(IdOf(l->Continuing())));
    }
    SourceMarker sm(this);
    if (l->HasResults()) {
        auto res = l->Results();
        for (size_t i = 0; i < res.Length(); ++i) {
            if (i > 0) {
                out_ << ", ";
            }
            SourceMarker rs(this);
            EmitValueWithType(res[i]);
            rs.StoreResult(Usage{l, i});
        }
        out_ << " = ";
    }
    out_ << "loop [" << tint::Join(parts, ", ") << "]";
    sm.Store(l);

    out_ << " {  # " << NameOf(l);
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

void Disassembler::EmitSwitch(Switch* s) {
    SourceMarker sm(this);
    if (s->HasResults()) {
        auto res = s->Results();
        for (size_t i = 0; i < res.Length(); ++i) {
            if (i > 0) {
                out_ << ", ";
            }
            SourceMarker rs(this);
            EmitValueWithType(res[i]);
            rs.StoreResult(Usage{s, i});
        }
        out_ << " = ";
    }
    out_ << "switch ";
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
                out_ << "default";
            } else {
                EmitValue(selector.val);
            }
        }
        out_ << ", %b" << IdOf(c.Block()) << ")";
    }
    out_ << "]";
    sm.Store(s);

    out_ << " {  # " << NameOf(s);
    EmitLine();

    for (auto& c : s->Cases()) {
        ScopedIndent si(indent_size_);
        EmitBlock(c.Block(), "case");
    }

    Indent();
    out_ << "}";
}

void Disassembler::EmitTerminator(Terminator* b) {
    SourceMarker sm(this);
    size_t args_offset = 0;
    tint::Switch(
        b,
        [&](ir::Return*) {
            out_ << "ret";
            args_offset = ir::Return::kArgOperandOffset;
        },
        [&](ir::Continue* cont) {
            out_ << "continue %b" << IdOf(cont->Loop()->Continuing());
            args_offset = ir::Continue::kArgsOperandOffset;
        },
        [&](ir::ExitIf*) {
            out_ << "exit_if";
            args_offset = ir::ExitIf::kArgsOperandOffset;
        },
        [&](ir::ExitSwitch*) {
            out_ << "exit_switch";
            args_offset = ir::ExitSwitch::kArgsOperandOffset;
        },
        [&](ir::ExitLoop*) {
            out_ << "exit_loop";
            args_offset = ir::ExitLoop::kArgsOperandOffset;
        },
        [&](ir::NextIteration* ni) {
            out_ << "next_iteration %b" << IdOf(ni->Loop()->Body());
            args_offset = ir::NextIteration::kArgsOperandOffset;
        },
        [&](ir::Unreachable*) { out_ << "unreachable"; },
        [&](ir::BreakIf* bi) {
            out_ << "break_if ";
            EmitValue(bi->Condition());
            out_ << " %b" << IdOf(bi->Loop()->Body());
            args_offset = ir::BreakIf::kArgsOperandOffset;
        },
        [&](ir::TerminateInvocation*) { out_ << "terminate_invocation"; },
        [&](Default) { out_ << "unknown terminator " << b->TypeInfo().name; });

    if (!b->Args().IsEmpty()) {
        out_ << " ";
        EmitOperandList(b, args_offset);
    }
    sm.Store(b);

    tint::Switch(
        b,                                                                  //
        [&](ir::ExitIf* e) { out_ << "  # " << NameOf(e->If()); },          //
        [&](ir::ExitSwitch* e) { out_ << "  # " << NameOf(e->Switch()); },  //
        [&](ir::ExitLoop* e) { out_ << "  # " << NameOf(e->Loop()); }       //
    );
}

void Disassembler::EmitValueList(tint::Slice<Value* const> values) {
    for (size_t i = 0, n = values.Length(); i < n; i++) {
        if (i > 0) {
            out_ << ", ";
        }
        EmitValue(values[i]);
    }
}

void Disassembler::EmitBinary(Binary* b) {
    SourceMarker sm(this);
    EmitValueWithType(b);
    out_ << " = ";
    switch (b->Kind()) {
        case Binary::Kind::kAdd:
            out_ << "add";
            break;
        case Binary::Kind::kSubtract:
            out_ << "sub";
            break;
        case Binary::Kind::kMultiply:
            out_ << "mul";
            break;
        case Binary::Kind::kDivide:
            out_ << "div";
            break;
        case Binary::Kind::kModulo:
            out_ << "mod";
            break;
        case Binary::Kind::kAnd:
            out_ << "and";
            break;
        case Binary::Kind::kOr:
            out_ << "or";
            break;
        case Binary::Kind::kXor:
            out_ << "xor";
            break;
        case Binary::Kind::kEqual:
            out_ << "eq";
            break;
        case Binary::Kind::kNotEqual:
            out_ << "neq";
            break;
        case Binary::Kind::kLessThan:
            out_ << "lt";
            break;
        case Binary::Kind::kGreaterThan:
            out_ << "gt";
            break;
        case Binary::Kind::kLessThanEqual:
            out_ << "lte";
            break;
        case Binary::Kind::kGreaterThanEqual:
            out_ << "gte";
            break;
        case Binary::Kind::kShiftLeft:
            out_ << "shiftl";
            break;
        case Binary::Kind::kShiftRight:
            out_ << "shiftr";
            break;
    }
    out_ << " ";
    EmitOperandList(b);

    sm.Store(b);
}

void Disassembler::EmitUnary(Unary* u) {
    SourceMarker sm(this);
    EmitValueWithType(u);
    out_ << " = ";
    switch (u->Kind()) {
        case Unary::Kind::kComplement:
            out_ << "complement";
            break;
        case Unary::Kind::kNegation:
            out_ << "negation";
            break;
    }
    out_ << " ";
    EmitOperandList(u);

    sm.Store(u);
}

void Disassembler::EmitStructDecl(const type::Struct* str) {
    out_ << str->Name().Name() << " = struct @align(" << str->Align() << ")";
    if (str->StructFlags().Contains(type::StructFlag::kBlock)) {
        out_ << ", @block";
    }
    out_ << " {";
    EmitLine();
    for (auto* member : str->Members()) {
        out_ << "  " << member->Name().Name() << ":" << member->Type()->FriendlyName();
        out_ << " @offset(" << member->Offset() << ")";
        if (member->Attributes().invariant) {
            out_ << ", @invariant";
        }
        if (member->Attributes().location.has_value()) {
            out_ << ", @location(" << member->Attributes().location.value() << ")";
        }
        if (member->Attributes().interpolation.has_value()) {
            auto& interp = member->Attributes().interpolation.value();
            out_ << ", @interpolate(" << interp.type;
            if (interp.sampling != core::InterpolationSampling::kUndefined) {
                out_ << ", " << interp.sampling;
            }
            out_ << ")";
        }
        if (member->Attributes().builtin.has_value()) {
            out_ << ", @builtin(" << member->Attributes().builtin.value() << ")";
        }
        EmitLine();
    }
    out_ << "}";
    EmitLine();
    EmitLine();
}

}  // namespace tint::ir
