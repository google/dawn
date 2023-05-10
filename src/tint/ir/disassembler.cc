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
    for (const auto* inst : b->instructions) {
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

            Indent() << "%fn" << IdOf(f) << " = func " << f->name.Name()
                     << "():" << f->return_type->FriendlyName();

            if (f->pipeline_stage != Function::PipelineStage::kUndefined) {
                out_ << " [@" << f->pipeline_stage;

                if (f->workgroup_size) {
                    auto arr = f->workgroup_size.value();
                    out_ << " @workgroup_size(" << arr[0] << ", " << arr[1] << ", " << arr[2]
                         << ")";
                }

                if (!f->return_attributes.IsEmpty()) {
                    out_ << " ra:";

                    for (auto attr : f->return_attributes) {
                        out_ << " @" << attr;
                        if (attr == Function::ReturnAttribute::kLocation) {
                            out_ << "(" << f->return_location.value() << ")";
                        }
                    }
                }

                out_ << "]";
            }
            out_ << std::endl;

            {
                ScopedIndent func_indent(indent_size_);
                ScopedStopNode scope(stop_nodes_, f->end_target);
                Walk(f->start_target);
            }
            Walk(f->end_target);
        },
        [&](const ir::Block* b) {
            // If this block is dead, nothing to do
            if (b->IsDead()) {
                return;
            }

            Indent() << "%fn" << IdOf(b) << " = block" << std::endl;
            EmitBlockInstructions(b);

            if (b->branch.target->Is<FunctionTerminator>()) {
                Indent() << "ret";
            } else if (b->branch.target->Is<RootTerminator>()) {
                // Nothing to do
            } else {
                Indent() << "branch "
                         << "%fn" << IdOf(b->branch.target);
            }
            if (!b->branch.args.IsEmpty()) {
                out_ << " ";
                for (const auto* v : b->branch.args) {
                    if (v != b->branch.args.Front()) {
                        out_ << ", ";
                    }
                    EmitValue(v);
                }
            }
            out_ << std::endl;

            if (!b->branch.target->Is<FunctionTerminator>()) {
                out_ << std::endl;
            }

            Walk(b->branch.target);
        },
        [&](const ir::Switch* s) {
            Indent() << "%fn" << IdOf(s) << " = switch ";
            EmitValue(s->condition);
            out_ << " [";
            for (const auto& c : s->cases) {
                if (&c != &s->cases.Front()) {
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
                out_ << ", %fn" << IdOf(c.start.target) << ")";
            }
            if (s->merge.target->IsConnected()) {
                out_ << ", m: %fn" << IdOf(s->merge.target);
            }
            out_ << "]" << std::endl;

            {
                ScopedIndent switch_indent(indent_size_);
                ScopedStopNode scope(stop_nodes_, s->merge.target);
                for (const auto& c : s->cases) {
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
                    Walk(c.start.target);
                }
            }

            if (s->merge.target->IsConnected()) {
                Indent() << "# switch merge" << std::endl;
                Walk(s->merge.target);
            }
        },
        [&](const ir::If* i) {
            Indent() << "%fn" << IdOf(i) << " = if ";
            EmitValue(i->condition);
            out_ << " [t: %fn" << IdOf(i->true_.target) << ", f: %fn" << IdOf(i->false_.target);
            if (i->merge.target->IsConnected()) {
                out_ << ", m: %fn" << IdOf(i->merge.target);
            }
            out_ << "]" << std::endl;

            {
                ScopedIndent if_indent(indent_size_);
                ScopedStopNode scope(stop_nodes_, i->merge.target);

                Indent() << "# true branch" << std::endl;
                Walk(i->true_.target);

                if (!i->false_.target->IsDead()) {
                    Indent() << "# false branch" << std::endl;
                    Walk(i->false_.target);
                }
            }

            if (i->merge.target->IsConnected()) {
                Indent() << "# if merge" << std::endl;
                Walk(i->merge.target);
            }
        },
        [&](const ir::Loop* l) {
            Indent() << "%fn" << IdOf(l) << " = loop [s: %fn" << IdOf(l->start.target);

            if (l->continuing.target->IsConnected()) {
                out_ << ", c: %fn" << IdOf(l->continuing.target);
            }
            if (l->merge.target->IsConnected()) {
                out_ << ", m: %fn" << IdOf(l->merge.target);
            }
            out_ << "]" << std::endl;

            {
                ScopedStopNode loop_scope(stop_nodes_, l->merge.target);
                ScopedIndent loop_indent(indent_size_);
                {
                    ScopedStopNode inner_scope(stop_nodes_, l->continuing.target);
                    Indent() << "# loop start" << std::endl;
                    Walk(l->start.target);
                }

                if (l->continuing.target->IsConnected()) {
                    Indent() << "# loop continuing" << std::endl;
                    Walk(l->continuing.target);
                }
            }

            if (l->merge.target->IsConnected()) {
                Indent() << "# loop merge" << std::endl;
                Walk(l->merge.target);
            }
        },
        [&](const ir::FunctionTerminator*) {
            TINT_ASSERT(IR, in_function_);
            Indent() << "func_end" << std::endl << std::endl;
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
            emit(constant->value);
        },
        [&](const ir::Instruction* i) {
            out_ << "%" << IdOf(i);
            if (i->Type() != nullptr) {
                out_ << ":" << i->Type()->FriendlyName();
            }
        });
}

void Disassembler::EmitInstruction(const Instruction* inst) {
    tint::Switch(
        inst,  //
        [&](const ir::Binary* b) { EmitBinary(b); }, [&](const ir::Unary* u) { EmitUnary(u); },
        [&](const ir::Bitcast* b) {
            EmitValue(b);
            out_ << " = bitcast ";
            EmitArgs(b);
        },
        [&](const ir::Discard*) { out_ << "discard"; },
        [&](const ir::Builtin* b) {
            EmitValue(b);
            out_ << " = " << builtin::str(b->Func()) << " ";
            EmitArgs(b);
        },
        [&](const ir::Construct* c) {
            EmitValue(c);
            out_ << " = construct ";
            EmitArgs(c);
        },
        [&](const ir::Convert* c) {
            EmitValue(c);
            out_ << " = convert " << c->FromType()->FriendlyName() << ", ";
            EmitArgs(c);
        },
        [&](const ir::Store* s) {
            out_ << "store ";
            EmitValue(s->to);
            out_ << ", ";
            EmitValue(s->from);
        },
        [&](const ir::UserCall* uc) {
            EmitValue(uc);
            out_ << " = call " << uc->name.Name();
            if (uc->args.Length() > 0) {
                out_ << ", ";
            }
            EmitArgs(uc);
        },
        [&](const ir::Var* v) {
            EmitValue(v);
            out_ << " = var " << v->address_space << ", " << v->access;
            if (v->initializer) {
                out_ << ", ";
                EmitValue(v->initializer);
            }
        });
}

void Disassembler::EmitArgs(const Call* call) {
    bool first = true;
    for (const auto* arg : call->args) {
        if (!first) {
            out_ << ", ";
        }
        first = false;
        EmitValue(arg);
    }
}

void Disassembler::EmitBinary(const Binary* b) {
    EmitValue(b);
    out_ << " = ";
    switch (b->kind) {
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
    EmitValue(u);
    out_ << " = ";
    switch (u->kind) {
        case Unary::Kind::kAddressOf:
            out_ << "addr_of";
            break;
        case Unary::Kind::kComplement:
            out_ << "complement";
            break;
        case Unary::Kind::kIndirection:
            out_ << "indirection";
            break;
        case Unary::Kind::kNegation:
            out_ << "negation";
            break;
    }
    out_ << " ";
    EmitValue(u->Val());
}

}  // namespace tint::ir
