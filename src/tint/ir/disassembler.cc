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
#include "src/tint/program.h"

namespace tint::ir {
namespace {

class ScopedStopNode {
  public:
    ScopedStopNode(std::unordered_set<const FlowNode*>* stop_nodes_, const FlowNode* node)
        : stop_nodes__(stop_nodes_), node_(node) {
        stop_nodes__->insert(node_);
    }

    ~ScopedStopNode() { stop_nodes__->erase(node_); }

  private:
    std::unordered_set<const FlowNode*>* stop_nodes__;
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

Disassembler::Disassembler() = default;

Disassembler::~Disassembler() = default;

std::ostream& Disassembler::Indent() {
    for (uint32_t i = 0; i < indent_size_; i++) {
        out_ << " ";
    }
    return out_;
}

void Disassembler::EmitBlockOps(const Block* b) {
    for (const auto& op : b->ops) {
        out_ << op << std::endl;
    }
}

void Disassembler::Walk(const FlowNode* node) {
    if ((visited_.count(node) > 0) || (stop_nodes_.count(node) > 0)) {
        return;
    }
    visited_.insert(node);

    tint::Switch(
        node,
        [&](const ir::Function* f) {
            Indent() << "Function" << std::endl;

            {
                ScopedIndent func_indent(&indent_size_);
                ScopedStopNode scope(&stop_nodes_, f->end_target);
                Walk(f->start_target);
            }
            Walk(f->end_target);
        },
        [&](const ir::Block* b) {
            Indent() << "Block" << std::endl;
            EmitBlockOps(b);
            Walk(b->branch_target);
        },
        [&](const ir::Switch* s) {
            Indent() << "Switch (" << s->condition << ")" << std::endl;

            {
                ScopedIndent switch_indent(&indent_size_);
                ScopedStopNode scope(&stop_nodes_, s->merge_target);
                for (const auto& c : s->cases) {
                    Indent() << "Case" << std::endl;
                    ScopedIndent case_indent(&indent_size_);
                    Walk(c.start_target);
                }
            }

            Indent() << "Switch Merge" << std::endl;
            Walk(s->merge_target);
        },
        [&](const ir::If* i) {
            Indent() << "if (" << i->condition << ")" << std::endl;
            {
                ScopedIndent if_indent(&indent_size_);
                ScopedStopNode scope(&stop_nodes_, i->merge_target);

                Indent() << "true branch" << std::endl;
                Walk(i->true_target);

                Indent() << "false branch" << std::endl;
                Walk(i->false_target);
            }

            Indent() << "if merge" << std::endl;
            Walk(i->merge_target);
        },
        [&](const ir::Loop* l) {
            Indent() << "loop" << std::endl;
            {
                ScopedStopNode loop_scope(&stop_nodes_, l->merge_target);
                ScopedIndent loop_indent(&indent_size_);
                {
                    ScopedStopNode inner_scope(&stop_nodes_, l->continuing_target);
                    Indent() << "loop start" << std::endl;
                    Walk(l->start_target);
                }

                Indent() << "loop continuing" << std::endl;
                ScopedIndent continuing_indent(&indent_size_);
                Walk(l->continuing_target);
            }

            Indent() << "loop merge" << std::endl;
            Walk(l->merge_target);
        },
        [&](const ir::Terminator*) { Indent() << "Function end" << std::endl; });
}

std::string Disassembler::Disassemble(const Module& mod) {
    for (const auto* func : mod.functions) {
        Walk(func);
    }
    return out_.str();
}

}  // namespace tint::ir
