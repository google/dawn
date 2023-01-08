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

#include "src/tint/ir/block.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/switch.h"
#include "src/tint/ir/terminator.h"

namespace tint::ir {
namespace {

class ScopedStopNode {
  public:
    ScopedStopNode(std::unordered_set<const FlowNode*>* stop_nodes, const FlowNode* node)
        : stop_nodes_(stop_nodes), node_(node) {
        stop_nodes_->insert(node_);
    }

    ~ScopedStopNode() { stop_nodes_->erase(node_); }

  private:
    std::unordered_set<const FlowNode*>* stop_nodes_;
    const FlowNode* node_;
};

class ScopedIndent {
  public:
    explicit ScopedIndent(uint32_t* indent) : indent_(indent) { (*indent_) += 2; }

    ~ScopedIndent() { (*indent_) -= 2; }

  private:
    uint32_t* indent_;
};

}  // namespace

Disassembler::Disassembler(const Module& mod) : mod_(mod) {}

Disassembler::~Disassembler() = default;

std::ostream& Disassembler::Indent() {
    for (uint32_t i = 0; i < indent_size_; i++) {
        out_ << " ";
    }
    return out_;
}

void Disassembler::EmitBlockInstructions(const Block* b) {
    for (const auto* instr : b->instructions) {
        Indent();
        instr->ToString(out_, mod_.symbols) << std::endl;
    }
}

size_t Disassembler::GetIdForNode(const FlowNode* node) {
    TINT_ASSERT(IR, node);

    auto it = flow_node_to_id_.find(node);
    if (it != flow_node_to_id_.end()) {
        return it->second;
    }
    size_t id = next_node_id_++;
    flow_node_to_id_[node] = id;
    return id;
}

void Disassembler::Walk(const FlowNode* node) {
    if ((visited_.count(node) > 0) || (stop_nodes_.count(node) > 0)) {
        return;
    }
    visited_.insert(node);

    tint::Switch(
        node,
        [&](const ir::Function* f) {
            Indent() << "%bb" << GetIdForNode(f) << " = Function " << mod_.symbols.NameFor(f->name)
                     << std::endl;

            {
                ScopedIndent func_indent(&indent_size_);
                ScopedStopNode scope(&stop_nodes_, f->end_target);
                Walk(f->start_target);
            }
            Walk(f->end_target);
        },
        [&](const ir::Block* b) {
            // If this block is dead, nothing to do
            if (b->IsDead()) {
                Indent() << "# Dead" << std::endl;
                return;
            }

            Indent() << "%bb" << GetIdForNode(b) << " = Block" << std::endl;
            EmitBlockInstructions(b);

            if (b->branch.target->Is<Terminator>()) {
                Indent() << "Return";
            } else {
                Indent() << "BranchTo "
                         << "%bb" << GetIdForNode(b->branch.target);
            }
            out_ << " (";
            for (const auto* v : b->branch.args) {
                if (v != b->branch.args.Front()) {
                    out_ << ", ";
                }
                v->ToString(out_, mod_.symbols);
            }
            out_ << ")" << std::endl;

            if (!b->branch.target->Is<Terminator>()) {
                out_ << std::endl;
            }

            Walk(b->branch.target);
        },
        [&](const ir::Switch* s) {
            Indent() << "%bb" << GetIdForNode(s) << " = Switch (";
            s->condition->ToString(out_, mod_.symbols);
            out_ << ")" << std::endl;

            {
                ScopedIndent switch_indent(&indent_size_);
                ScopedStopNode scope(&stop_nodes_, s->merge.target);
                for (const auto& c : s->cases) {
                    Indent() << "# Case ";
                    for (const auto& selector : c.selectors) {
                        if (&selector != &c.selectors.Front()) {
                            out_ << " ";
                        }

                        if (selector.IsDefault()) {
                            out_ << "default";
                        } else {
                            selector.val->ToString(out_, mod_.symbols);
                        }
                    }
                    out_ << std::endl;
                    Walk(c.start.target);
                }
            }

            Indent() << "# Switch Merge" << std::endl;
            Walk(s->merge.target);
        },
        [&](const ir::If* i) {
            Indent() << "%bb" << GetIdForNode(i) << " = if (";
            i->condition->ToString(out_, mod_.symbols);
            out_ << ")" << std::endl;

            {
                ScopedIndent if_indent(&indent_size_);
                ScopedStopNode scope(&stop_nodes_, i->merge.target);

                Indent() << "# true branch" << std::endl;
                Walk(i->true_.target);

                Indent() << "# false branch" << std::endl;
                Walk(i->false_.target);
            }

            if (!i->merge.target->IsDisconnected()) {
                Indent() << "# if merge" << std::endl;
                Walk(i->merge.target);
            }
        },
        [&](const ir::Loop* l) {
            Indent() << "%bb" << GetIdForNode(l) << " = loop" << std::endl;
            {
                ScopedStopNode loop_scope(&stop_nodes_, l->merge.target);
                ScopedIndent loop_indent(&indent_size_);
                {
                    ScopedStopNode inner_scope(&stop_nodes_, l->continuing.target);
                    Indent() << "# loop start" << std::endl;
                    Walk(l->start.target);
                }

                Indent() << "# loop continuing" << std::endl;
                Walk(l->continuing.target);
            }

            Indent() << "# loop merge" << std::endl;
            Walk(l->merge.target);
        },
        [&](const ir::Terminator*) { Indent() << "FunctionEnd" << std::endl
                                              << std::endl; });
}

std::string Disassembler::Disassemble() {
    for (const auto* func : mod_.functions) {
        Walk(func);
    }
    return out_.str();
}

}  // namespace tint::ir
