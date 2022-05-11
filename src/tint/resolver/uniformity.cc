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

#include "src/tint/resolver/uniformity.h"

#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/resolver/dependency_graph.h"
#include "src/tint/scope_stack.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/if_statement.h"
#include "src/tint/sem/info.h"
#include "src/tint/sem/loop_statement.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/sem/type_conversion.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/block_allocator.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/unique_vector.h"

// Set to `1` to dump the uniformity graph for each function in graphviz format.
#define TINT_DUMP_UNIFORMITY_GRAPH 0

namespace tint::resolver {

namespace {

/// CallSiteTag describes the uniformity requirements on the call sites of a function.
enum CallSiteTag {
    CallSiteRequiredToBeUniform,
    CallSiteNoRestriction,
};

/// FunctionTag describes a functions effects on uniformity.
enum FunctionTag {
    SubsequentControlFlowMayBeNonUniform,
    ReturnValueMayBeNonUniform,
    NoRestriction,
};

/// ParameterTag describes the uniformity requirements of values passed to a function parameter.
enum ParameterTag {
    ParameterRequiredToBeUniform,
    ParameterRequiredToBeUniformForSubsequentControlFlow,
    ParameterRequiredToBeUniformForReturnValue,
    ParameterNoRestriction,
};

/// ParameterInfo holds information about the uniformity requirements and effects for a particular
/// function parameter.
struct ParameterInfo {
    /// The parameter's uniformity requirements.
    ParameterTag tag = ParameterNoRestriction;
    /// Will be `true` if this function may cause the contents of this pointer parameter to become
    /// non-uniform.
    bool pointer_may_become_non_uniform = false;
    /// The parameters that are required to be uniform for the contents of this pointer parameter to
    /// be uniform at function exit.
    std::vector<const sem::Parameter*> pointer_param_output_sources;
};

/// FunctionInfo holds information about the uniformity requirements and effects for a particular
/// function.
struct FunctionInfo {
    /// The call site uniformity requirements.
    CallSiteTag callsite_tag;
    /// The function's uniformity effects.
    FunctionTag function_tag;
    /// The uniformity requirements of the function's parameters.
    std::vector<ParameterInfo> parameters;
};

/// Node represents a node in the graph of control flow and value nodes within the analysis of a
/// single function.
struct Node {
    /// Constructor
    /// @param t the node tag (used for debugging)
    Node([[maybe_unused]] std::string t, const ast::Node* a)
        :
#if TINT_DUMP_UNIFORMITY_GRAPH
          tag(t),
#endif
          ast(a) {
    }

#if TINT_DUMP_UNIFORMITY_GRAPH
    /// The node tag.
    const std::string tag;
#endif

    /// The corresponding AST node, or nullptr.
    const ast::Node* ast = nullptr;

    /// The function call argument index, or UINT32_MAX.
    uint32_t arg_index = std::numeric_limits<uint32_t>::max();

    /// The set of edges from this node to other nodes in the graph.
    utils::UniqueVector<Node*> edges;

    /// The node that this node was visited from, or nullptr if not visited.
    Node* visited_from = nullptr;

    /// Add a edge to the `to` node.
    /// @param to the destination node
    void AddEdge(Node* to) { edges.add(to); }
};

/// UniformityGraph is used to analyze the uniformity requirements and effects of functions in a
/// module.
class UniformityGraph {
  public:
    /// Constructor.
    /// @param builder the program to analyze
    explicit UniformityGraph(ProgramBuilder* builder)
        : builder_(builder), sem_(builder->Sem()), diagnostics_(builder->Diagnostics()) {}

    /// Destructor.
    ~UniformityGraph() {}

    /// Build and analyze the graph to determine whether the program satisfies the uniformity
    /// constraints of WGSL.
    /// @param dependency_graph the dependency-ordered module-scope declarations
    /// @returns true if all uniformity constraints are satisfied, otherise false
    bool Build(const DependencyGraph& dependency_graph) {
#if TINT_DUMP_UNIFORMITY_GRAPH
        std::cout << "digraph G {\n";
        std::cout << "rankdir=BT\n";
#endif

        // Process all functions in the module.
        bool success = true;
        for (auto* decl : dependency_graph.ordered_globals) {
            if (auto* func = decl->As<ast::Function>()) {
                if (!ProcessFunction(func)) {
                    success = false;
                    break;
                }
            }
        }

#if TINT_DUMP_UNIFORMITY_GRAPH
        std::cout << "\n}\n";
#endif

        return success;
    }

  private:
    const ProgramBuilder* builder_;
    const sem::Info& sem_;
    diag::List& diagnostics_;

    /// Map of analyzed function results.
    std::unordered_map<const ast::Function*, FunctionInfo> functions_;

    /// Allocator for nodes.
    utils::BlockAllocator<Node> nodes_;

    /// Name of the function currently being analyzed.
    std::string current_function_;

    /// Special `RequiredToBeUniform` node.
    Node* required_to_be_uniform_;
    /// Special `MayBeNonUniform` node.
    Node* may_be_non_uniform_;
    /// Special `CF_return` node.
    Node* cf_return_;
    /// Special `Value_return` node.
    Node* value_return_;
    /// Special `{param}_return` nodes for pointer parameters.
    std::unordered_map<const sem::Parameter*, Node*> pointer_param_returns_;

    /// Map from variables to their value nodes in the graph, scoped with respect to control flow.
    ScopeStack<const sem::Variable*, Node*> variables_;

    /// The set of a local read-write vars that are in scope at any given point in the process.
    /// Includes pointer parameters.
    std::unordered_set<const sem::Variable*> local_var_decls_;

    /// LoopSwitchInfo tracks information about the value of variables for a control flow construct.
    struct LoopSwitchInfo {
        /// The type of this control flow construct.
        std::string type;
        /// The input values for local variables at the start of this construct.
        std::unordered_map<const sem::Variable*, Node*> var_in_nodes;
        /// The exit values for local variables at the end of this construct.
        std::unordered_map<const sem::Variable*, Node*> var_exit_nodes;
    };

    /// Map from control flow statements to the corresponding LoopSwitchInfo structure.
    std::unordered_map<const sem::Statement*, LoopSwitchInfo> loop_switch_infos_;

    /// A list of tags that have already been used within the current function.
    std::unordered_set<std::string> tags_;

    /// Create a new node.
    /// @param tag a tag used to identify the node for debugging purposes.
    /// @param ast the optional AST node that this node corresponds to
    /// @returns the new node
    Node* CreateNode(std::string tag, const ast::Node* ast = nullptr) {
        std::string unique_tag = tag;

#if TINT_DUMP_UNIFORMITY_GRAPH
        // Make the tag unique.
        // This only matters if we're dumping the graph.
        int suffix = 0;
        while (tags_.count(unique_tag)) {
            unique_tag = tag + "_$" + std::to_string(++suffix);
        }
        tags_.insert(unique_tag);
#endif

        return nodes_.Create(current_function_ + "." + unique_tag, ast);
    }

    /// Process a function.
    /// @param func the function to process
    /// @returns true if there are no uniformity issues, false otherwise
    bool ProcessFunction(const ast::Function* func) {
        nodes_.Reset();
        variables_.Clear();
        pointer_param_returns_.clear();
        tags_.clear();

        current_function_ = builder_->Symbols().NameFor(func->symbol);

        // Create special nodes.
        Node* cf_start = CreateNode("CF_start");
        required_to_be_uniform_ = CreateNode("RequiredToBeUniform");
        may_be_non_uniform_ = CreateNode("MayBeNonUniform");
        cf_return_ = CreateNode("CF_return");
        if (func->return_type) {
            value_return_ = CreateNode("Value_return");
        }

        // Create nodes for parameters.
        std::vector<Node*> param_init_values(func->params.size(), nullptr);
        for (size_t i = 0; i < func->params.size(); i++) {
            auto* param = func->params[i];
            auto name = builder_->Symbols().NameFor(param->symbol);
            auto sem = sem_.Get<sem::Parameter>(param);

            Node* node_init;
            if (sem->Type()->Is<sem::Pointer>()) {
                node_init = CreateNode("ptrparam_" + name + "_init");
                pointer_param_returns_[sem] = CreateNode("ptrparam_" + name + "_return");
                local_var_decls_.insert(sem);
            } else {
                node_init = CreateNode("param_" + name);
            }
            param_init_values[i] = node_init;
            variables_.Set(sem, node_init);
        }

        // Process function body.
        if (func->body) {
            ProcessStatement(cf_start, func->body);
        }

#if TINT_DUMP_UNIFORMITY_GRAPH
        // Dump the graph for this function as a subgraph.
        std::cout << "\nsubgraph cluster_" << current_function_ << " {\n";
        std::cout << "  label=" << current_function_ << ";";
        for (auto* node : nodes_.Objects()) {
            std::cout << "\n  \"" << node->tag << "\";";
            for (auto* edge : node->edges) {
                std::cout << "\n  \"" << node->tag << "\" -> \"" << edge->tag << "\";";
            }
        }
        std::cout << "\n}\n";
#endif

        FunctionInfo& info = functions_[func];
        info.callsite_tag = CallSiteNoRestriction;
        info.function_tag = NoRestriction;
        info.parameters.resize(func->params.size());

        // Look at which nodes are reachable from "RequiredToBeUniform".
        {
            utils::UniqueVector<Node*> reachable;
            Traverse(required_to_be_uniform_, reachable);
            if (reachable.contains(may_be_non_uniform_)) {
                MakeError();
                return false;
            }
            if (reachable.contains(cf_start)) {
                info.callsite_tag = CallSiteRequiredToBeUniform;
            }

            // Set the parameter tag to ParameterRequiredToBeUniform for each parameter node that
            // was reachable.
            for (size_t i = 0; i < func->params.size(); i++) {
                auto* param = func->params[i];
                if (reachable.contains(variables_.Get(sem_.Get(param)))) {
                    info.parameters[i].tag = ParameterRequiredToBeUniform;
                }
            }
        }

        // Look at which nodes are reachable from "CF_return"
        {
            utils::UniqueVector<Node*> reachable;
            Traverse(cf_return_, reachable);
            if (reachable.contains(may_be_non_uniform_)) {
                info.function_tag = SubsequentControlFlowMayBeNonUniform;
            }

            // Set the parameter tag to ParameterRequiredToBeUniformForSubsequentControlFlow for
            // each parameter node that was reachable.
            for (size_t i = 0; i < func->params.size(); i++) {
                auto* param = func->params[i];
                if (reachable.contains(variables_.Get(sem_.Get(param)))) {
                    info.parameters[i].tag = ParameterRequiredToBeUniformForSubsequentControlFlow;
                }
            }
        }

        // If "Value_return" exists, look at which nodes are reachable from it
        if (value_return_) {
            utils::UniqueVector<Node*> reachable;
            Traverse(value_return_, reachable);
            if (reachable.contains(may_be_non_uniform_)) {
                info.function_tag = ReturnValueMayBeNonUniform;
            }

            // Set the parameter tag to ParameterRequiredToBeUniformForReturnValue for each
            // parameter node that was reachable.
            for (size_t i = 0; i < func->params.size(); i++) {
                auto* param = func->params[i];
                if (reachable.contains(variables_.Get(sem_.Get(param)))) {
                    info.parameters[i].tag = ParameterRequiredToBeUniformForReturnValue;
                }
            }
        }

        // Traverse the graph for each pointer parameter.
        for (size_t i = 0; i < func->params.size(); i++) {
            auto* param_dest = sem_.Get<sem::Parameter>(func->params[i]);
            if (!param_dest->Type()->Is<sem::Pointer>()) {
                continue;
            }

            // Reset "visited" state for all nodes.
            for (auto* node : nodes_.Objects()) {
                node->visited_from = nullptr;
            }

            utils::UniqueVector<Node*> reachable;
            Traverse(pointer_param_returns_[param_dest], reachable);
            if (reachable.contains(may_be_non_uniform_)) {
                info.parameters[i].pointer_may_become_non_uniform = true;
            }

            // Check every other parameter to see if they feed into this parameter's final value.
            for (size_t j = 0; j < func->params.size(); j++) {
                auto* param_source = sem_.Get<sem::Parameter>(func->params[j]);
                if (reachable.contains(param_init_values[j])) {
                    info.parameters[i].pointer_param_output_sources.push_back(param_source);
                }
            }
        }

        return true;
    }

    /// Process a statement, returning the new control flow node.
    /// @param cf the input control flow node
    /// @param stmt the statement to process d
    /// @returns the new control flow node
    Node* ProcessStatement(Node* cf, const ast::Statement* stmt) {
        return Switch(
            stmt,

            [&](const ast::AssignmentStatement* a) {
                auto [cf1, v1] = ProcessExpression(cf, a->rhs);
                if (a->lhs->Is<ast::PhonyExpression>()) {
                    return cf1;
                } else {
                    auto [cf2, l2] = ProcessLValueExpression(cf1, a->lhs);
                    l2->AddEdge(v1);
                    return cf2;
                }
            },

            [&](const ast::BlockStatement* b) {
                std::unordered_map<const sem::Variable*, Node*> scoped_assignments;
                {
                    // Push a new scope for variable assignments in the block.
                    variables_.Push();
                    TINT_DEFER(variables_.Pop());

                    for (auto* s : b->statements) {
                        cf = ProcessStatement(cf, s);
                        if (!sem_.Get(s)->Behaviors().Contains(sem::Behavior::kNext)) {
                            break;
                        }
                    }

                    if (sem_.Get<sem::FunctionBlockStatement>(b)) {
                        // We've reached the end of the function body.
                        // Add edges from pointer parameter outputs to their current value.
                        for (auto param : pointer_param_returns_) {
                            param.second->AddEdge(variables_.Get(param.first));
                        }
                    }

                    scoped_assignments = std::move(variables_.Top());
                }

                // Propagate all variables assignments to the containing scope if the behavior is
                // either 'Next' or 'Fallthrough'.
                auto& behaviors = sem_.Get(b)->Behaviors();
                if (behaviors.Contains(sem::Behavior::kNext) ||
                    behaviors.Contains(sem::Behavior::kFallthrough)) {
                    for (auto var : scoped_assignments) {
                        variables_.Set(var.first, var.second);
                    }
                }

                // Remove any variables declared in this scope from the set of in-scope variables.
                for (auto* d : sem_.Get<sem::BlockStatement>(b)->Decls()) {
                    local_var_decls_.erase(sem_.Get<sem::LocalVariable>(d));
                }

                return cf;
            },

            [&](const ast::BreakStatement* b) {
                // Find the loop or switch statement that we are in.
                auto* parent = sem_.Get(b)
                                   ->FindFirstParent<sem::SwitchStatement, sem::LoopStatement,
                                                     sem::ForLoopStatement>();
                TINT_ASSERT(Resolver, loop_switch_infos_.count(parent));
                auto& info = loop_switch_infos_.at(parent);

                // Propagate variable values to the loop/switch exit nodes.
                for (auto* var : local_var_decls_) {
                    // Skip variables that were declared inside this loop/switch.
                    if (auto* lv = var->As<sem::LocalVariable>();
                        lv &&
                        lv->Statement()->FindFirstParent([&](auto* s) { return s == parent; })) {
                        continue;
                    }

                    // Add an edge from the variable exit node to its value at this point.
                    auto* exit_node = utils::GetOrCreate(info.var_exit_nodes, var, [&]() {
                        auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                        return CreateNode(name + "_value_" + info.type + "_exit");
                    });
                    exit_node->AddEdge(variables_.Get(var));
                }

                return cf;
            },

            [&](const ast::CallStatement* c) {
                auto [cf1, _] = ProcessCall(cf, c->expr);
                return cf1;
            },

            [&](const ast::CompoundAssignmentStatement* c) {
                // The compound assignment statement `a += b` is equivalent to `a = a + b`.
                auto [cf1, v1] = ProcessExpression(cf, c->lhs);
                auto [cf2, v2] = ProcessExpression(cf1, c->rhs);
                auto* result = CreateNode("binary_expr_result");
                result->AddEdge(v1);
                result->AddEdge(v2);

                auto [cf3, l3] = ProcessLValueExpression(cf2, c->lhs);
                l3->AddEdge(result);
                return cf3;
            },

            [&](const ast::ContinueStatement* c) {
                // Find the loop statement that we are in.
                auto* parent =
                    sem_.Get(c)->FindFirstParent<sem::LoopStatement, sem::ForLoopStatement>();
                TINT_ASSERT(Resolver, loop_switch_infos_.count(parent));
                auto& info = loop_switch_infos_.at(parent);

                // Propagate assignments to the loop input nodes.
                for (auto* var : local_var_decls_) {
                    // Skip variables that were declared inside this loop.
                    if (auto* lv = var->As<sem::LocalVariable>();
                        lv &&
                        lv->Statement()->FindFirstParent([&](auto* s) { return s == parent; })) {
                        continue;
                    }

                    // Add an edge from the variable's loop input node to its value at this point.
                    TINT_ASSERT(Resolver, info.var_in_nodes.count(var));
                    auto* in_node = info.var_in_nodes.at(var);
                    auto* out_node = variables_.Get(var);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }
                return cf;
            },

            [&](const ast::DiscardStatement*) {
                cf_return_->AddEdge(cf);
                return cf;
            },

            [&](const ast::FallthroughStatement*) { return cf; },

            [&](const ast::ForLoopStatement* f) {
                auto* sem_loop = sem_.Get(f);
                auto cfx = CreateNode("loop_start");

                // Insert the initializer before the loop.
                auto cf_init = cf;
                if (f->initializer) {
                    cf_init = ProcessStatement(cf, f->initializer);
                }
                auto cf_start = cf_init;

                auto& info = loop_switch_infos_[sem_loop];
                info.type = "forloop";

                // Create input nodes for any variables declared before this loop.
                for (auto* v : local_var_decls_) {
                    auto name = builder_->Symbols().NameFor(v->Declaration()->symbol);
                    auto* in_node = CreateNode(name + "_value_forloop_in");
                    in_node->AddEdge(variables_.Get(v));
                    info.var_in_nodes[v] = in_node;
                    variables_.Set(v, in_node);
                }

                // Insert the condition at the start of the loop body.
                if (f->condition) {
                    auto [cf_cond, v] = ProcessExpression(cfx, f->condition);
                    auto* cf_condition_end = CreateNode("for_condition_CFend");
                    cf_condition_end->AddEdge(v);
                    cf_start = cf_condition_end;

                    // Propagate assignments to the loop exit nodes.
                    for (auto* var : local_var_decls_) {
                        auto* exit_node = utils::GetOrCreate(info.var_exit_nodes, var, [&]() {
                            auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                            return CreateNode(name + "_value_" + info.type + "_exit");
                        });
                        exit_node->AddEdge(variables_.Get(var));
                    }
                }
                auto cf1 = ProcessStatement(cf_start, f->body);

                // Insert the continuing statement at the end of the loop body.
                if (f->continuing) {
                    auto cf2 = ProcessStatement(cf1, f->continuing);
                    cfx->AddEdge(cf2);
                } else {
                    cfx->AddEdge(cf1);
                }
                cfx->AddEdge(cf);

                // Add edges from variable loop input nodes to their values at the end of the loop.
                for (auto v : info.var_in_nodes) {
                    auto* in_node = v.second;
                    auto* out_node = variables_.Get(v.first);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }

                // Set each variable's exit node as its value in the outer scope.
                for (auto v : info.var_exit_nodes) {
                    variables_.Set(v.first, v.second);
                }

                loop_switch_infos_.erase(sem_loop);

                if (sem_loop->Behaviors() == sem::Behaviors{sem::Behavior::kNext}) {
                    return cf;
                } else {
                    return cfx;
                }
            },

            [&](const ast::IfStatement* i) {
                auto* sem_if = sem_.Get(i);
                auto [cfx, v] = ProcessExpression(cf, i->condition);

                std::unordered_map<const sem::Variable*, Node*> true_vars;
                std::unordered_map<const sem::Variable*, Node*> false_vars;

                // Helper to process a statement with a new scope for variable assignments.
                // Populates `assigned_vars` with new nodes for any variables that are assigned in
                // this statement.
                auto process_in_scope =
                    [&](Node* cf_in, const ast::Statement* s,
                        std::unordered_map<const sem::Variable*, Node*>& assigned_vars) {
                        // Push a new scope for variable assignments.
                        variables_.Push();

                        // Process the statement.
                        auto cf_out = ProcessStatement(cf_in, s);

                        assigned_vars = variables_.Top();

                        // Pop the scope and return.
                        variables_.Pop();
                        return cf_out;
                    };

                auto cf1 = process_in_scope(v, i->body, true_vars);

                bool true_has_next = sem_.Get(i->body)->Behaviors().Contains(sem::Behavior::kNext);
                bool false_has_next = true;

                Node* cf2 = nullptr;
                if (i->else_statement) {
                    cf2 = process_in_scope(v, i->else_statement, false_vars);

                    false_has_next =
                        sem_.Get(i->else_statement)->Behaviors().Contains(sem::Behavior::kNext);
                }

                // Update values for any variables assigned in the if or else blocks.
                for (auto var : local_var_decls_) {
                    // Skip variables not assigned in either block.
                    if (true_vars.count(var) == 0 && false_vars.count(var) == 0) {
                        continue;
                    }

                    // Create an exit node for the variable.
                    auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                    auto* out_node = CreateNode(name + "_value_if_exit");

                    // Add edges to the assigned value or the initial value.
                    // Only add edges if the behavior for that block contains 'Next'.
                    if (true_has_next) {
                        if (true_vars.count(var)) {
                            out_node->AddEdge(true_vars.at(var));
                        } else {
                            out_node->AddEdge(variables_.Get(var));
                        }
                    }
                    if (false_has_next) {
                        if (false_vars.count(var)) {
                            out_node->AddEdge(false_vars.at(var));
                        } else {
                            out_node->AddEdge(variables_.Get(var));
                        }
                    }

                    variables_.Set(var, out_node);
                }

                if (sem_if->Behaviors() != sem::Behaviors{sem::Behavior::kNext}) {
                    auto* cf_end = CreateNode("if_CFend");
                    cf_end->AddEdge(cf1);
                    if (cf2) {
                        cf_end->AddEdge(cf2);
                    }
                    return cf_end;
                }
                return cf;
            },

            [&](const ast::IncrementDecrementStatement* i) {
                // The increment/decrement statement `i++` is equivalent to `i = i + 1`.
                auto [cf1, v1] = ProcessExpression(cf, i->lhs);
                auto* result = CreateNode("incdec_result");
                result->AddEdge(v1);
                result->AddEdge(cf1);

                auto [cf2, l2] = ProcessLValueExpression(cf1, i->lhs);
                l2->AddEdge(result);
                return cf2;
            },

            [&](const ast::LoopStatement* l) {
                auto* sem_loop = sem_.Get(l);
                auto cfx = CreateNode("loop_start");

                auto& info = loop_switch_infos_[sem_loop];
                info.type = "loop";

                // Create input nodes for any variables declared before this loop.
                for (auto* v : local_var_decls_) {
                    auto name = builder_->Symbols().NameFor(v->Declaration()->symbol);
                    auto* in_node = CreateNode(name + "_value_loop_in");
                    in_node->AddEdge(variables_.Get(v));
                    info.var_in_nodes[v] = in_node;
                    variables_.Set(v, in_node);
                }

                auto cf1 = ProcessStatement(cfx, l->body);
                if (l->continuing) {
                    auto cf2 = ProcessStatement(cf1, l->continuing);
                    cfx->AddEdge(cf2);
                } else {
                    cfx->AddEdge(cf1);
                }
                cfx->AddEdge(cf);

                // Add edges from variable loop input nodes to their values at the end of the loop.
                for (auto v : info.var_in_nodes) {
                    auto* in_node = v.second;
                    auto* out_node = variables_.Get(v.first);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }

                // Set each variable's exit node as its value in the outer scope.
                for (auto v : info.var_exit_nodes) {
                    variables_.Set(v.first, v.second);
                }

                loop_switch_infos_.erase(sem_loop);

                if (sem_loop->Behaviors() == sem::Behaviors{sem::Behavior::kNext}) {
                    return cf;
                } else {
                    return cfx;
                }
            },
            [&](const ast::ReturnStatement* r) {
                Node* cf_ret;
                if (r->value) {
                    auto [cf1, v] = ProcessExpression(cf, r->value);
                    cf_return_->AddEdge(cf1);
                    value_return_->AddEdge(v);
                    cf_ret = cf1;
                } else {
                    TINT_ASSERT(Resolver, cf != nullptr);
                    cf_return_->AddEdge(cf);
                    cf_ret = cf;
                }

                // Add edges from each pointer parameter output to its current value.
                for (auto param : pointer_param_returns_) {
                    param.second->AddEdge(variables_.Get(param.first));
                }

                return cf_ret;
            },
            [&](const ast::SwitchStatement* s) {
                auto* sem_switch = sem_.Get(s);
                auto [cfx, v] = ProcessExpression(cf, s->condition);

                Node* cf_end = nullptr;
                if (sem_switch->Behaviors() != sem::Behaviors{sem::Behavior::kNext}) {
                    cf_end = CreateNode("switch_CFend");
                }

                auto& info = loop_switch_infos_[sem_switch];
                info.type = "switch";

                auto cf_n = v;
                bool previous_case_has_fallthrough = false;
                for (auto* c : s->body) {
                    auto* sem_case = sem_.Get(c);

                    if (previous_case_has_fallthrough) {
                        cf_n = ProcessStatement(cf_n, c->body);
                    } else {
                        variables_.Push();
                        cf_n = ProcessStatement(v, c->body);
                    }

                    if (cf_end) {
                        cf_end->AddEdge(cf_n);
                    }

                    bool has_fallthrough =
                        sem_case->Behaviors().Contains(sem::Behavior::kFallthrough);
                    if (!has_fallthrough) {
                        if (sem_case->Behaviors().Contains(sem::Behavior::kNext)) {
                            // Propagate variable values to the switch exit nodes.
                            for (auto* var : local_var_decls_) {
                                // Skip variables that were declared inside the switch.
                                if (auto* lv = var->As<sem::LocalVariable>();
                                    lv && lv->Statement()->FindFirstParent(
                                              [&](auto* st) { return st == sem_switch; })) {
                                    continue;
                                }

                                // Add an edge from the variable exit node to its new value.
                                auto* exit_node =
                                    utils::GetOrCreate(info.var_exit_nodes, var, [&]() {
                                        auto name =
                                            builder_->Symbols().NameFor(var->Declaration()->symbol);
                                        return CreateNode(name + "_value_" + info.type + "_exit");
                                    });
                                exit_node->AddEdge(variables_.Get(var));
                            }
                        }
                        variables_.Pop();
                    }
                    previous_case_has_fallthrough = has_fallthrough;
                }

                // Update nodes for any variables assigned in the switch statement.
                for (auto var : info.var_exit_nodes) {
                    variables_.Set(var.first, var.second);
                }

                return cf_end ? cf_end : cf;
            },
            [&](const ast::VariableDeclStatement* decl) {
                Node* node;
                if (decl->variable->constructor) {
                    auto [cf1, v] = ProcessExpression(cf, decl->variable->constructor);
                    cf = cf1;
                    node = v;
                } else {
                    node = cf;
                }
                variables_.Set(sem_.Get(decl->variable), node);

                if (!decl->variable->is_const) {
                    local_var_decls_.insert(sem_.Get<sem::LocalVariable>(decl->variable));
                }

                return cf;
            },
            [&](Default) {
                TINT_ICE(Resolver, diagnostics_)
                    << "unknown statement type: " << std::string(stmt->TypeInfo().name);
                return nullptr;
            });
    }

    /// Process an identifier expression.
    /// @param cf the input control flow node
    /// @param ident the identifier expression to process
    /// @returns a pair of (control flow node, value node)
    std::pair<Node*, Node*> ProcessIdentExpression(Node* cf,
                                                   const ast::IdentifierExpression* ident) {
        // Helper to check if the entry point attribute of `obj` indicates non-uniformity.
        auto has_nonuniform_entry_point_attribute = [](auto* obj) {
            // Only the num_workgroups and workgroup_id builtins are uniform.
            if (auto* builtin = ast::GetAttribute<ast::BuiltinAttribute>(obj->attributes)) {
                if (builtin->builtin == ast::Builtin::kNumWorkgroups ||
                    builtin->builtin == ast::Builtin::kWorkgroupId) {
                    return false;
                }
            }
            return true;
        };

        auto name = builder_->Symbols().NameFor(ident->symbol);
        auto* sem = sem_.Get<sem::VariableUser>(ident)->Variable();
        return Switch(
            sem,

            [&](const sem::Parameter* param) {
                auto* user_func = param->Owner()->As<sem::Function>();
                if (user_func && user_func->Declaration()->IsEntryPoint()) {
                    if (auto* str = param->Type()->As<sem::Struct>()) {
                        // We consider the whole struct to be non-uniform if any one of its members
                        // is non-uniform.
                        bool uniform = true;
                        for (auto* member : str->Members()) {
                            if (has_nonuniform_entry_point_attribute(member->Declaration())) {
                                uniform = false;
                            }
                        }
                        return std::make_pair(cf, uniform ? cf : may_be_non_uniform_);
                    } else {
                        if (has_nonuniform_entry_point_attribute(param->Declaration())) {
                            return std::make_pair(cf, may_be_non_uniform_);
                        }
                        return std::make_pair(cf, cf);
                    }
                } else {
                    auto* result = CreateNode(name + "_result");
                    auto* x = variables_.Get(param);
                    result->AddEdge(cf);
                    result->AddEdge(x);
                    return std::make_pair(cf, result);
                }
            },

            [&](const sem::GlobalVariable* global) {
                if (global->Declaration()->is_const || global->Access() == ast::Access::kRead) {
                    return std::make_pair(cf, cf);
                } else {
                    return std::make_pair(cf, may_be_non_uniform_);
                }
            },

            [&](const sem::LocalVariable* local) {
                auto* result = CreateNode(name + "_result");
                result->AddEdge(cf);
                if (auto* x = variables_.Get(local)) {
                    result->AddEdge(x);
                }
                return std::make_pair(cf, result);
            },

            [&](Default) {
                TINT_ICE(Resolver, diagnostics_)
                    << "unknown identifier expression type: " << std::string(sem->TypeInfo().name);
                return std::pair<Node*, Node*>(nullptr, nullptr);
            });
    }

    /// Process an expression.
    /// @param cf the input control flow node
    /// @param expr the expression to process
    /// @returns a pair of (control flow node, value node)
    std::pair<Node*, Node*> ProcessExpression(Node* cf, const ast::Expression* expr) {
        return Switch(
            expr,

            [&](const ast::BinaryExpression* b) {
                if (b->IsLogical()) {
                    // Short-circuiting binary operators are a special case.
                    auto [cf1, v1] = ProcessExpression(cf, b->lhs);
                    auto [cf2, v2] = ProcessExpression(v1, b->rhs);
                    return std::pair<Node*, Node*>(cf2, v2);
                } else {
                    auto [cf1, v1] = ProcessExpression(cf, b->lhs);
                    auto [cf2, v2] = ProcessExpression(cf1, b->rhs);
                    auto* result = CreateNode("binary_expr_result");
                    result->AddEdge(v1);
                    result->AddEdge(v2);
                    return std::pair<Node*, Node*>(cf2, result);
                }
            },

            [&](const ast::BitcastExpression* b) { return ProcessExpression(cf, b->expr); },

            [&](const ast::CallExpression* c) { return ProcessCall(cf, c); },

            [&](const ast::IdentifierExpression* i) { return ProcessIdentExpression(cf, i); },

            [&](const ast::IndexAccessorExpression* i) {
                auto [cf1, v1] = ProcessExpression(cf, i->object);
                auto [cf2, v2] = ProcessExpression(cf1, i->index);
                auto* result = CreateNode("index_accessor_result");
                result->AddEdge(v1);
                result->AddEdge(v2);
                return std::pair<Node*, Node*>(cf2, result);
            },

            [&](const ast::LiteralExpression*) { return std::make_pair(cf, cf); },

            [&](const ast::MemberAccessorExpression* m) {
                return ProcessExpression(cf, m->structure);
            },

            [&](const ast::UnaryOpExpression* u) {
                if (u->op == ast::UnaryOp::kIndirection) {
                    // Cut the analysis short, since we only need to know the originating variable
                    // which is being accessed.
                    auto* source_var = sem_.Get(u)->SourceVariable();
                    auto* value = variables_.Get(source_var);
                    if (!value) {
                        value = cf;
                    }
                    return std::pair<Node*, Node*>(cf, value);
                }
                return ProcessExpression(cf, u->expr);
            },

            [&](Default) {
                TINT_ICE(Resolver, diagnostics_)
                    << "unknown expression type: " << std::string(expr->TypeInfo().name);
                return std::pair<Node*, Node*>(nullptr, nullptr);
            });
    }

    /// Process an LValue expression.
    /// @param cf the input control flow node
    /// @param expr the expression to process
    /// @returns a pair of (control flow node, variable node)
    std::pair<Node*, Node*> ProcessLValueExpression(Node* cf, const ast::Expression* expr) {
        return Switch(
            expr,

            [&](const ast::IdentifierExpression* i) {
                auto name = builder_->Symbols().NameFor(i->symbol);
                auto* sem = sem_.Get<sem::VariableUser>(i);
                if (sem->Variable()->Is<sem::GlobalVariable>()) {
                    return std::make_pair(cf, may_be_non_uniform_);
                } else if (auto* local = sem->Variable()->As<sem::LocalVariable>()) {
                    // Create a new value node for this variable.
                    auto* value = CreateNode(name + "_lvalue");
                    auto* old_value = variables_.Set(local, value);

                    // Aggregate values link back to their previous value, as they can never become
                    // uniform again.
                    if (!local->Type()->UnwrapRef()->is_scalar() && old_value) {
                        value->AddEdge(old_value);
                    }

                    return std::make_pair(cf, value);
                } else {
                    TINT_ICE(Resolver, diagnostics_)
                        << "unknown lvalue identifier expression type: "
                        << std::string(sem->Variable()->TypeInfo().name);
                    return std::pair<Node*, Node*>(nullptr, nullptr);
                }
            },

            [&](const ast::IndexAccessorExpression* i) {
                auto [cf1, l1] = ProcessLValueExpression(cf, i->object);
                auto [cf2, v2] = ProcessExpression(cf1, i->index);
                l1->AddEdge(v2);
                return std::pair<Node*, Node*>(cf2, l1);
            },

            [&](const ast::MemberAccessorExpression* m) {
                return ProcessLValueExpression(cf, m->structure);
            },

            [&](const ast::UnaryOpExpression* u) {
                if (u->op == ast::UnaryOp::kIndirection) {
                    // Cut the analysis short, since we only need to know the originating variable
                    // that is being written to.
                    auto* source_var = sem_.Get(u)->SourceVariable();
                    auto name = builder_->Symbols().NameFor(source_var->Declaration()->symbol);
                    auto* deref = CreateNode(name + "_deref");
                    auto* old_value = variables_.Set(source_var, deref);

                    // Aggregate values link back to their previous value, as they can never become
                    // uniform again.
                    if (!source_var->Type()->UnwrapRef()->UnwrapPtr()->is_scalar() && old_value) {
                        deref->AddEdge(old_value);
                    }

                    return std::pair<Node*, Node*>(cf, deref);
                }
                return ProcessLValueExpression(cf, u->expr);
            },

            [&](Default) {
                TINT_ICE(Resolver, diagnostics_)
                    << "unknown lvalue expression type: " << std::string(expr->TypeInfo().name);
                return std::pair<Node*, Node*>(nullptr, nullptr);
            });
    }

    /// Process a function call expression.
    /// @param cf the input control flow node
    /// @param call the function call to process
    /// @returns a pair of (control flow node, value node)
    std::pair<Node*, Node*> ProcessCall(Node* cf, const ast::CallExpression* call) {
        std::string name;
        if (call->target.name) {
            name = builder_->Symbols().NameFor(call->target.name->symbol);
        } else {
            name = call->target.type->FriendlyName(builder_->Symbols());
        }

        // Process call arguments
        Node* cf_last_arg = cf;
        std::vector<Node*> args;
        for (size_t i = 0; i < call->args.size(); i++) {
            auto [cf_i, arg_i] = ProcessExpression(cf_last_arg, call->args[i]);

            // Capture the index of this argument in a new node.
            // Note: This is an additional node that isn't described in the specification, for the
            // purpose of providing diagnostic information.
            Node* arg_node = CreateNode(name + "_arg_" + std::to_string(i), call);
            arg_node->arg_index = static_cast<uint32_t>(i);
            arg_node->AddEdge(arg_i);

            cf_last_arg = cf_i;
            args.push_back(arg_node);
        }

        Node* result = CreateNode("Result_" + name);
        Node* cf_after = CreateNode("CF_after_" + name, call);

        // Get tags for the callee.
        CallSiteTag callsite_tag = CallSiteNoRestriction;
        FunctionTag function_tag = NoRestriction;
        auto* sem = sem_.Get(call);
        const FunctionInfo* func_info = nullptr;
        Switch(
            sem->Target(),
            [&](const sem::Builtin* builtin) {
                // Most builtins have no restrictions. The exceptions are barriers, derivatives, and
                // some texture sampling builtins.
                if (builtin->IsBarrier()) {
                    callsite_tag = CallSiteRequiredToBeUniform;
                } else if (builtin->IsDerivative() ||
                           builtin->Type() == sem::BuiltinType::kTextureSample ||
                           builtin->Type() == sem::BuiltinType::kTextureSampleBias ||
                           builtin->Type() == sem::BuiltinType::kTextureSampleCompare) {
                    callsite_tag = CallSiteRequiredToBeUniform;
                    function_tag = ReturnValueMayBeNonUniform;
                } else {
                    callsite_tag = CallSiteNoRestriction;
                    function_tag = NoRestriction;
                }
            },
            [&](const sem::Function* func) {
                // We must have already analyzed the user-defined function since we process
                // functions in dependency order.
                TINT_ASSERT(Resolver, functions_.count(func->Declaration()));
                auto& info = functions_.at(func->Declaration());
                callsite_tag = info.callsite_tag;
                function_tag = info.function_tag;
                func_info = &info;
            },
            [&](const sem::TypeConstructor*) {
                callsite_tag = CallSiteNoRestriction;
                function_tag = NoRestriction;
            },
            [&](const sem::TypeConversion*) {
                callsite_tag = CallSiteNoRestriction;
                function_tag = NoRestriction;
            },
            [&](Default) {
                TINT_ICE(Resolver, diagnostics_) << "unhandled function call target: " << name;
            });

        if (callsite_tag == CallSiteRequiredToBeUniform) {
            // Note: This deviates from the rules in the specification, which would add the edge
            // directly to the incoming CF node. Going through CF_after instead makes it easier to
            // produce diagnostics that can identify the function being called.
            required_to_be_uniform_->AddEdge(cf_after);
        }
        cf_after->AddEdge(cf_last_arg);

        if (function_tag == SubsequentControlFlowMayBeNonUniform) {
            cf_after->AddEdge(may_be_non_uniform_);
        } else if (function_tag == ReturnValueMayBeNonUniform) {
            result->AddEdge(may_be_non_uniform_);
        }

        result->AddEdge(cf_after);

        // For each argument, add edges based on parameter tags.
        for (size_t i = 0; i < args.size(); i++) {
            if (func_info) {
                switch (func_info->parameters[i].tag) {
                    case ParameterRequiredToBeUniform:
                        required_to_be_uniform_->AddEdge(args[i]);
                        break;
                    case ParameterRequiredToBeUniformForSubsequentControlFlow:
                        cf_after->AddEdge(args[i]);
                        break;
                    case ParameterRequiredToBeUniformForReturnValue:
                        result->AddEdge(args[i]);
                        break;
                    case ParameterNoRestriction:
                        break;
                }

                auto* sem_arg = sem_.Get(call->args[i]);
                if (sem_arg->Type()->Is<sem::Pointer>()) {
                    auto* ptr_result =
                        CreateNode(name + "_ptrarg_" + std::to_string(i) + "_result");
                    if (func_info->parameters[i].pointer_may_become_non_uniform) {
                        ptr_result->AddEdge(may_be_non_uniform_);
                    } else {
                        // Add edges from the resulting pointer value to any other arguments that
                        // feed it.
                        for (auto* source : func_info->parameters[i].pointer_param_output_sources) {
                            ptr_result->AddEdge(args[source->Index()]);
                        }
                    }

                    // Update the current stored value for this pointer argument.
                    auto* source_var = sem_arg->SourceVariable();
                    TINT_ASSERT(Resolver, source_var);
                    variables_.Set(source_var, ptr_result);
                }
            } else {
                // All builtin function parameters are RequiredToBeUniformForReturnValue, as are
                // parameters for type constructors and type conversions.
                // The arrayLength() builtin is a special case, as there is currently no way for it
                // to have a non-uniform return value.
                auto* builtin = sem->Target()->As<sem::Builtin>();
                if (!builtin || builtin->Type() != sem::BuiltinType::kArrayLength) {
                    result->AddEdge(args[i]);
                }
            }
        }

        return {cf_after, result};
    }

    /// Recursively traverse a graph starting at `node`, inserting all nodes that are reached into
    /// `reachable`.
    /// @param node the starting node
    /// @param reachable the set of reachable nodes to populate
    void Traverse(Node* node, utils::UniqueVector<Node*>& reachable) {
        reachable.add(node);
        for (auto* to : node->edges) {
            if (to->visited_from == nullptr) {
                to->visited_from = node;
                Traverse(to, reachable);
            }
        }
    }

    /// Generate an error for a required_to_be_uniform->may_be_non_uniform path.
    void MakeError() {
        // Trace back to find a node that is required to be uniform that was reachable from a
        // non-uniform value or control flow node.
        Node* current = may_be_non_uniform_;
        while (current) {
            TINT_ASSERT(Resolver, current->visited_from);
            if (current->visited_from == required_to_be_uniform_) {
                break;
            }
            current = current->visited_from;
        }

        // The node will always have an corresponding call expression.
        auto* call = current->ast->As<ast::CallExpression>();
        TINT_ASSERT(Resolver, call);
        auto* target = sem_.Get(call)->Target();

        std::string name;
        if (auto* builtin = target->As<sem::Builtin>()) {
            name = builtin->str();
        } else if (auto* user = target->As<sem::Function>()) {
            name = builder_->Symbols().NameFor(user->Declaration()->symbol);
        }

        // TODO(jrprice): Switch to error instead of warning when feedback has settled.
        if (current->arg_index != std::numeric_limits<uint32_t>::max()) {
            // The requirement was on a function parameter.
            auto param_name = builder_->Symbols().NameFor(
                target->Parameters()[current->arg_index]->Declaration()->symbol);
            diagnostics_.add_warning(
                diag::System::Resolver,
                "parameter '" + param_name + "' of '" + name + "' must be uniform",
                call->args[current->arg_index]->source);
            // TODO(jrprice): Show the reason why.
        } else {
            // The requirement was on a function callsite.
            diagnostics_.add_warning(diag::System::Resolver,
                                     "'" + name + "' must only be called from uniform control flow",
                                     call->source);
            // TODO(jrprice): Show full call stack to the problematic builtin.
        }
    }
};

}  // namespace

bool AnalyzeUniformity(ProgramBuilder* builder, const DependencyGraph& dependency_graph) {
    if (builder->AST().Extensions().count(
            ast::Enable::ExtensionKind::kChromiumDisableUniformityAnalysis)) {
        return true;
    }

    UniformityGraph graph(builder);
    return graph.Build(dependency_graph);
}

}  // namespace tint::resolver
