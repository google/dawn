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
#include "src/tint/ir/binary.h"
#include "src/tint/ir/bitcast.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/builtin.h"
#include "src/tint/ir/construct.h"
#include "src/tint/ir/convert.h"
#include "src/tint/ir/discard.h"
#include "src/tint/ir/function_terminator.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/jump.h"
#include "src/tint/ir/load.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/root_terminator.h"
#include "src/tint/ir/store.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/user_call.h"
#include "src/tint/ir/var.h"
#include "src/tint/switch.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/scoped_assignment.h"

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

void Disassembler::EmitBlockInstructions(const Block* b) {
    for (const auto* inst : b->Instructions()) {
        Indent();
        EmitInstruction(inst);
        out_ << std::endl;
    }
}

size_t Disassembler::IdOf(const FlowNode* node) {
    TINT_ASSERT(IR, node);
    return flow_node_ids_.GetOrCreate(node, [&] { return flow_node_ids_.Count(); });
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

std::string Disassembler::Disassemble() {
    if (mod_.root_block) {
        walk_list_.push_back(mod_.root_block);
        Walk();
        TINT_ASSERT(IR, walk_list_.empty());
    }

    for (auto* func : mod_.functions) {
        walk_list_.push_back(func);
        Walk();
        TINT_ASSERT(IR, walk_list_.empty());
    }
    return out_.str();
}

void Disassembler::Walk() {
    utils::Hashset<const FlowNode*, 32> visited_;

    while (!walk_list_.empty()) {
        const FlowNode* node = walk_list_.front();
        walk_list_.pop_front();

        if (visited_.Contains(node)) {
            continue;
        }
        visited_.Add(node);

        tint::Switch(
            node,
            [&](const ir::Function* f) {
                in_function_ = true;

                Indent() << "%fn" << IdOf(f) << " = func " << f->Name().Name() << "(";
                for (auto* p : f->Params()) {
                    if (p != f->Params().Front()) {
                        out_ << ", ";
                    }
                    out_ << "%" << IdOf(p) << ":" << p->Type()->FriendlyName();
                }
                out_ << "):" << f->ReturnType()->FriendlyName();

                if (f->Stage() != Function::PipelineStage::kUndefined) {
                    out_ << " [@" << f->Stage();

                    if (f->WorkgroupSize()) {
                        auto arr = f->WorkgroupSize().value();
                        out_ << " @workgroup_size(" << arr[0] << ", " << arr[1] << ", " << arr[2]
                             << ")";
                    }

                    if (!f->ReturnAttributes().IsEmpty()) {
                        out_ << " ra:";

                        for (auto attr : f->ReturnAttributes()) {
                            out_ << " @" << attr;
                            if (attr == Function::ReturnAttribute::kLocation) {
                                out_ << "(" << f->ReturnLocation().value() << ")";
                            }
                        }
                    }

                    out_ << "]";
                }
                out_ << " -> %fn" << IdOf(f->StartTarget()) << std::endl;
                walk_list_.push_back(f->StartTarget());
            },
            [&](const ir::Block* b) {
                // If this block is dead, nothing to do
                if (!b->HasBranchTarget()) {
                    return;
                }

                Indent() << "%fn" << IdOf(b) << " = block";
                if (!b->Params().IsEmpty()) {
                    out_ << " (";
                    for (auto* p : b->Params()) {
                        if (p != b->Params().Front()) {
                            out_ << ", ";
                        }
                        EmitValue(p);
                    }
                    out_ << ")";
                }

                out_ << " {" << std::endl;
                {
                    ScopedIndent si(indent_size_);
                    EmitBlockInstructions(b);
                }
                Indent() << "}" << std::endl;

                if (!b->Branch()->To()->Is<FunctionTerminator>()) {
                    out_ << std::endl;
                }

                walk_list_.push_back(b->Branch()->To());
            },
            [&](const ir::FunctionTerminator* t) {
                TINT_ASSERT(IR, in_function_);
                Indent() << "%fn" << IdOf(t) << " = func_terminator" << std::endl << std::endl;
                in_function_ = false;
            },
            [&](const ir::RootTerminator* t) {
                TINT_ASSERT(IR, !in_function_);
                Indent() << "%fn" << IdOf(t) << " = root_terminator" << std::endl << std::endl;
            });
    }
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
                        out_ << splat->Type()->FriendlyName() << " ";
                        emit(splat->Index(0));
                    },
                    [&](const constant::Composite* composite) {
                        out_ << composite->Type()->FriendlyName() << " ";
                        for (const auto* elem : composite->elements) {
                            if (elem != composite->elements[0]) {
                                out_ << ", ";
                            }
                            emit(elem);
                        }
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
            out_ << " = bitcast ";
            EmitArgs(b);
        },
        [&](const ir::Discard*) { out_ << "discard"; },
        [&](const ir::Builtin* b) {
            EmitValueWithType(b);
            out_ << " = " << builtin::str(b->Func()) << " ";
            EmitArgs(b);
        },
        [&](const ir::Construct* c) {
            EmitValueWithType(c);
            out_ << " = construct ";
            EmitArgs(c);
        },
        [&](const ir::Convert* c) {
            EmitValueWithType(c);
            out_ << " = convert " << c->FromType()->FriendlyName() << ", ";
            EmitArgs(c);
        },
        [&](const ir::Load* l) {
            EmitValueWithType(l);
            out_ << " = load ";
            EmitValue(l->From());
        },
        [&](const ir::Store* s) {
            out_ << "store ";
            EmitValue(s->To());
            out_ << ", ";
            EmitValue(s->From());
        },
        [&](const ir::UserCall* uc) {
            EmitValueWithType(uc);
            out_ << " = call " << uc->Name().Name();
            if (!uc->Args().IsEmpty()) {
                out_ << ", ";
            }
            EmitArgs(uc);
        },
        [&](const ir::Var* v) {
            EmitValueWithType(v);
            out_ << " = var";
            if (v->Initializer()) {
                out_ << ", ";
                EmitValue(v->Initializer());
            }
        },
        [&](const ir::Branch* b) { EmitBranch(b); },
        [&](Default) { out_ << "Unknown instruction: " << inst->TypeInfo().name; });
}

void Disassembler::EmitIf(const If* i) {
    out_ << "if ";
    EmitValue(i->Condition());

    bool has_true = i->True()->HasBranchTarget();
    bool has_false = i->False()->HasBranchTarget();

    out_ << " [";
    if (has_true) {
        out_ << "t: %fn" << IdOf(i->True());
    }
    if (has_false) {
        if (has_true) {
            out_ << ", ";
        }
        out_ << "f: %fn" << IdOf(i->False());
    }
    if (i->Merge()->IsConnected()) {
        out_ << ", m: %fn" << IdOf(i->Merge());
    }
    out_ << "]";

    if (has_true) {
        walk_list_.push_back(i->True());
    }
    if (has_false) {
        walk_list_.push_back(i->False());
    }
    if (i->Merge()->IsConnected()) {
        walk_list_.push_back(i->Merge());
    }
}

void Disassembler::EmitLoop(const Loop* l) {
    out_ << "loop [s: %fn" << IdOf(l->Start());

    if (l->Continuing()->IsConnected()) {
        out_ << ", c: %fn" << IdOf(l->Continuing());
    }
    if (l->Merge()->IsConnected()) {
        out_ << ", m: %fn" << IdOf(l->Merge());
    }
    out_ << "]";

    { walk_list_.push_back(l->Start()); }

    if (l->Continuing()->IsConnected()) {
        walk_list_.push_back(l->Continuing());
    }
    if (l->Merge()->IsConnected()) {
        walk_list_.push_back(l->Merge());
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
        out_ << ", %fn" << IdOf(c.Start()) << ")";
    }
    if (s->Merge()->IsConnected()) {
        out_ << ", m: %fn" << IdOf(s->Merge());
    }
    out_ << "]";

    for (auto& c : s->Cases()) {
        walk_list_.push_back(c.Start());
    }
    if (s->Merge()->IsConnected()) {
        walk_list_.push_back(s->Merge());
    }
}

void Disassembler::EmitBranch(const Branch* b) {
    if (b->Is<Jump>()) {
        out_ << "jmp ";

        // Stuff the thing we're jumping too into the front of the walk list so it will be emitted
        // next.
        walk_list_.push_front(b->To());
    } else {
        out_ << "br ";
    }

    std::string suffix = "";
    out_ << "%fn" << IdOf(b->To());
    if (b->To()->Is<FunctionTerminator>()) {
        suffix = "return";
    } else if (b->To()->Is<RootTerminator>()) {
        suffix = "root_end";
    }

    if (!b->Args().IsEmpty()) {
        out_ << " ";
        for (auto* v : b->Args()) {
            if (v != b->Args().Front()) {
                out_ << ", ";
            }
            EmitValue(v);
        }
    }
    if (!suffix.empty()) {
        out_ << "  # " << suffix;
    }
}

void Disassembler::EmitArgs(const Call* call) {
    bool first = true;
    for (const auto* arg : call->Args()) {
        if (!first) {
            out_ << ", ";
        }
        first = false;
        EmitValue(arg);
    }
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
}

}  // namespace tint::ir
