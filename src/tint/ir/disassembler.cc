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

class ScopedStopNode {
    static constexpr size_t N = 32;

  public:
    ScopedStopNode(utils::Hashset<const FlowNode*, N>& stop_nodes, const FlowNode* node)
        : stop_nodes_(stop_nodes), node_(node) {
        stop_nodes_.Add(node_);
    }

    ~ScopedStopNode() { stop_nodes_.Remove(node_); }

  private:
    utils::Hashset<const FlowNode*, N>& stop_nodes_;
    const FlowNode* node_;
};

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

void Disassembler::Walk(const FlowNode* node) {
    if (visited_.Contains(node) || stop_nodes_.Contains(node)) {
        return;
    }
    visited_.Add(node);

    tint::Switch(
        node,
        [&](const ir::Function* f) {
            TINT_SCOPED_ASSIGNMENT(in_function_, true);

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
            out_ << " {" << std::endl;

            {
                ScopedIndent func_indent(indent_size_);
                ScopedStopNode scope(stop_nodes_, f->EndTarget());
                Walk(f->StartTarget());
            }
            out_ << "} ";
            Walk(f->EndTarget());
        },
        [&](const ir::Block* b) {
            // If this block is dead, nothing to do
            if (!b->HasBranchTarget()) {
                return;
            }

            Indent() << "%fn" << IdOf(b) << " = block";
            if (!b->Params().IsEmpty()) {
                out_ << " (";
                for (const auto* p : b->Params()) {
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
            Indent() << "}";

            std::string suffix = "";
            if (b->Branch().target->Is<FunctionTerminator>()) {
                out_ << " -> %func_end";
                suffix = "return";
            } else if (b->Branch().target->Is<RootTerminator>()) {
                // Nothing to do
            } else {
                out_ << " -> "
                     << "%fn" << IdOf(b->Branch().target);
                suffix = "branch";
            }
            if (!b->Branch().args.IsEmpty()) {
                out_ << " ";
                for (const auto* v : b->Branch().args) {
                    if (v != b->Branch().args.Front()) {
                        out_ << ", ";
                    }
                    EmitValue(v);
                }
            }
            if (!suffix.empty()) {
                out_ << " # " << suffix;
            }
            out_ << std::endl;

            if (!b->Branch().target->Is<FunctionTerminator>()) {
                out_ << std::endl;
            }

            Walk(b->Branch().target);
        },
        [&](const ir::Switch* s) {
            Indent() << "%fn" << IdOf(s) << " = switch ";
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
                out_ << ", %fn" << IdOf(c.Start().target) << ")";
            }
            if (s->Merge().target->IsConnected()) {
                out_ << ", m: %fn" << IdOf(s->Merge().target);
            }
            out_ << "]" << std::endl;

            {
                ScopedIndent switch_indent(indent_size_);
                ScopedStopNode scope(stop_nodes_, s->Merge().target);
                for (const auto& c : s->Cases()) {
                    Indent() << "# case ";
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
                    out_ << std::endl;
                    Walk(c.Start().target);
                }
            }

            if (s->Merge().target->IsConnected()) {
                Indent() << "# switch merge" << std::endl;
                Walk(s->Merge().target);
            }
        },
        [&](const ir::If* i) {
            Indent() << "%fn" << IdOf(i) << " = if ";
            EmitValue(i->Condition());

            bool has_true = i->True().target->HasBranchTarget();
            bool has_false = i->False().target->HasBranchTarget();

            out_ << " [";
            if (has_true) {
                out_ << "t: %fn" << IdOf(i->True().target);
            }
            if (has_false) {
                if (has_true) {
                    out_ << ", ";
                }
                out_ << "f: %fn" << IdOf(i->False().target);
            }
            if (i->Merge().target->IsConnected()) {
                out_ << ", m: %fn" << IdOf(i->Merge().target);
            }
            out_ << "]" << std::endl;

            {
                ScopedIndent if_indent(indent_size_);
                ScopedStopNode scope(stop_nodes_, i->Merge().target);

                if (has_true) {
                    Indent() << "# true branch" << std::endl;
                    Walk(i->True().target);
                }

                if (has_false) {
                    Indent() << "# false branch" << std::endl;
                    Walk(i->False().target);
                }
            }

            if (i->Merge().target->IsConnected()) {
                Indent() << "# if merge" << std::endl;
                Walk(i->Merge().target);
            }
        },
        [&](const ir::Loop* l) {
            Indent() << "%fn" << IdOf(l) << " = loop [s: %fn" << IdOf(l->Start().target);

            if (l->Continuing().target->IsConnected()) {
                out_ << ", c: %fn" << IdOf(l->Continuing().target);
            }
            if (l->Merge().target->IsConnected()) {
                out_ << ", m: %fn" << IdOf(l->Merge().target);
            }
            out_ << "]" << std::endl;

            {
                ScopedStopNode loop_scope(stop_nodes_, l->Merge().target);
                ScopedIndent loop_indent(indent_size_);
                {
                    ScopedStopNode inner_scope(stop_nodes_, l->Continuing().target);
                    Indent() << "# loop start" << std::endl;
                    Walk(l->Start().target);
                }

                if (l->Continuing().target->IsConnected()) {
                    Indent() << "# loop continuing" << std::endl;
                    Walk(l->Continuing().target);
                }
            }

            if (l->Merge().target->IsConnected()) {
                Indent() << "# loop merge" << std::endl;
                Walk(l->Merge().target);
            }
        },
        [&](const ir::FunctionTerminator*) {
            TINT_ASSERT(IR, in_function_);
            Indent() << "%func_end" << std::endl << std::endl;
        },
        [&](const ir::RootTerminator*) {
            TINT_ASSERT(IR, !in_function_);
            out_ << std::endl;
        });
}

std::string Disassembler::Disassemble() {
    if (mod_.root_block) {
        Walk(mod_.root_block);
    }

    for (const auto* func : mod_.functions) {
        Walk(func);
    }
    return out_.str();
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
        inst,  //
        [&](const ir::Binary* b) { EmitBinary(b); }, [&](const ir::Unary* u) { EmitUnary(u); },
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
        });
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
