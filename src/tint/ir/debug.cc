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

#include "src/tint/ir/debug.h"

#include <unordered_map>
#include <unordered_set>

#include "src/tint/ir/block.h"
#include "src/tint/ir/function_terminator.h"
#include "src/tint/ir/if.h"
#include "src/tint/ir/loop.h"
#include "src/tint/ir/switch.h"
#include "src/tint/switch.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {

// static
std::string Debug::AsDotGraph(const Module* mod) {
    size_t block_count = 0;

    std::unordered_set<const Block*> visited;
    std::unordered_set<const Block*> merge_blocks;
    std::unordered_map<const Block*, std::string> block_to_name;
    utils::StringStream out;

    auto name_for = [&](const Block* blk) -> std::string {
        if (block_to_name.count(blk) > 0) {
            return block_to_name[blk];
        }

        std::string name = "blk_" + std::to_string(block_count);
        block_count += 1;

        block_to_name[blk] = name;
        return name;
    };

    std::function<void(const Block*)> Graph = [&](const Block* blk) {
        if (visited.count(blk) > 0) {
            return;
        }
        visited.insert(blk);

        tint::Switch(
            blk,
            [&](const ir::FunctionTerminator*) {
                // Already done
            },
            [&](const ir::Block* b) {
                if (block_to_name.count(b) == 0) {
                    out << name_for(b) << R"( [label="block"])" << std::endl;
                }
                out << name_for(b) << " -> " << name_for(b->Branch()->To());

                // Dashed lines to merge blocks
                if (merge_blocks.count(b->Branch()->To()) != 0) {
                    out << " [style=dashed]";
                }

                out << std::endl;
                Graph(b->Branch()->To());
            });
    };

    out << "digraph G {" << std::endl;
    for (const auto* func : mod->functions) {
        // Cluster each function to label and draw a box around it.
        out << "subgraph cluster_" << func->Name().Name() << " {" << std::endl;
        out << R"(label=")" << func->Name().Name() << R"(")" << std::endl;
        out << name_for(func->StartTarget()) << R"( [label="start"])" << std::endl;
        out << name_for(func->EndTarget()) << R"( [label="end"])" << std::endl;
        Graph(func->StartTarget());
        out << "}" << std::endl;
    }
    out << "}";
    return out.str();
}

}  // namespace tint::ir
