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
#include "src/tint/sem/type_conversion.h"
#include "src/tint/sem/type_initializer.h"
#include "src/tint/sem/variable.h"
#include "src/tint/sem/while_statement.h"
#include "src/tint/utils/block_allocator.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/unique_vector.h"

// Set to `1` to dump the uniformity graph for each function in graphviz format.
#define TINT_DUMP_UNIFORMITY_GRAPH 0

namespace tint::resolver {

namespace {

/// Unwraps `u->expr`'s chain of indirect (*) and address-of (&) expressions, returning the first
/// expression that is neither of these.
/// E.g. If `u` is `*(&(*(&p)))`, returns `p`.
const ast::Expression* UnwrapIndirectAndAddressOfChain(const ast::UnaryOpExpression* u) {
    auto* e = u->expr;
    while (true) {
        auto* unary = e->As<ast::UnaryOpExpression>();
        if (unary &&
            (unary->op == ast::UnaryOp::kIndirection || unary->op == ast::UnaryOp::kAddressOf)) {
            e = unary->expr;
        } else {
            break;
        }
    }
    return e;
}

/// CallSiteTag describes the uniformity requirements on the call sites of a function.
enum CallSiteTag {
    CallSiteRequiredToBeUniform,
    CallSiteNoRestriction,
};

/// FunctionTag describes a functions effects on uniformity.
enum FunctionTag {
    ReturnValueMayBeNonUniform,
    NoRestriction,
};

/// ParameterTag describes the uniformity requirements of values passed to a function parameter.
enum ParameterTag {
    ParameterRequiredToBeUniform,
    ParameterRequiredToBeUniformForReturnValue,
    ParameterNoRestriction,
};

/// Node represents a node in the graph of control flow and value nodes within the analysis of a
/// single function.
struct Node {
    /// Constructor
    /// @param a the corresponding AST node
    explicit Node(const ast::Node* a) : ast(a) {}

#if TINT_DUMP_UNIFORMITY_GRAPH
    /// The node tag.
    std::string tag;
#endif

    /// Type describes the type of the node, which is used to determine additional diagnostic
    /// information.
    enum Type {
        kRegular,
        kFunctionCallArgument,
        kFunctionCallPointerArgumentResult,
        kFunctionCallReturnValue,
    };

    /// The type of the node.
    Type type = kRegular;

    /// `true` if this node represents a potential control flow change.
    bool affects_control_flow = false;

    /// The corresponding AST node, or nullptr.
    const ast::Node* ast = nullptr;

    /// The function call argument index, if applicable.
    uint32_t arg_index;

    /// The set of edges from this node to other nodes in the graph.
    utils::UniqueVector<Node*, 4> edges;

    /// The node that this node was visited from, or nullptr if not visited.
    Node* visited_from = nullptr;

    /// Add an edge to the `to` node.
    /// @param to the destination node
    void AddEdge(Node* to) { edges.Add(to); }
};

/// ParameterInfo holds information about the uniformity requirements and effects for a particular
/// function parameter.
struct ParameterInfo {
    /// The semantic node in corresponds to this parameter.
    const sem::Parameter* sem;
    /// The parameter's uniformity requirements.
    ParameterTag tag = ParameterNoRestriction;
    /// Will be `true` if this function may cause the contents of this pointer parameter to become
    /// non-uniform.
    bool pointer_may_become_non_uniform = false;
    /// The parameters that are required to be uniform for the contents of this pointer parameter to
    /// be uniform at function exit.
    utils::Vector<const sem::Parameter*, 8> pointer_param_output_sources;
    /// The node in the graph that corresponds to this parameter's initial value.
    Node* init_value;
    /// The node in the graph that corresponds to this parameter's output value (or nullptr).
    Node* pointer_return_value = nullptr;
};

/// FunctionInfo holds information about the uniformity requirements and effects for a particular
/// function, as well as the control flow graph.
struct FunctionInfo {
    /// Constructor
    /// @param func the AST function
    /// @param builder the program builder
    FunctionInfo(const ast::Function* func, const ProgramBuilder* builder) {
        name = builder->Symbols().NameFor(func->symbol);
        callsite_tag = CallSiteNoRestriction;
        function_tag = NoRestriction;

        // Create special nodes.
        required_to_be_uniform = CreateNode("RequiredToBeUniform");
        may_be_non_uniform = CreateNode("MayBeNonUniform");
        cf_start = CreateNode("CF_start");
        if (func->return_type) {
            value_return = CreateNode("Value_return");
        }

        // Create nodes for parameters.
        parameters.Resize(func->params.Length());
        for (size_t i = 0; i < func->params.Length(); i++) {
            auto* param = func->params[i];
            auto param_name = builder->Symbols().NameFor(param->symbol);
            auto* sem = builder->Sem().Get<sem::Parameter>(param);
            parameters[i].sem = sem;

            Node* node_init;
            if (sem->Type()->Is<sem::Pointer>()) {
                node_init = CreateNode("ptrparam_" + name + "_init");
                parameters[i].pointer_return_value = CreateNode("ptrparam_" + name + "_return");
                local_var_decls.Add(sem);
            } else {
                node_init = CreateNode("param_" + name);
            }
            parameters[i].init_value = node_init;
            variables.Set(sem, node_init);
        }
    }

    /// The name of the function.
    std::string name;

    /// The call site uniformity requirements.
    CallSiteTag callsite_tag;
    /// The function's uniformity effects.
    FunctionTag function_tag;
    /// The uniformity requirements of the function's parameters.
    utils::Vector<ParameterInfo, 8> parameters;

    /// The control flow graph.
    utils::BlockAllocator<Node> nodes;

    /// Special `RequiredToBeUniform` node.
    Node* required_to_be_uniform;
    /// Special `MayBeNonUniform` node.
    Node* may_be_non_uniform;
    /// Special `CF_start` node.
    Node* cf_start;
    /// Special `Value_return` node.
    Node* value_return;

    /// Map from variables to their value nodes in the graph, scoped with respect to control flow.
    ScopeStack<const sem::Variable*, Node*> variables;

    /// The set of a local read-write vars that are in scope at any given point in the process.
    /// Includes pointer parameters.
    utils::Hashset<const sem::Variable*, 8> local_var_decls;

    /// The set of partial pointer variables - pointers that point to a subobject (into an array or
    /// struct).
    utils::Hashset<const sem::Variable*, 4> partial_ptrs;

    /// LoopSwitchInfo tracks information about the value of variables for a control flow construct.
    struct LoopSwitchInfo {
        /// The type of this control flow construct.
        std::string type;
        /// The input values for local variables at the start of this construct.
        utils::Hashmap<const sem::Variable*, Node*, 8> var_in_nodes;
        /// The exit values for local variables at the end of this construct.
        utils::Hashmap<const sem::Variable*, Node*, 8> var_exit_nodes;
    };

    /// @returns a LoopSwitchInfo for the given statement, allocating the LoopSwitchInfo if this is
    /// the first call with the given statement.
    LoopSwitchInfo& LoopSwitchInfoFor(const sem::Statement* stmt) {
        return *loop_switch_infos.GetOrCreate(stmt,
                                              [&] { return loop_switch_info_allocator.Create(); });
    }

    /// Disassociates the LoopSwitchInfo for the given statement.
    void RemoveLoopSwitchInfoFor(const sem::Statement* stmt) { loop_switch_infos.Remove(stmt); }

    /// Create a new node.
    /// @param tag a tag used to identify the node for debugging purposes
    /// @param ast the optional AST node that this node corresponds to
    /// @returns the new node
    Node* CreateNode([[maybe_unused]] std::string tag, const ast::Node* ast = nullptr) {
        auto* node = nodes.Create(ast);

#if TINT_DUMP_UNIFORMITY_GRAPH
        // Make the tag unique and set it.
        // This only matters if we're dumping the graph.
        std::string unique_tag = tag;
        int suffix = 0;
        while (tags_.count(unique_tag)) {
            unique_tag = tag + "_$" + std::to_string(++suffix);
        }
        tags_.insert(unique_tag);
        node->tag = name + "." + unique_tag;
#endif

        return node;
    }

    /// Reset the visited status of every node in the graph.
    void ResetVisited() {
        for (auto* node : nodes.Objects()) {
            node->visited_from = nullptr;
        }
    }

  private:
    /// A list of tags that have already been used within the current function.
    utils::Hashset<std::string, 8> tags_;

    /// Map from control flow statements to the corresponding LoopSwitchInfo structure.
    utils::Hashmap<const sem::Statement*, LoopSwitchInfo*, 8> loop_switch_infos;

    /// Allocator of LoopSwitchInfos
    utils::BlockAllocator<LoopSwitchInfo> loop_switch_info_allocator;
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
    utils::Hashmap<const ast::Function*, FunctionInfo, 8> functions_;

    /// The function currently being analyzed.
    FunctionInfo* current_function_;

    /// Create a new node.
    /// @param tag a tag used to identify the node for debugging purposes.
    /// @param ast the optional AST node that this node corresponds to
    /// @returns the new node
    Node* CreateNode(std::string tag, const ast::Node* ast = nullptr) {
        return current_function_->CreateNode(std::move(tag), ast);
    }

    /// Process a function.
    /// @param func the function to process
    /// @returns true if there are no uniformity issues, false otherwise
    bool ProcessFunction(const ast::Function* func) {
        current_function_ = functions_.Add(func, FunctionInfo(func, builder_)).value;

        // Process function body.
        if (func->body) {
            ProcessStatement(current_function_->cf_start, func->body);
        }

#if TINT_DUMP_UNIFORMITY_GRAPH
        // Dump the graph for this function as a subgraph.
        std::cout << "\nsubgraph cluster_" << current_function_->name << " {\n";
        std::cout << "  label=" << current_function_->name << ";";
        for (auto* node : current_function_->nodes.Objects()) {
            std::cout << "\n  \"" << node->tag << "\";";
            for (auto* edge : node->edges) {
                std::cout << "\n  \"" << node->tag << "\" -> \"" << edge->tag << "\";";
            }
        }
        std::cout << "\n}\n";
#endif

        // Look at which nodes are reachable from "RequiredToBeUniform".
        {
            utils::UniqueVector<Node*, 4> reachable;
            Traverse(current_function_->required_to_be_uniform, &reachable);
            if (reachable.Contains(current_function_->may_be_non_uniform)) {
                MakeError(*current_function_, current_function_->may_be_non_uniform);
                return false;
            }
            if (reachable.Contains(current_function_->cf_start)) {
                current_function_->callsite_tag = CallSiteRequiredToBeUniform;
            }

            // Set the parameter tag to ParameterRequiredToBeUniform for each parameter node that
            // was reachable.
            for (size_t i = 0; i < func->params.Length(); i++) {
                auto* param = func->params[i];
                if (reachable.Contains(current_function_->variables.Get(sem_.Get(param)))) {
                    current_function_->parameters[i].tag = ParameterRequiredToBeUniform;
                }
            }
        }

        // If "Value_return" exists, look at which nodes are reachable from it
        if (current_function_->value_return) {
            utils::UniqueVector<Node*, 4> reachable;
            Traverse(current_function_->value_return, &reachable);
            if (reachable.Contains(current_function_->may_be_non_uniform)) {
                current_function_->function_tag = ReturnValueMayBeNonUniform;
            }

            // Set the parameter tag to ParameterRequiredToBeUniformForReturnValue for each
            // parameter node that was reachable.
            for (size_t i = 0; i < func->params.Length(); i++) {
                auto* param = func->params[i];
                if (reachable.Contains(current_function_->variables.Get(sem_.Get(param)))) {
                    current_function_->parameters[i].tag =
                        ParameterRequiredToBeUniformForReturnValue;
                }
            }
        }

        // Traverse the graph for each pointer parameter.
        for (size_t i = 0; i < func->params.Length(); i++) {
            if (current_function_->parameters[i].pointer_return_value == nullptr) {
                continue;
            }

            // Reset "visited" state for all nodes.
            current_function_->ResetVisited();

            utils::UniqueVector<Node*, 4> reachable;
            Traverse(current_function_->parameters[i].pointer_return_value, &reachable);
            if (reachable.Contains(current_function_->may_be_non_uniform)) {
                current_function_->parameters[i].pointer_may_become_non_uniform = true;
            }

            // Check every other parameter to see if they feed into this parameter's final value.
            for (size_t j = 0; j < func->params.Length(); j++) {
                auto* param_source = sem_.Get<sem::Parameter>(func->params[j]);
                if (reachable.Contains(current_function_->parameters[j].init_value)) {
                    current_function_->parameters[i].pointer_param_output_sources.Push(
                        param_source);
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
                utils::Hashmap<const sem::Variable*, Node*, 8> scoped_assignments;
                {
                    // Push a new scope for variable assignments in the block.
                    current_function_->variables.Push();
                    TINT_DEFER(current_function_->variables.Pop());

                    for (auto* s : b->statements) {
                        cf = ProcessStatement(cf, s);
                        if (!sem_.Get(s)->Behaviors().Contains(sem::Behavior::kNext)) {
                            break;
                        }
                    }

                    if (sem_.Get<sem::FunctionBlockStatement>(b)) {
                        // We've reached the end of the function body.
                        // Add edges from pointer parameter outputs to their current value.
                        for (auto param : current_function_->parameters) {
                            if (param.pointer_return_value) {
                                param.pointer_return_value->AddEdge(
                                    current_function_->variables.Get(param.sem));
                            }
                        }
                    }

                    scoped_assignments = std::move(current_function_->variables.Top());
                }

                // Propagate all variables assignments to the containing scope if the behavior is
                // 'Next'.
                auto& behaviors = sem_.Get(b)->Behaviors();
                if (behaviors.Contains(sem::Behavior::kNext)) {
                    for (auto var : scoped_assignments) {
                        current_function_->variables.Set(var.key, var.value);
                    }
                }

                // Remove any variables declared in this scope from the set of in-scope variables.
                for (auto decl : sem_.Get<sem::BlockStatement>(b)->Decls()) {
                    current_function_->local_var_decls.Remove(decl.value.variable);
                }

                return cf;
            },

            [&](const ast::BreakStatement* b) {
                // Find the loop or switch statement that we are in.
                auto* parent = sem_.Get(b)
                                   ->FindFirstParent<sem::SwitchStatement, sem::LoopStatement,
                                                     sem::ForLoopStatement, sem::WhileStatement>();

                auto& info = current_function_->LoopSwitchInfoFor(parent);

                // Propagate variable values to the loop/switch exit nodes.
                for (auto* var : current_function_->local_var_decls) {
                    // Skip variables that were declared inside this loop/switch.
                    if (auto* lv = var->As<sem::LocalVariable>();
                        lv &&
                        lv->Statement()->FindFirstParent([&](auto* s) { return s == parent; })) {
                        continue;
                    }

                    // Add an edge from the variable exit node to its value at this point.
                    auto* exit_node = info.var_exit_nodes.GetOrCreate(var, [&]() {
                        auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                        return CreateNode(name + "_value_" + info.type + "_exit");
                    });
                    exit_node->AddEdge(current_function_->variables.Get(var));
                }

                return cf;
            },

            [&](const ast::BreakIfStatement* b) {
                // This works very similar to the IfStatement uniformity below, execpt instead of
                // processing the body, we directly inline the BreakStatement uniformity from
                // above.

                auto [_, v_cond] = ProcessExpression(cf, b->condition);

                // Add a diagnostic node to capture the control flow change.
                auto* v = current_function_->CreateNode("break_if_stmt", b);
                v->affects_control_flow = true;
                v->AddEdge(v_cond);

                {
                    auto* parent = sem_.Get(b)->FindFirstParent<sem::LoopStatement>();
                    auto& info = current_function_->LoopSwitchInfoFor(parent);

                    // Propagate variable values to the loop exit nodes.
                    for (auto* var : current_function_->local_var_decls) {
                        // Skip variables that were declared inside this loop.
                        if (auto* lv = var->As<sem::LocalVariable>();
                            lv && lv->Statement()->FindFirstParent(
                                      [&](auto* s) { return s == parent; })) {
                            continue;
                        }

                        // Add an edge from the variable exit node to its value at this point.
                        auto* exit_node = info.var_exit_nodes.GetOrCreate(var, [&]() {
                            auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                            return CreateNode(name + "_value_" + info.type + "_exit");
                        });

                        exit_node->AddEdge(current_function_->variables.Get(var));
                    }
                }

                auto* sem_break_if = sem_.Get(b);
                if (sem_break_if->Behaviors() != sem::Behaviors{sem::Behavior::kNext}) {
                    auto* cf_end = CreateNode("break_if_CFend");
                    cf_end->AddEdge(v);
                    return cf_end;
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
                auto* parent = sem_.Get(c)
                                   ->FindFirstParent<sem::LoopStatement, sem::ForLoopStatement,
                                                     sem::WhileStatement>();
                auto& info = current_function_->LoopSwitchInfoFor(parent);

                // Propagate assignments to the loop input nodes.
                for (auto* var : current_function_->local_var_decls) {
                    // Skip variables that were declared inside this loop.
                    if (auto* lv = var->As<sem::LocalVariable>();
                        lv &&
                        lv->Statement()->FindFirstParent([&](auto* s) { return s == parent; })) {
                        continue;
                    }

                    // Add an edge from the variable's loop input node to its value at this point.
                    auto** in_node = info.var_in_nodes.Find(var);
                    TINT_ASSERT(Resolver, in_node != nullptr);
                    auto* out_node = current_function_->variables.Get(var);
                    if (out_node != *in_node) {
                        (*in_node)->AddEdge(out_node);
                    }
                }
                return cf;
            },

            [&](const ast::DiscardStatement*) { return cf; },

            [&](const ast::ForLoopStatement* f) {
                auto* sem_loop = sem_.Get(f);
                auto* cfx = CreateNode("loop_start");

                // Insert the initializer before the loop.
                auto* cf_init = cf;
                if (f->initializer) {
                    cf_init = ProcessStatement(cf, f->initializer);
                }
                auto* cf_start = cf_init;

                auto& info = current_function_->LoopSwitchInfoFor(sem_loop);
                info.type = "forloop";

                // Create input nodes for any variables declared before this loop.
                for (auto* v : current_function_->local_var_decls) {
                    auto name = builder_->Symbols().NameFor(v->Declaration()->symbol);
                    auto* in_node = CreateNode(name + "_value_forloop_in");
                    in_node->AddEdge(current_function_->variables.Get(v));
                    info.var_in_nodes.Replace(v, in_node);
                    current_function_->variables.Set(v, in_node);
                }

                // Insert the condition at the start of the loop body.
                if (f->condition) {
                    auto [cf_cond, v] = ProcessExpression(cfx, f->condition);
                    auto* cf_condition_end = CreateNode("for_condition_CFend", f);
                    cf_condition_end->affects_control_flow = true;
                    cf_condition_end->AddEdge(v);
                    cf_start = cf_condition_end;

                    // Propagate assignments to the loop exit nodes.
                    for (auto* var : current_function_->local_var_decls) {
                        auto* exit_node = info.var_exit_nodes.GetOrCreate(var, [&]() {
                            auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                            return CreateNode(name + "_value_" + info.type + "_exit");
                        });
                        exit_node->AddEdge(current_function_->variables.Get(var));
                    }
                }
                auto* cf1 = ProcessStatement(cf_start, f->body);

                // Insert the continuing statement at the end of the loop body.
                if (f->continuing) {
                    auto* cf2 = ProcessStatement(cf1, f->continuing);
                    cfx->AddEdge(cf2);
                } else {
                    cfx->AddEdge(cf1);
                }
                cfx->AddEdge(cf);

                // Add edges from variable loop input nodes to their values at the end of the loop.
                for (auto v : info.var_in_nodes) {
                    auto* in_node = v.value;
                    auto* out_node = current_function_->variables.Get(v.key);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }

                // Set each variable's exit node as its value in the outer scope.
                for (auto& v : info.var_exit_nodes) {
                    current_function_->variables.Set(v.key, v.value);
                }

                current_function_->RemoveLoopSwitchInfoFor(sem_loop);

                if (sem_loop->Behaviors() == sem::Behaviors{sem::Behavior::kNext}) {
                    return cf;
                } else {
                    return cfx;
                }
            },

            [&](const ast::WhileStatement* w) {
                auto* sem_loop = sem_.Get(w);
                auto* cfx = CreateNode("loop_start");

                auto* cf_start = cf;

                auto& info = current_function_->LoopSwitchInfoFor(sem_loop);
                info.type = "whileloop";

                // Create input nodes for any variables declared before this loop.
                for (auto* v : current_function_->local_var_decls) {
                    auto name = builder_->Symbols().NameFor(v->Declaration()->symbol);
                    auto* in_node = CreateNode(name + "_value_forloop_in");
                    in_node->AddEdge(current_function_->variables.Get(v));
                    info.var_in_nodes.Replace(v, in_node);
                    current_function_->variables.Set(v, in_node);
                }

                // Insert the condition at the start of the loop body.
                {
                    auto [cf_cond, v] = ProcessExpression(cfx, w->condition);
                    auto* cf_condition_end = CreateNode("while_condition_CFend", w);
                    cf_condition_end->affects_control_flow = true;
                    cf_condition_end->AddEdge(v);
                    cf_start = cf_condition_end;
                }

                // Propagate assignments to the loop exit nodes.
                for (auto* var : current_function_->local_var_decls) {
                    auto* exit_node = info.var_exit_nodes.GetOrCreate(var, [&]() {
                        auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                        return CreateNode(name + "_value_" + info.type + "_exit");
                    });
                    exit_node->AddEdge(current_function_->variables.Get(var));
                }
                auto* cf1 = ProcessStatement(cf_start, w->body);
                cfx->AddEdge(cf1);
                cfx->AddEdge(cf);

                // Add edges from variable loop input nodes to their values at the end of the loop.
                for (auto& v : info.var_in_nodes) {
                    auto* in_node = v.value;
                    auto* out_node = current_function_->variables.Get(v.key);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }

                // Set each variable's exit node as its value in the outer scope.
                for (auto v : info.var_exit_nodes) {
                    current_function_->variables.Set(v.key, v.value);
                }

                current_function_->RemoveLoopSwitchInfoFor(sem_loop);

                if (sem_loop->Behaviors() == sem::Behaviors{sem::Behavior::kNext}) {
                    return cf;
                } else {
                    return cfx;
                }
            },

            [&](const ast::IfStatement* i) {
                auto* sem_if = sem_.Get(i);
                auto [_, v_cond] = ProcessExpression(cf, i->condition);

                // Add a diagnostic node to capture the control flow change.
                auto* v = current_function_->CreateNode("if_stmt", i);
                v->affects_control_flow = true;
                v->AddEdge(v_cond);

                utils::Hashmap<const sem::Variable*, Node*, 8> true_vars;
                utils::Hashmap<const sem::Variable*, Node*, 8> false_vars;

                // Helper to process a statement with a new scope for variable assignments.
                // Populates `assigned_vars` with new nodes for any variables that are assigned in
                // this statement.
                auto process_in_scope =
                    [&](Node* cf_in, const ast::Statement* s,
                        utils::Hashmap<const sem::Variable*, Node*, 8>& assigned_vars) {
                        // Push a new scope for variable assignments.
                        current_function_->variables.Push();

                        // Process the statement.
                        auto* cf_out = ProcessStatement(cf_in, s);

                        assigned_vars = current_function_->variables.Top();

                        // Pop the scope and return.
                        current_function_->variables.Pop();
                        return cf_out;
                    };

                auto* cf1 = process_in_scope(v, i->body, true_vars);

                bool true_has_next = sem_.Get(i->body)->Behaviors().Contains(sem::Behavior::kNext);
                bool false_has_next = true;

                Node* cf2 = nullptr;
                if (i->else_statement) {
                    cf2 = process_in_scope(v, i->else_statement, false_vars);

                    false_has_next =
                        sem_.Get(i->else_statement)->Behaviors().Contains(sem::Behavior::kNext);
                }

                // Update values for any variables assigned in the if or else blocks.
                for (auto* var : current_function_->local_var_decls) {
                    // Skip variables not assigned in either block.
                    if (!true_vars.Contains(var) && !false_vars.Contains(var)) {
                        continue;
                    }

                    // Create an exit node for the variable.
                    auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                    auto* out_node = CreateNode(name + "_value_if_exit");

                    // Add edges to the assigned value or the initial value.
                    // Only add edges if the behavior for that block contains 'Next'.
                    if (true_has_next) {
                        if (true_vars.Contains(var)) {
                            out_node->AddEdge(*true_vars.Find(var));
                        } else {
                            out_node->AddEdge(current_function_->variables.Get(var));
                        }
                    }
                    if (false_has_next) {
                        if (false_vars.Contains(var)) {
                            out_node->AddEdge(*false_vars.Find(var));
                        } else {
                            out_node->AddEdge(current_function_->variables.Get(var));
                        }
                    }

                    current_function_->variables.Set(var, out_node);
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
                auto* cfx = CreateNode("loop_start");

                auto& info = current_function_->LoopSwitchInfoFor(sem_loop);
                info.type = "loop";

                // Create input nodes for any variables declared before this loop.
                for (auto* v : current_function_->local_var_decls) {
                    auto name = builder_->Symbols().NameFor(v->Declaration()->symbol);
                    auto* in_node = CreateNode(name + "_value_loop_in", v->Declaration());
                    in_node->AddEdge(current_function_->variables.Get(v));
                    info.var_in_nodes.Replace(v, in_node);
                    current_function_->variables.Set(v, in_node);
                }

                auto* cf1 = ProcessStatement(cfx, l->body);
                if (l->continuing) {
                    auto* cf2 = ProcessStatement(cf1, l->continuing);
                    cfx->AddEdge(cf2);
                } else {
                    cfx->AddEdge(cf1);
                }
                cfx->AddEdge(cf);

                // Add edges from variable loop input nodes to their values at the end of the loop.
                for (auto v : info.var_in_nodes) {
                    auto* in_node = v.value;
                    auto* out_node = current_function_->variables.Get(v.key);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }

                // Set each variable's exit node as its value in the outer scope.
                for (auto v : info.var_exit_nodes) {
                    current_function_->variables.Set(v.key, v.value);
                }

                current_function_->RemoveLoopSwitchInfoFor(sem_loop);

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
                    current_function_->value_return->AddEdge(v);
                    cf_ret = cf1;
                } else {
                    TINT_ASSERT(Resolver, cf != nullptr);
                    cf_ret = cf;
                }

                // Add edges from each pointer parameter output to its current value.
                for (auto param : current_function_->parameters) {
                    if (param.pointer_return_value) {
                        param.pointer_return_value->AddEdge(
                            current_function_->variables.Get(param.sem));
                    }
                }

                return cf_ret;
            },

            [&](const ast::SwitchStatement* s) {
                auto* sem_switch = sem_.Get(s);
                auto [cfx, v_cond] = ProcessExpression(cf, s->condition);

                // Add a diagnostic node to capture the control flow change.
                auto* v = current_function_->CreateNode("switch_stmt", s);
                v->affects_control_flow = true;
                v->AddEdge(v_cond);

                Node* cf_end = nullptr;
                if (sem_switch->Behaviors() != sem::Behaviors{sem::Behavior::kNext}) {
                    cf_end = CreateNode("switch_CFend");
                }

                auto& info = current_function_->LoopSwitchInfoFor(sem_switch);
                info.type = "switch";

                auto* cf_n = v;
                for (auto* c : s->body) {
                    auto* sem_case = sem_.Get(c);

                    current_function_->variables.Push();
                    cf_n = ProcessStatement(v, c->body);

                    if (cf_end) {
                        cf_end->AddEdge(cf_n);
                    }

                    if (sem_case->Behaviors().Contains(sem::Behavior::kNext)) {
                        // Propagate variable values to the switch exit nodes.
                        for (auto* var : current_function_->local_var_decls) {
                            // Skip variables that were declared inside the switch.
                            if (auto* lv = var->As<sem::LocalVariable>();
                                lv && lv->Statement()->FindFirstParent(
                                          [&](auto* st) { return st == sem_switch; })) {
                                continue;
                            }

                            // Add an edge from the variable exit node to its new value.
                            auto* exit_node = info.var_exit_nodes.GetOrCreate(var, [&]() {
                                auto name = builder_->Symbols().NameFor(var->Declaration()->symbol);
                                return CreateNode(name + "_value_" + info.type + "_exit");
                            });
                            exit_node->AddEdge(current_function_->variables.Get(var));
                        }
                    }
                    current_function_->variables.Pop();
                }

                // Update nodes for any variables assigned in the switch statement.
                for (auto var : info.var_exit_nodes) {
                    current_function_->variables.Set(var.key, var.value);
                }

                return cf_end ? cf_end : cf;
            },

            [&](const ast::VariableDeclStatement* decl) {
                Node* node;
                auto* sem_var = sem_.Get(decl->variable);
                if (decl->variable->initializer) {
                    auto [cf1, v] = ProcessExpression(cf, decl->variable->initializer);
                    cf = cf1;
                    node = v;

                    // Store if lhs is a partial pointer
                    if (sem_var->Type()->Is<sem::Pointer>()) {
                        auto* init = sem_.Get(decl->variable->initializer);
                        if (auto* unary_init = init->Declaration()->As<ast::UnaryOpExpression>()) {
                            auto* e = UnwrapIndirectAndAddressOfChain(unary_init);
                            if (e->IsAnyOf<ast::IndexAccessorExpression,
                                           ast::MemberAccessorExpression>()) {
                                current_function_->partial_ptrs.Add(sem_var);
                            }
                        }
                    }
                } else {
                    node = cf;
                }
                current_function_->variables.Set(sem_var, node);

                if (decl->variable->Is<ast::Var>()) {
                    current_function_->local_var_decls.Add(
                        sem_.Get<sem::LocalVariable>(decl->variable));
                }

                return cf;
            },

            [&](const ast::StaticAssert*) {
                return cf;  // No impact on uniformity
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
                if (builtin->builtin == ast::BuiltinValue::kNumWorkgroups ||
                    builtin->builtin == ast::BuiltinValue::kWorkgroupId) {
                    return false;
                }
            }
            return true;
        };

        auto name = builder_->Symbols().NameFor(ident->symbol);
        auto* sem = sem_.Get(ident)->UnwrapMaterialize()->As<sem::VariableUser>()->Variable();
        auto* node = CreateNode(name + "_ident_expr", ident);
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
                        node->AddEdge(uniform ? cf : current_function_->may_be_non_uniform);
                        return std::make_pair(cf, node);
                    } else {
                        if (has_nonuniform_entry_point_attribute(param->Declaration())) {
                            node->AddEdge(current_function_->may_be_non_uniform);
                        } else {
                            node->AddEdge(cf);
                        }
                        return std::make_pair(cf, node);
                    }
                } else {
                    auto* x = current_function_->variables.Get(param);
                    node->AddEdge(cf);
                    node->AddEdge(x);
                    return std::make_pair(cf, node);
                }
            },

            [&](const sem::GlobalVariable* global) {
                if (!global->Declaration()->Is<ast::Var>() ||
                    global->Access() == ast::Access::kRead) {
                    node->AddEdge(cf);
                } else {
                    node->AddEdge(current_function_->may_be_non_uniform);
                }
                return std::make_pair(cf, node);
            },

            [&](const sem::LocalVariable* local) {
                node->AddEdge(cf);
                if (auto* x = current_function_->variables.Get(local)) {
                    node->AddEdge(x);
                }
                return std::make_pair(cf, node);
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

                    // Add a diagnostic node to capture the control flow change.
                    auto* v1_cf = current_function_->CreateNode("short_circuit_op", b);
                    v1_cf->affects_control_flow = true;
                    v1_cf->AddEdge(v1);

                    auto [cf2, v2] = ProcessExpression(v1_cf, b->rhs);
                    return std::pair<Node*, Node*>(cf, v2);
                } else {
                    auto [cf1, v1] = ProcessExpression(cf, b->lhs);
                    auto [cf2, v2] = ProcessExpression(cf1, b->rhs);
                    auto* result = CreateNode("binary_expr_result", b);
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
                    auto* root_ident = sem_.Get(u)->RootIdentifier();
                    auto* value = current_function_->variables.Get(root_ident);
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

    /// @param u unary expression with op == kIndirection
    /// @returns true if `u` is an indirection unary expression that ultimately dereferences a
    /// partial pointer, false otherwise.
    bool IsDerefOfPartialPointer(const ast::UnaryOpExpression* u) {
        TINT_ASSERT(Resolver, u->op == ast::UnaryOp::kIndirection);

        // To determine if we're dereferencing a partial pointer, unwrap *&
        // chains; if the final expression is an identifier, see if it's a
        // partial pointer. If it's not an identifier, then it must be an
        // index/accessor expression, and thus a partial pointer.
        auto* e = UnwrapIndirectAndAddressOfChain(u);
        if (auto* var_user = sem_.Get<sem::VariableUser>(e)) {
            if (current_function_->partial_ptrs.Contains(var_user->Variable())) {
                return true;
            }
        } else {
            TINT_ASSERT(
                Resolver,
                (e->IsAnyOf<ast::IndexAccessorExpression, ast::MemberAccessorExpression>()));
            return true;
        }
        return false;
    }

    /// Process an LValue expression.
    /// @param cf the input control flow node
    /// @param expr the expression to process
    /// @returns a pair of (control flow node, variable node)
    std::pair<Node*, Node*> ProcessLValueExpression(Node* cf,
                                                    const ast::Expression* expr,
                                                    bool is_partial_reference = false) {
        return Switch(
            expr,

            [&](const ast::IdentifierExpression* i) {
                auto name = builder_->Symbols().NameFor(i->symbol);
                auto* sem = sem_.Get<sem::VariableUser>(i);
                if (sem->Variable()->Is<sem::GlobalVariable>()) {
                    return std::make_pair(cf, current_function_->may_be_non_uniform);
                } else if (auto* local = sem->Variable()->As<sem::LocalVariable>()) {
                    // Create a new value node for this variable.
                    auto* value = CreateNode(name + "_lvalue");
                    auto* old_value = current_function_->variables.Set(local, value);

                    // If i is part of an expression that is a partial reference to a variable (e.g.
                    // index or member access), we link back to the variable's previous value. If
                    // the previous value was non-uniform, a partial assignment will not make it
                    // uniform.
                    if (is_partial_reference && old_value) {
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
                auto [cf1, l1] =
                    ProcessLValueExpression(cf, i->object, /*is_partial_reference*/ true);
                auto [cf2, v2] = ProcessExpression(cf1, i->index);
                l1->AddEdge(v2);
                return std::pair<Node*, Node*>(cf2, l1);
            },

            [&](const ast::MemberAccessorExpression* m) {
                return ProcessLValueExpression(cf, m->structure, /*is_partial_reference*/ true);
            },

            [&](const ast::UnaryOpExpression* u) {
                if (u->op == ast::UnaryOp::kIndirection) {
                    // Cut the analysis short, since we only need to know the originating variable
                    // that is being written to.
                    auto* root_ident = sem_.Get(u)->RootIdentifier();
                    auto name = builder_->Symbols().NameFor(root_ident->Declaration()->symbol);
                    auto* deref = CreateNode(name + "_deref");
                    auto* old_value = current_function_->variables.Set(root_ident, deref);

                    if (old_value) {
                        // If derefercing a partial reference or partial pointer, we link back to
                        // the variable's previous value. If the previous value was non-uniform, a
                        // partial assignment will not make it uniform.
                        if (is_partial_reference || IsDerefOfPartialPointer(u)) {
                            deref->AddEdge(old_value);
                        }
                    }
                    return std::pair<Node*, Node*>(cf, deref);
                }
                return ProcessLValueExpression(cf, u->expr, is_partial_reference);
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
        utils::Vector<Node*, 8> args;
        for (size_t i = 0; i < call->args.Length(); i++) {
            auto [cf_i, arg_i] = ProcessExpression(cf_last_arg, call->args[i]);

            // Capture the index of this argument in a new node.
            // Note: This is an additional node that isn't described in the specification, for the
            // purpose of providing diagnostic information.
            Node* arg_node = CreateNode(name + "_arg_" + std::to_string(i), call);
            arg_node->type = Node::kFunctionCallArgument;
            arg_node->arg_index = static_cast<uint32_t>(i);
            arg_node->AddEdge(arg_i);

            cf_last_arg = cf_i;
            args.Push(arg_node);
        }

        // Note: This is an additional node that isn't described in the specification, for the
        // purpose of providing diagnostic information.
        Node* call_node = CreateNode(name + "_call", call);
        call_node->AddEdge(cf_last_arg);

        Node* result = CreateNode(name + "_return_value", call);
        result->type = Node::kFunctionCallReturnValue;
        Node* cf_after = CreateNode("CF_after_" + name, call);

        // Get tags for the callee.
        CallSiteTag callsite_tag = CallSiteNoRestriction;
        FunctionTag function_tag = NoRestriction;
        auto* sem = SemCall(call);
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
                auto* info = functions_.Find(func->Declaration());
                TINT_ASSERT(Resolver, info != nullptr);
                callsite_tag = info->callsite_tag;
                function_tag = info->function_tag;
                func_info = info;
            },
            [&](const sem::TypeInitializer*) {
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
            current_function_->required_to_be_uniform->AddEdge(call_node);
        }
        cf_after->AddEdge(call_node);

        if (function_tag == ReturnValueMayBeNonUniform) {
            result->AddEdge(current_function_->may_be_non_uniform);
        }

        result->AddEdge(cf_after);

        // For each argument, add edges based on parameter tags.
        for (size_t i = 0; i < args.Length(); i++) {
            if (func_info) {
                switch (func_info->parameters[i].tag) {
                    case ParameterRequiredToBeUniform:
                        current_function_->required_to_be_uniform->AddEdge(args[i]);
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
                        CreateNode(name + "_ptrarg_" + std::to_string(i) + "_result", call);
                    ptr_result->type = Node::kFunctionCallPointerArgumentResult;
                    ptr_result->arg_index = static_cast<uint32_t>(i);
                    if (func_info->parameters[i].pointer_may_become_non_uniform) {
                        ptr_result->AddEdge(current_function_->may_be_non_uniform);
                    } else {
                        // Add edge to the call to catch when it's called in non-uniform control
                        // flow.
                        ptr_result->AddEdge(call_node);

                        // Add edges from the resulting pointer value to any other arguments that
                        // feed it.
                        for (auto* source : func_info->parameters[i].pointer_param_output_sources) {
                            ptr_result->AddEdge(args[source->Index()]);
                        }
                    }

                    // Update the current stored value for this pointer argument.
                    auto* root_ident = sem_arg->RootIdentifier();
                    TINT_ASSERT(Resolver, root_ident);
                    current_function_->variables.Set(root_ident, ptr_result);
                }
            } else {
                // All builtin function parameters are RequiredToBeUniformForReturnValue, as are
                // parameters for type initializers and type conversions.
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

    /// Traverse a graph starting at `source`, inserting all visited nodes into `reachable` and
    /// recording which node they were reached from.
    /// @param source the starting node
    /// @param reachable the set of reachable nodes to populate, if required
    void Traverse(Node* source, utils::UniqueVector<Node*, 4>* reachable = nullptr) {
        utils::Vector<Node*, 8> to_visit{source};

        while (!to_visit.IsEmpty()) {
            auto* node = to_visit.Back();
            to_visit.Pop();

            if (reachable) {
                reachable->Add(node);
            }
            for (auto* to : node->edges) {
                if (to->visited_from == nullptr) {
                    to->visited_from = node;
                    to_visit.Push(to);
                }
            }
        }
    }

    /// Trace back along a path from `start` until finding a node that matches a predicate.
    /// @param start the starting node
    /// @param pred the predicate function
    /// @returns the first node found that matches the predicate, or nullptr
    template <typename F>
    Node* TraceBackAlongPathUntil(Node* start, F&& pred) {
        auto* current = start;
        while (current) {
            if (pred(current)) {
                break;
            }
            current = current->visited_from;
        }
        return current;
    }

    /// Recursively descend through the function called by `call` and the functions that it calls in
    /// order to find a call to a builtin function that requires uniformity.
    const ast::CallExpression* FindBuiltinThatRequiresUniformity(const ast::CallExpression* call) {
        auto* target = SemCall(call)->Target();
        if (target->Is<sem::Builtin>()) {
            // This is a call to a builtin, so we must be done.
            return call;
        } else if (auto* user = target->As<sem::Function>()) {
            // This is a call to a user-defined function, so inspect the functions called by that
            // function and look for one whose node has an edge from the RequiredToBeUniform node.
            auto* target_info = functions_.Find(user->Declaration());
            for (auto* call_node : target_info->required_to_be_uniform->edges) {
                if (call_node->type == Node::kRegular) {
                    auto* child_call = call_node->ast->As<ast::CallExpression>();
                    return FindBuiltinThatRequiresUniformity(child_call);
                }
            }
            TINT_ASSERT(Resolver, false && "unable to find child call with uniformity requirement");
        } else {
            TINT_ASSERT(Resolver, false && "unexpected call expression type");
        }
        return nullptr;
    }

    /// Add diagnostic notes to show where control flow became non-uniform on the way to a node.
    /// @param function the function being analyzed
    /// @param required_to_be_uniform the node to traverse from
    /// @param may_be_non_uniform the node to traverse to
    void ShowCauseOfNonUniformity(FunctionInfo& function,
                                  Node* required_to_be_uniform,
                                  Node* may_be_non_uniform) {
        // Traverse the graph to generate a path from the node to the source of non-uniformity.
        function.ResetVisited();
        Traverse(required_to_be_uniform);

        // Get the source of the non-uniform value.
        auto* non_uniform_source = may_be_non_uniform->visited_from;
        TINT_ASSERT(Resolver, non_uniform_source);

        // Show where the non-uniform value results in non-uniform control flow.
        auto* control_flow = TraceBackAlongPathUntil(
            non_uniform_source, [](Node* node) { return node->affects_control_flow; });
        if (control_flow) {
            diagnostics_.add_note(diag::System::Resolver,
                                  "control flow depends on non-uniform value",
                                  control_flow->ast->source);
            // TODO(jrprice): There are cases where the function with uniformity requirements is not
            // actually inside this control flow construct, for example:
            // - A conditional interrupt (e.g. break), with a barrier elsewhere in the loop
            // - A conditional assignment to a variable, which is later used to guard a barrier
            // In these cases, the diagnostics are not entirely accurate as they may not highlight
            // the actual cause of divergence.
        }

        auto get_var_type = [&](const sem::Variable* var) {
            switch (var->AddressSpace()) {
                case ast::AddressSpace::kStorage:
                    return "read_write storage buffer ";
                case ast::AddressSpace::kWorkgroup:
                    return "workgroup storage variable ";
                case ast::AddressSpace::kPrivate:
                    return "module-scope private variable ";
                default:
                    if (ast::HasAttribute<ast::BuiltinAttribute>(var->Declaration()->attributes)) {
                        return "builtin ";
                    } else if (ast::HasAttribute<ast::LocationAttribute>(
                                   var->Declaration()->attributes)) {
                        return "user-defined input ";
                    } else {
                        // TODO(jrprice): Provide more info for this case.
                    }
                    break;
            }
            return "";
        };

        // Show the source of the non-uniform value.
        Switch(
            non_uniform_source->ast,
            [&](const ast::IdentifierExpression* ident) {
                auto* var = sem_.Get<sem::VariableUser>(ident)->Variable();
                std::string var_type = get_var_type(var);
                diagnostics_.add_note(diag::System::Resolver,
                                      "reading from " + var_type + "'" +
                                          builder_->Symbols().NameFor(ident->symbol) +
                                          "' may result in a non-uniform value",
                                      ident->source);
            },
            [&](const ast::Variable* v) {
                auto* var = sem_.Get(v);
                std::string var_type = get_var_type(var);
                diagnostics_.add_note(diag::System::Resolver,
                                      "reading from " + var_type + "'" +
                                          builder_->Symbols().NameFor(v->symbol) +
                                          "' may result in a non-uniform value",
                                      v->source);
            },
            [&](const ast::CallExpression* c) {
                auto target_name = builder_->Symbols().NameFor(
                    c->target.name->As<ast::IdentifierExpression>()->symbol);
                switch (non_uniform_source->type) {
                    case Node::kFunctionCallReturnValue: {
                        diagnostics_.add_note(
                            diag::System::Resolver,
                            "return value of '" + target_name + "' may be non-uniform", c->source);
                        break;
                    }
                    case Node::kFunctionCallPointerArgumentResult: {
                        diagnostics_.add_note(
                            diag::System::Resolver,
                            "pointer contents may become non-uniform after calling '" +
                                target_name + "'",
                            c->args[non_uniform_source->arg_index]->source);
                        break;
                    }
                    default: {
                        TINT_ICE(Resolver, diagnostics_) << "unhandled source of non-uniformity";
                        break;
                    }
                }
            },
            [&](const ast::Expression* e) {
                diagnostics_.add_note(diag::System::Resolver,
                                      "result of expression may be non-uniform", e->source);
            },
            [&](Default) {
                TINT_ICE(Resolver, diagnostics_) << "unhandled source of non-uniformity";
            });
    }

    /// Generate an error message for a uniformity issue.
    /// @param function the function that the diagnostic is being produced for
    /// @param source_node the node that has caused a uniformity issue in `function`
    /// @param note `true` if the diagnostic should be emitted as a note
    void MakeError(FunctionInfo& function, Node* source_node, bool note = false) {
        // Helper to produce a diagnostic message with the severity required by this invocation of
        // the `MakeError` function.
        auto report = [&](Source source, std::string msg) {
            diag::Diagnostic error{};
            auto failureSeverity =
                kUniformityFailuresAsError ? diag::Severity::Error : diag::Severity::Warning;
            error.severity = note ? diag::Severity::Note : failureSeverity;
            error.system = diag::System::Resolver;
            error.source = source;
            error.message = msg;
            diagnostics_.add(std::move(error));
        };

        // Traverse the graph to generate a path from RequiredToBeUniform to the source node.
        function.ResetVisited();
        Traverse(function.required_to_be_uniform);
        TINT_ASSERT(Resolver, source_node->visited_from);

        // Find a node that is required to be uniform that has a path to the source node.
        auto* cause = TraceBackAlongPathUntil(source_node, [&](Node* node) {
            return node->visited_from == function.required_to_be_uniform;
        });

        // The node will always have a corresponding call expression.
        auto* call = cause->ast->As<ast::CallExpression>();
        TINT_ASSERT(Resolver, call);
        auto* target = SemCall(call)->Target();

        std::string func_name;
        if (auto* builtin = target->As<sem::Builtin>()) {
            func_name = builtin->str();
        } else if (auto* user = target->As<sem::Function>()) {
            func_name = builder_->Symbols().NameFor(user->Declaration()->symbol);
        }

        if (cause->type == Node::kFunctionCallArgument) {
            // The requirement was on a function parameter.
            auto param_name = builder_->Symbols().NameFor(
                target->Parameters()[cause->arg_index]->Declaration()->symbol);
            report(call->args[cause->arg_index]->source,
                   "parameter '" + param_name + "' of '" + func_name + "' must be uniform");

            // If this is a call to a user-defined function, add a note to show the reason that the
            // parameter is required to be uniform.
            if (auto* user = target->As<sem::Function>()) {
                auto* next_function = functions_.Find(user->Declaration());
                Node* next_cause = next_function->parameters[cause->arg_index].init_value;
                MakeError(*next_function, next_cause, true);
            }
        } else {
            // The requirement was on a function callsite.
            report(call->source,
                   "'" + func_name + "' must only be called from uniform control flow");

            // If this is a call to a user-defined function, add a note to show the builtin that
            // causes the uniformity requirement.
            auto* innermost_call = FindBuiltinThatRequiresUniformity(call);
            if (innermost_call != call) {
                auto* sem_call = SemCall(call);
                auto* sem_innermost_call = SemCall(innermost_call);

                // Determine whether the builtin is being called directly or indirectly.
                bool indirect = false;
                if (sem_call->Target()->As<sem::Function>() !=
                    sem_innermost_call->Stmt()->Function()) {
                    indirect = true;
                }

                auto* builtin = sem_innermost_call->Target()->As<sem::Builtin>();
                diagnostics_.add_note(diag::System::Resolver,
                                      "'" + func_name + "' requires uniformity because it " +
                                          (indirect ? "indirectly " : "") + "calls " +
                                          builtin->str(),
                                      innermost_call->source);
            }
        }

        // Show the cause of non-uniformity (starting at the top-level error).
        if (!note) {
            ShowCauseOfNonUniformity(function, function.required_to_be_uniform,
                                     function.may_be_non_uniform);
        }
    }

    // Helper for obtaining the sem::Call node for the ast::CallExpression
    const sem::Call* SemCall(const ast::CallExpression* expr) const {
        return sem_.Get(expr)->UnwrapMaterialize()->As<sem::Call>();
    }
};

}  // namespace

bool AnalyzeUniformity(ProgramBuilder* builder, const DependencyGraph& dependency_graph) {
    UniformityGraph graph(builder);
    return graph.Build(dependency_graph);
}

}  // namespace tint::resolver
