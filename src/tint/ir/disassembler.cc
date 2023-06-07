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

#include "src/tint/ir/disassembler.h"

#include "src//tint/ir/unary.h"
#include "src/tint/constant/composite.h"
#include "src/tint/constant/scalar.h"
#include "src/tint/constant/splat.h"
#include "src/tint/ir/access.h"
#include "src/tint/ir/binary.h"
#include "src/tint/ir/bitcast.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/break_if.h"
#include "src/tint/ir/builtin.h"
#include "src/tint/ir/construct.h"
#include "src/tint/ir/continue.h"
#include "src/tint/ir/convert.h"
#include "src/tint/ir/discard.h"
#include "src/tint/ir/exit_if.h"
#include "src/tint/ir/exit_loop.h"
#include "src/tint/ir/exit_switch.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/next_iteration.h"
#include "src/tint/ir/return.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/swizzle.h"
#include "src/tint/ir/transform/block_decorated_structs.h"
#include "src/tint/ir/user_call.h"
#include "src/tint/ir/var.h"
#include "src/tint/switch.h"
#include "src/tint/type/struct.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/utils/string.h"

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

Disassembler::Disassembler(const Module& mod) : mod_(mod) {}

Disassembler::~Disassembler() = default;

utils::StringStream& Disassembler::Indent() {
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

void Disassembler::EmitBlockInstructions(const Block* b) {
    for (const auto* inst : *b) {
        Indent();
        EmitInstruction(inst);
    }
}

size_t Disassembler::IdOf(const Block* node) {
    TINT_ASSERT(IR, node);
    return block_ids_.GetOrCreate(node, [&] { return block_ids_.Count(); });
}

std::string_view Disassembler::IdOf(const Value* value) {
    TINT_ASSERT(IR, value);
    return value_ids_.GetOrCreate(value, [&] {
        if (auto sym = mod_.NameOf(value)) {
            return sym.Name();
        }
        return std::to_string(value_ids_.Count());
    });
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
        Indent() << "# Root block";
        EmitLine();
        WalkInternal(mod_.root_block);
        EmitLine();
    }

    for (auto* func : mod_.functions) {
        EmitFunction(func);
    }
    return out_.str();
}

void Disassembler::Walk(const Block* blk) {
    if (visited_.Contains(blk)) {
        return;
    }
    visited_.Add(blk);
    WalkInternal(blk);
}

void Disassembler::WalkInternal(const Block* blk) {
    SourceMarker sm(this);
    Indent() << "%b" << IdOf(blk) << " = block";
    if (!blk->Params().IsEmpty()) {
        out_ << " (";
        EmitValueList(blk->Params().Slice());
        out_ << ")";
    }

    out_ << " {";
    EmitLine();
    {
        ScopedIndent si(indent_size_);
        EmitBlockInstructions(blk);
    }
    Indent() << "}";
    sm.Store(blk);

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
        if (loc.interpolation->sampling != builtin::InterpolationSampling::kUndefined) {
            out_ << ", ";
            out_ << loc.interpolation->sampling;
        }
        out_ << ")";
    }
}

void Disassembler::EmitParamAttributes(const FunctionParam* p) {
    if (!p->Invariant() && !p->Location().has_value() && !p->BindingPoint().has_value() &&
        !p->Builtin().has_value()) {
        return;
    }

    out_ << " [";

    bool need_comma = false;
    auto comma = [&]() {
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

void Disassembler::EmitReturnAttributes(const Function* func) {
    if (!func->ReturnInvariant() && !func->ReturnLocation().has_value() &&
        !func->ReturnBuiltin().has_value()) {
        return;
    }

    out_ << " [";

    bool need_comma = false;
    auto comma = [&]() {
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

void Disassembler::EmitFunction(const Function* func) {
    in_function_ = true;

    Indent() << "%" << IdOf(func) << " =";

    if (func->Stage() != Function::PipelineStage::kUndefined) {
        out_ << " @" << func->Stage();
    }
    if (func->WorkgroupSize()) {
        auto arr = func->WorkgroupSize().value();
        out_ << " @workgroup_size(" << arr[0] << ", " << arr[1] << ", " << arr[2] << ")";
    }

    out_ << " func(";

    for (const auto* p : func->Params()) {
        if (p != func->Params().Front()) {
            out_ << ", ";
        }
        out_ << "%" << IdOf(p) << ":" << p->Type()->FriendlyName();

        EmitParamAttributes(p);
    }
    out_ << "):" << func->ReturnType()->FriendlyName();

    EmitReturnAttributes(func);

    out_ << " -> %b" << IdOf(func->StartTarget()) << " {";
    EmitLine();

    {
        ScopedIndent si(indent_size_);
        Walk(func->StartTarget());
    }
    Indent() << "}";
    EmitLine();
}

void Disassembler::EmitValueWithType(const Value* val) {
    EmitValue(val);
    if (auto* i = val->As<ir::Instruction>(); i->Type() != nullptr) {
        out_ << ":" << i->Type()->FriendlyName();
    }
}

void Disassembler::EmitValue(const Value* val) {
    tint::Switch(
        val,
        [&](const ir::Constant* constant) {
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
        [&](const ir::Instruction* i) { out_ << "%" << IdOf(i); },
        [&](const ir::BlockParam* p) {
            out_ << "%" << IdOf(p) << ":" << p->Type()->FriendlyName();
        },
        [&](const ir::FunctionParam* p) { out_ << "%" << IdOf(p); },
        [&](Default) { out_ << "Unknown value: " << val->TypeInfo().name; });
}

void Disassembler::EmitInstructionName(std::string_view name, const Instruction* inst) {
    SourceMarker sm(this);
    out_ << name;
    sm.Store(inst);
}

void Disassembler::EmitInstruction(const Instruction* inst) {
    tint::Switch(
        inst,                                         //
        [&](const ir::Switch* s) { EmitSwitch(s); },  //
        [&](const ir::If* i) { EmitIf(i); },          //
        [&](const ir::Loop* l) { EmitLoop(l); },      //
        [&](const ir::Binary* b) { EmitBinary(b); },  //
        [&](const ir::Unary* u) { EmitUnary(u); },
        [&](const ir::Bitcast* b) {
            EmitValueWithType(b);
            out_ << " = ";
            EmitInstructionName("bitcast", b);
            out_ << " ";
            EmitArgs(b);
            EmitLine();
        },
        [&](const ir::Discard* d) {
            EmitInstructionName("discard", d);
            EmitLine();
        },
        [&](const ir::Builtin* b) {
            EmitValueWithType(b);
            out_ << " = ";
            EmitInstructionName(builtin::str(b->Func()), b);
            out_ << " ";
            EmitArgs(b);
            EmitLine();
        },
        [&](const ir::Construct* c) {
            EmitValueWithType(c);
            out_ << " = ";
            EmitInstructionName("construct", c);
            out_ << " ";
            EmitArgs(c);
            EmitLine();
        },
        [&](const ir::Convert* c) {
            EmitValueWithType(c);
            out_ << " = ";
            EmitInstructionName("convert", c);
            out_ << " " << c->FromType()->FriendlyName() << ", ";
            EmitArgs(c);
            EmitLine();
        },
        [&](const ir::Load* l) {
            EmitValueWithType(l);
            out_ << " = ";
            EmitInstructionName("load", l);
            out_ << " ";
            EmitValue(l->From());
            EmitLine();
        },
        [&](const ir::Store* s) {
            EmitInstructionName("store", s);
            out_ << " ";
            EmitValue(s->To());
            out_ << ", ";
            EmitValue(s->From());
            EmitLine();
        },
        [&](const ir::UserCall* uc) {
            EmitValueWithType(uc);
            out_ << " = ";
            EmitInstructionName("call", uc);
            out_ << " %" << IdOf(uc->Func());
            if (!uc->Args().IsEmpty()) {
                out_ << ", ";
            }
            EmitArgs(uc);
            EmitLine();
        },
        [&](const ir::Var* v) {
            EmitValueWithType(v);
            out_ << " = ";
            EmitInstructionName("var", v);
            if (v->Initializer()) {
                out_ << ", ";
                EmitValue(v->Initializer());
            }
            if (v->BindingPoint().has_value()) {
                out_ << " ";
                EmitBindingPoint(v->BindingPoint().value());
            }
            EmitLine();
        },
        [&](const ir::Access* a) {
            EmitValueWithType(a);
            out_ << " = ";
            EmitInstructionName("access", a);
            out_ << " ";
            EmitValue(a->Object());
            out_ << ", ";
            for (size_t i = 0; i < a->Indices().Length(); ++i) {
                if (i > 0) {
                    out_ << ", ";
                }
                EmitValue(a->Indices()[i]);
            }
            EmitLine();
        },
        [&](const ir::Swizzle* s) {
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
            EmitLine();
        },
        [&](const ir::Branch* b) { EmitBranch(b); },
        [&](Default) { out_ << "Unknown instruction: " << inst->TypeInfo().name; });
}

void Disassembler::EmitOperand(const Value* val, const Instruction* inst, uint32_t index) {
    SourceMarker condMarker(this);
    EmitValue(val);
    condMarker.Store(Operand{inst, index});
}

void Disassembler::EmitIf(const If* i) {
    SourceMarker sm(this);
    out_ << "if ";
    EmitOperand(i->Condition(), i, If::kConditionOperandIndex);

    bool has_true = i->True()->HasBranchTarget();
    bool has_false = i->False()->HasBranchTarget();

    out_ << " [";
    if (has_true) {
        out_ << "t: %b" << IdOf(i->True());
    }
    if (has_false) {
        if (has_true) {
            out_ << ", ";
        }
        out_ << "f: %b" << IdOf(i->False());
    }
    if (i->Merge()->HasBranchTarget()) {
        out_ << ", m: %b" << IdOf(i->Merge());
    }
    out_ << "]";
    sm.Store(i);

    EmitLine();

    if (has_true) {
        ScopedIndent si(indent_size_);
        Indent() << "# True block";
        EmitLine();

        Walk(i->True());
        EmitLine();
    }
    if (has_false) {
        ScopedIndent si(indent_size_);
        Indent() << "# False block";
        EmitLine();

        Walk(i->False());
        EmitLine();
    }
    if (i->Merge()->HasBranchTarget()) {
        Indent() << "# Merge block";
        EmitLine();
        Walk(i->Merge());
        EmitLine();
    }
}

void Disassembler::EmitLoop(const Loop* l) {
    utils::Vector<std::string, 4> parts;
    if (l->Initializer()->HasBranchTarget()) {
        parts.Push("i: %b" + std::to_string(IdOf(l->Initializer())));
    }
    if (l->Body()->HasBranchTarget()) {
        parts.Push("b: %b" + std::to_string(IdOf(l->Body())));
    }

    if (l->Continuing()->HasBranchTarget()) {
        parts.Push("c: %b" + std::to_string(IdOf(l->Continuing())));
    }
    if (l->Merge()->HasBranchTarget()) {
        parts.Push("m: %b" + std::to_string(IdOf(l->Merge())));
    }
    SourceMarker sm(this);
    out_ << "loop [" << utils::Join(parts, ", ") << "]";
    sm.Store(l);
    EmitLine();

    if (l->Initializer()->HasBranchTarget()) {
        ScopedIndent si(indent_size_);
        Indent() << "# Initializer block";
        EmitLine();
        Walk(l->Initializer());
        EmitLine();
    }

    if (l->Body()->HasBranchTarget()) {
        ScopedIndent si(indent_size_);
        Indent() << "# Body block";
        EmitLine();
        Walk(l->Body());
        EmitLine();
    }

    if (l->Continuing()->HasBranchTarget()) {
        ScopedIndent si(indent_size_);
        Indent() << "# Continuing block";
        EmitLine();
        Walk(l->Continuing());
        EmitLine();
    }
    if (l->Merge()->HasBranchTarget()) {
        Indent() << "# Merge block";
        EmitLine();

        Walk(l->Merge());
        EmitLine();
    }
}

void Disassembler::EmitSwitch(const Switch* s) {
    out_ << "switch ";
    EmitValue(s->Condition());
    out_ << " [";
    for (const auto& c : s->Cases()) {
        if (&c != &s->Cases().Front()) {
            out_ << ", ";
        }
        out_ << "c: (";
        for (const auto& selector : c.selectors) {
            if (&selector != &c.selectors.Front()) {
                out_ << " ";
            }

            if (selector.IsDefault()) {
                out_ << "default";
            } else {
                EmitValue(selector.val);
            }
        }
        out_ << ", %b" << IdOf(c.Start()) << ")";
    }
    if (s->Merge()->HasBranchTarget()) {
        out_ << ", m: %b" << IdOf(s->Merge());
    }
    out_ << "]";
    EmitLine();

    for (auto& c : s->Cases()) {
        ScopedIndent si(indent_size_);
        Indent() << "# Case block";
        EmitLine();

        Walk(c.Start());
        EmitLine();
    }
    if (s->Merge()->HasBranchTarget()) {
        Indent() << "# Merge block";
        EmitLine();

        Walk(s->Merge());
        EmitLine();
    }
}

void Disassembler::EmitBranch(const Branch* b) {
    SourceMarker sm(this);
    tint::Switch(
        b,  //
        [&](const ir::Return*) { out_ << "ret"; },
        [&](const ir::Continue* cont) {
            out_ << "continue %b" << IdOf(cont->Loop()->Continuing());
        },
        [&](const ir::ExitIf* ei) { out_ << "exit_if %b" << IdOf(ei->If()->Merge()); },
        [&](const ir::ExitSwitch* es) { out_ << "exit_switch %b" << IdOf(es->Switch()->Merge()); },
        [&](const ir::ExitLoop* el) { out_ << "exit_loop %b" << IdOf(el->Loop()->Merge()); },
        [&](const ir::NextIteration* ni) {
            out_ << "next_iteration %b" << IdOf(ni->Loop()->Body());
        },
        [&](const ir::BreakIf* bi) {
            out_ << "break_if ";
            EmitValue(bi->Condition());
            out_ << " %b" << IdOf(bi->Loop()->Body());
        },
        [&](Default) { out_ << "Unknown branch " << b->TypeInfo().name; });

    if (!b->Args().IsEmpty()) {
        out_ << " ";
        EmitValueList(b->Args());
    }
    sm.Store(b);

    EmitLine();
}

void Disassembler::EmitValueList(utils::Slice<Value const* const> values) {
    for (auto* v : values) {
        if (v != values.Front()) {
            out_ << ", ";
        }
        EmitValue(v);
    }
}

void Disassembler::EmitArgs(const Call* call) {
    EmitValueList(call->Args());
}

void Disassembler::EmitBinary(const Binary* b) {
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
    EmitValue(b->LHS());
    out_ << ", ";
    EmitValue(b->RHS());
    EmitLine();
}

void Disassembler::EmitUnary(const Unary* u) {
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
    EmitValue(u->Val());
    EmitLine();
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
            if (interp.sampling != builtin::InterpolationSampling::kUndefined) {
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
