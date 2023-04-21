// Copyright 2021 The Tint Authors.
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

#include "src/tint/resolver/dependency_graph.h"

#include <string>
#include <utility>
#include <vector>

#include "src/tint/ast/alias.h"
#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/break_if_statement.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/compound_assignment_statement.h"
#include "src/tint/ast/const.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/diagnostic_attribute.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/identifier.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/increment_decrement_statement.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/invariant_attribute.h"
#include "src/tint/ast/let.h"
#include "src/tint/ast/location_attribute.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/must_use_attribute.h"
#include "src/tint/ast/override.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/ast/stride_attribute.h"
#include "src/tint/ast/struct.h"
#include "src/tint/ast/struct_member_align_attribute.h"
#include "src/tint/ast/struct_member_offset_attribute.h"
#include "src/tint/ast/struct_member_size_attribute.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/templated_identifier.h"
#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/ast/var.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/ast/while_statement.h"
#include "src/tint/ast/workgroup_attribute.h"
#include "src/tint/builtin/builtin.h"
#include "src/tint/builtin/builtin_value.h"
#include "src/tint/scope_stack.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/switch.h"
#include "src/tint/utils/block_allocator.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/utils/unique_vector.h"

#define TINT_DUMP_DEPENDENCY_GRAPH 0

namespace tint::resolver {
namespace {

// Forward declaration
struct Global;

/// Dependency describes how one global depends on another global
struct DependencyInfo {
    /// The source of the symbol that forms the dependency
    Source source;
};

/// DependencyEdge describes the two Globals used to define a dependency
/// relationship.
struct DependencyEdge {
    /// The Global that depends on #to
    const Global* from;
    /// The Global that is depended on by #from
    const Global* to;
};

/// DependencyEdgeCmp implements the contracts of std::equal_to<DependencyEdge>
/// and std::hash<DependencyEdge>.
struct DependencyEdgeCmp {
    /// Equality operator
    bool operator()(const DependencyEdge& lhs, const DependencyEdge& rhs) const {
        return lhs.from == rhs.from && lhs.to == rhs.to;
    }
    /// Hashing operator
    inline std::size_t operator()(const DependencyEdge& d) const {
        return utils::Hash(d.from, d.to);
    }
};

/// A map of DependencyEdge to DependencyInfo
using DependencyEdges =
    utils::Hashmap<DependencyEdge, DependencyInfo, 64, DependencyEdgeCmp, DependencyEdgeCmp>;

/// Global describes a module-scope variable, type or function.
struct Global {
    explicit Global(const ast::Node* n) : node(n) {}

    /// The declaration ast::Node
    const ast::Node* node;
    /// A list of dependencies that this global depends on
    utils::Vector<Global*, 8> deps;
};

/// A map of global name to Global
using GlobalMap = utils::Hashmap<Symbol, Global*, 16>;

/// Raises an ICE that a global ast::Node type was not handled by this system.
void UnhandledNode(diag::List& diagnostics, const ast::Node* node) {
    TINT_ICE(Resolver, diagnostics) << "unhandled node type: " << node->TypeInfo().name;
}

/// Raises an error diagnostic with the given message and source.
void AddError(diag::List& diagnostics, const std::string& msg, const Source& source) {
    diagnostics.add_error(diag::System::Resolver, msg, source);
}

/// Raises a note diagnostic with the given message and source.
void AddNote(diag::List& diagnostics, const std::string& msg, const Source& source) {
    diagnostics.add_note(diag::System::Resolver, msg, source);
}

/// DependencyScanner is used to traverse a module to build the list of
/// global-to-global dependencies.
class DependencyScanner {
  public:
    /// Constructor
    /// @param globals_by_name map of global symbol to Global pointer
    /// @param diagnostics diagnostic messages, appended with any errors found
    /// @param graph the dependency graph to populate with resolved symbols
    /// @param edges the map of globals-to-global dependency edges, which will
    /// be populated by calls to Scan()
    DependencyScanner(const GlobalMap& globals_by_name,
                      diag::List& diagnostics,
                      DependencyGraph& graph,
                      DependencyEdges& edges)
        : globals_(globals_by_name),
          diagnostics_(diagnostics),
          graph_(graph),
          dependency_edges_(edges) {
        // Register all the globals at global-scope
        for (auto it : globals_by_name) {
            scope_stack_.Set(it.key, it.value->node);
        }
    }

    /// Walks the global declarations, resolving symbols, and determining the
    /// dependencies of each global.
    void Scan(Global* global) {
        TINT_SCOPED_ASSIGNMENT(current_global_, global);
        Switch(
            global->node,
            [&](const ast::Struct* str) {
                Declare(str->name->symbol, str);
                for (auto* member : str->members) {
                    TraverseAttributes(member->attributes);
                    TraverseExpression(member->type);
                }
            },
            [&](const ast::Alias* alias) {
                Declare(alias->name->symbol, alias);
                TraverseExpression(alias->type);
            },
            [&](const ast::Function* func) {
                Declare(func->name->symbol, func);
                TraverseFunction(func);
            },
            [&](const ast::Variable* v) {
                Declare(v->name->symbol, v);
                TraverseVariable(v);
            },
            [&](const ast::DiagnosticDirective*) {
                // Diagnostic directives do not affect the dependency graph.
            },
            [&](const ast::Enable*) {
                // Enable directives do not affect the dependency graph.
            },
            [&](const ast::ConstAssert* assertion) { TraverseExpression(assertion->condition); },
            [&](Default) { UnhandledNode(diagnostics_, global->node); });
    }

  private:
    /// Traverses the variable, performing symbol resolution.
    void TraverseVariable(const ast::Variable* v) {
        if (auto* var = v->As<ast::Var>()) {
            TraverseExpression(var->declared_address_space);
            TraverseExpression(var->declared_access);
        }
        TraverseExpression(v->type);
        TraverseAttributes(v->attributes);
        TraverseExpression(v->initializer);
    }

    /// Traverses the function, performing symbol resolution and determining global dependencies.
    void TraverseFunction(const ast::Function* func) {
        TraverseAttributes(func->attributes);
        TraverseAttributes(func->return_type_attributes);
        // Perform symbol resolution on all the parameter types before registering
        // the parameters themselves. This allows the case of declaring a parameter
        // with the same identifier as its type.
        for (auto* param : func->params) {
            TraverseAttributes(param->attributes);
            TraverseExpression(param->type);
        }
        // Resolve the return type
        TraverseExpression(func->return_type);

        // Push the scope stack for the parameters and function body.
        scope_stack_.Push();
        TINT_DEFER(scope_stack_.Pop());

        for (auto* param : func->params) {
            if (auto* shadows = scope_stack_.Get(param->name->symbol)) {
                graph_.shadows.Add(param, shadows);
            }
            Declare(param->name->symbol, param);
        }
        if (func->body) {
            TraverseStatements(func->body->statements);
        }
    }

    /// Traverses the statements, performing symbol resolution and determining
    /// global dependencies.
    void TraverseStatements(utils::VectorRef<const ast::Statement*> stmts) {
        for (auto* s : stmts) {
            TraverseStatement(s);
        }
    }

    /// Traverses the statement, performing symbol resolution and determining
    /// global dependencies.
    void TraverseStatement(const ast::Statement* stmt) {
        if (!stmt) {
            return;
        }
        Switch(
            stmt,  //
            [&](const ast::AssignmentStatement* a) {
                TraverseExpression(a->lhs);
                TraverseExpression(a->rhs);
            },
            [&](const ast::BlockStatement* b) {
                scope_stack_.Push();
                TINT_DEFER(scope_stack_.Pop());
                TraverseStatements(b->statements);
            },
            [&](const ast::BreakIfStatement* b) { TraverseExpression(b->condition); },
            [&](const ast::CallStatement* r) { TraverseExpression(r->expr); },
            [&](const ast::CompoundAssignmentStatement* a) {
                TraverseExpression(a->lhs);
                TraverseExpression(a->rhs);
            },
            [&](const ast::ForLoopStatement* l) {
                scope_stack_.Push();
                TINT_DEFER(scope_stack_.Pop());
                TraverseStatement(l->initializer);
                TraverseExpression(l->condition);
                TraverseStatement(l->continuing);
                TraverseStatement(l->body);
            },
            [&](const ast::IncrementDecrementStatement* i) { TraverseExpression(i->lhs); },
            [&](const ast::LoopStatement* l) {
                scope_stack_.Push();
                TINT_DEFER(scope_stack_.Pop());
                TraverseStatements(l->body->statements);
                TraverseStatement(l->continuing);
            },
            [&](const ast::IfStatement* i) {
                TraverseExpression(i->condition);
                TraverseStatement(i->body);
                if (i->else_statement) {
                    TraverseStatement(i->else_statement);
                }
            },
            [&](const ast::ReturnStatement* r) { TraverseExpression(r->value); },
            [&](const ast::SwitchStatement* s) {
                TraverseExpression(s->condition);
                for (auto* c : s->body) {
                    for (auto* sel : c->selectors) {
                        TraverseExpression(sel->expr);
                    }
                    TraverseStatement(c->body);
                }
            },
            [&](const ast::VariableDeclStatement* v) {
                if (auto* shadows = scope_stack_.Get(v->variable->name->symbol)) {
                    graph_.shadows.Add(v->variable, shadows);
                }
                TraverseVariable(v->variable);
                Declare(v->variable->name->symbol, v->variable);
            },
            [&](const ast::WhileStatement* w) {
                scope_stack_.Push();
                TINT_DEFER(scope_stack_.Pop());
                TraverseExpression(w->condition);
                TraverseStatement(w->body);
            },
            [&](const ast::ConstAssert* assertion) { TraverseExpression(assertion->condition); },
            [&](Default) {
                if (TINT_UNLIKELY((!stmt->IsAnyOf<ast::BreakStatement, ast::ContinueStatement,
                                                  ast::DiscardStatement>()))) {
                    UnhandledNode(diagnostics_, stmt);
                }
            });
    }

    /// Adds the symbol definition to the current scope, raising an error if two
    /// symbols collide within the same scope.
    void Declare(Symbol symbol, const ast::Node* node) {
        auto* old = scope_stack_.Set(symbol, node);
        if (old != nullptr && node != old) {
            auto name = symbol.Name();
            AddError(diagnostics_, "redeclaration of '" + name + "'", node->source);
            AddNote(diagnostics_, "'" + name + "' previously declared here", old->source);
        }
    }

    /// Traverses the expression @p root_expr, performing symbol resolution and determining global
    /// dependencies.
    void TraverseExpression(const ast::Expression* root_expr) {
        if (!root_expr) {
            return;
        }

        utils::Vector<const ast::Expression*, 8> pending{root_expr};
        while (!pending.IsEmpty()) {
            ast::TraverseExpressions(pending.Pop(), diagnostics_, [&](const ast::Expression* expr) {
                Switch(
                    expr,
                    [&](const ast::IdentifierExpression* e) {
                        AddDependency(e->identifier, e->identifier->symbol);
                        if (auto* tmpl_ident = e->identifier->As<ast::TemplatedIdentifier>()) {
                            for (auto* arg : tmpl_ident->arguments) {
                                pending.Push(arg);
                            }
                        }
                    },
                    [&](const ast::CallExpression* call) { TraverseExpression(call->target); },
                    [&](const ast::BitcastExpression* cast) { TraverseExpression(cast->type); });
                return ast::TraverseAction::Descend;
            });
        }
    }

    /// Traverses the attribute list, performing symbol resolution and
    /// determining global dependencies.
    void TraverseAttributes(utils::VectorRef<const ast::Attribute*> attrs) {
        for (auto* attr : attrs) {
            TraverseAttribute(attr);
        }
    }

    /// Traverses the attribute, performing symbol resolution and determining
    /// global dependencies.
    void TraverseAttribute(const ast::Attribute* attr) {
        bool handled = Switch(
            attr,
            [&](const ast::BindingAttribute* binding) {
                TraverseExpression(binding->expr);
                return true;
            },
            [&](const ast::BuiltinAttribute* builtin) {
                TraverseExpression(builtin->builtin);
                return true;
            },
            [&](const ast::GroupAttribute* group) {
                TraverseExpression(group->expr);
                return true;
            },
            [&](const ast::IdAttribute* id) {
                TraverseExpression(id->expr);
                return true;
            },
            [&](const ast::InterpolateAttribute* interpolate) {
                TraverseExpression(interpolate->type);
                TraverseExpression(interpolate->sampling);
                return true;
            },
            [&](const ast::LocationAttribute* loc) {
                TraverseExpression(loc->expr);
                return true;
            },
            [&](const ast::StructMemberAlignAttribute* align) {
                TraverseExpression(align->expr);
                return true;
            },
            [&](const ast::StructMemberSizeAttribute* size) {
                TraverseExpression(size->expr);
                return true;
            },
            [&](const ast::WorkgroupAttribute* wg) {
                TraverseExpression(wg->x);
                TraverseExpression(wg->y);
                TraverseExpression(wg->z);
                return true;
            },
            [&](const ast::InternalAttribute* i) {
                for (auto* dep : i->dependencies) {
                    TraverseExpression(dep);
                }
                return true;
            });
        if (handled) {
            return;
        }

        if (attr->IsAnyOf<ast::BuiltinAttribute, ast::DiagnosticAttribute,
                          ast::InterpolateAttribute, ast::InvariantAttribute, ast::MustUseAttribute,
                          ast::StageAttribute, ast::StrideAttribute,
                          ast::StructMemberOffsetAttribute>()) {
            return;
        }

        UnhandledNode(diagnostics_, attr);
    }

    /// Adds the dependency from @p from to @p to, erroring if @p to cannot be resolved.
    void AddDependency(const ast::Identifier* from, Symbol to) {
        auto* resolved = scope_stack_.Get(to);
        if (!resolved) {
            switch (to.Type()) {
                case Symbol::BuiltinType::kNone:
                    graph_.resolved_identifiers.Add(from, UnresolvedIdentifier{to.Name()});
                    break;
                case Symbol::BuiltinType::kFunction:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(to.BuiltinValue<builtin::Function>()));
                    break;
                case Symbol::BuiltinType::kBuiltin:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(to.BuiltinValue<builtin::Builtin>()));
                    break;
                case Symbol::BuiltinType::kBuiltinValue:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(to.BuiltinValue<builtin::BuiltinValue>()));
                    break;
                case Symbol::BuiltinType::kAddressSpace:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(to.BuiltinValue<builtin::AddressSpace>()));
                    break;
                case Symbol::BuiltinType::kTexelFormat:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(to.BuiltinValue<builtin::TexelFormat>()));
                    break;
                case Symbol::BuiltinType::kAccess:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(to.BuiltinValue<builtin::Access>()));
                    break;
                case Symbol::BuiltinType::kInterpolationType:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(to.BuiltinValue<builtin::InterpolationType>()));
                    break;
                case Symbol::BuiltinType::kInterpolationSampling:
                    graph_.resolved_identifiers.Add(
                        from,
                        ResolvedIdentifier(to.BuiltinValue<builtin::InterpolationSampling>()));
                    break;
            }
            return;
        }

        if (auto global = globals_.Find(to); global && (*global)->node == resolved) {
            if (dependency_edges_.Add(DependencyEdge{current_global_, *global},
                                      DependencyInfo{from->source})) {
                current_global_->deps.Push(*global);
            }
        }

        graph_.resolved_identifiers.Add(from, ResolvedIdentifier(resolved));
    }

    using VariableMap = utils::Hashmap<Symbol, const ast::Variable*, 32>;
    const GlobalMap& globals_;
    diag::List& diagnostics_;
    DependencyGraph& graph_;
    DependencyEdges& dependency_edges_;

    ScopeStack<Symbol, const ast::Node*> scope_stack_;
    Global* current_global_ = nullptr;
};

/// The global dependency analysis system
struct DependencyAnalysis {
  public:
    /// Constructor
    DependencyAnalysis(diag::List& diagnostics, DependencyGraph& graph)
        : diagnostics_(diagnostics), graph_(graph) {}

    /// Performs global dependency analysis on the module, emitting any errors to
    /// #diagnostics.
    /// @returns true if analysis found no errors, otherwise false.
    bool Run(const ast::Module& module) {
        // Reserve container memory
        graph_.resolved_identifiers.Reserve(module.GlobalDeclarations().Length());
        sorted_.Reserve(module.GlobalDeclarations().Length());

        // Collect all the named globals from the AST module
        GatherGlobals(module);

        // Traverse the named globals to build the dependency graph
        DetermineDependencies();

        // Sort the globals into dependency order
        SortGlobals();

        // Dump the dependency graph if TINT_DUMP_DEPENDENCY_GRAPH is non-zero
        DumpDependencyGraph();

        graph_.ordered_globals = sorted_.Release();

        return !diagnostics_.contains_errors();
    }

  private:
    /// @param node the ast::Node of the global declaration
    /// @returns the symbol of the global declaration node
    /// @note will raise an ICE if the node is not a type, function or variable
    /// declaration
    Symbol SymbolOf(const ast::Node* node) const {
        return Switch(
            node,  //
            [&](const ast::TypeDecl* td) { return td->name->symbol; },
            [&](const ast::Function* func) { return func->name->symbol; },
            [&](const ast::Variable* var) { return var->name->symbol; },
            [&](const ast::DiagnosticDirective*) { return Symbol(); },
            [&](const ast::Enable*) { return Symbol(); },
            [&](const ast::ConstAssert*) { return Symbol(); },
            [&](Default) {
                UnhandledNode(diagnostics_, node);
                return Symbol{};
            });
    }

    /// @param node the ast::Node of the global declaration
    /// @returns the name of the global declaration node
    /// @note will raise an ICE if the node is not a type, function or variable
    /// declaration
    std::string NameOf(const ast::Node* node) const { return SymbolOf(node).Name(); }

    /// @param node the ast::Node of the global declaration
    /// @returns a string representation of the global declaration kind
    /// @note will raise an ICE if the node is not a type, function or variable
    /// declaration
    std::string KindOf(const ast::Node* node) {
        return Switch(
            node,                                                     //
            [&](const ast::Struct*) { return "struct"; },             //
            [&](const ast::Alias*) { return "alias"; },               //
            [&](const ast::Function*) { return "function"; },         //
            [&](const ast::Variable* v) { return v->Kind(); },        //
            [&](const ast::ConstAssert*) { return "const_assert"; },  //
            [&](Default) {
                UnhandledNode(diagnostics_, node);
                return "<error>";
            });
    }

    /// Traverses `module`, collecting all the global declarations and populating
    /// the #globals and #declaration_order fields.
    void GatherGlobals(const ast::Module& module) {
        for (auto* node : module.GlobalDeclarations()) {
            auto* global = allocator_.Create(node);
            if (auto symbol = SymbolOf(node); symbol.IsValid()) {
                globals_.Add(symbol, global);
            }
            declaration_order_.Push(global);
        }
    }

    /// Walks the global declarations, determining the dependencies of each global
    /// and adding these to each global's Global::deps field.
    void DetermineDependencies() {
        DependencyScanner scanner(globals_, diagnostics_, graph_, dependency_edges_);
        for (auto* global : declaration_order_) {
            scanner.Scan(global);
        }
    }

    /// Performs a depth-first traversal of `root`'s dependencies, calling `enter`
    /// as the function decends into each dependency and `exit` when bubbling back
    /// up towards the root.
    /// @param enter is a function with the signature: `bool(Global*)`. The
    /// `enter` function returns true if TraverseDependencies() should traverse
    /// the dependency, otherwise it will be skipped.
    /// @param exit is a function with the signature: `void(Global*)`. The `exit`
    /// function is only called if the corresponding `enter` call returned true.
    template <typename ENTER, typename EXIT>
    void TraverseDependencies(const Global* root, ENTER&& enter, EXIT&& exit) {
        // Entry is a single entry in the traversal stack. Entry points to a
        // dep_idx'th dependency of Entry::global.
        struct Entry {
            const Global* global;  // The parent global
            size_t dep_idx;        // The dependency index in `global->deps`
        };

        if (!enter(root)) {
            return;
        }

        utils::Vector<Entry, 16> stack{Entry{root, 0}};
        while (true) {
            auto& entry = stack.Back();
            // Have we exhausted the dependencies of entry.global?
            if (entry.dep_idx < entry.global->deps.Length()) {
                // No, there's more dependencies to traverse.
                auto& dep = entry.global->deps[entry.dep_idx];
                // Does the caller want to enter this dependency?
                if (enter(dep)) {               // Yes.
                    stack.Push(Entry{dep, 0});  // Enter the dependency.
                } else {
                    entry.dep_idx++;  // No. Skip this node.
                }
            } else {
                // Yes. Time to back up.
                // Exit this global, pop the stack, and if there's another parent node,
                // increment its dependency index, and loop again.
                exit(entry.global);
                stack.Pop();
                if (stack.IsEmpty()) {
                    return;  // All done.
                }
                stack.Back().dep_idx++;
            }
        }
    }

    /// SortGlobals sorts the globals into dependency order, erroring if cyclic
    /// dependencies are found. The sorted dependencies are assigned to #sorted.
    void SortGlobals() {
        if (diagnostics_.contains_errors()) {
            return;  // This code assumes there are no undeclared identifiers.
        }

        // Make sure all directives go before any other global declarations.
        for (auto* global : declaration_order_) {
            if (global->node->IsAnyOf<ast::DiagnosticDirective, ast::Enable>()) {
                sorted_.Add(global->node);
            }
        }

        for (auto* global : declaration_order_) {
            if (global->node->IsAnyOf<ast::DiagnosticDirective, ast::Enable>()) {
                // Skip directives here, as they are already added.
                continue;
            }
            utils::UniqueVector<const Global*, 8> stack;
            TraverseDependencies(
                global,
                [&](const Global* g) {  // Enter
                    if (!stack.Add(g)) {
                        CyclicDependencyFound(g, stack.Release());
                        return false;
                    }
                    if (sorted_.Contains(g->node)) {
                        // Visited this global already.
                        // stack was pushed, but exit() will not be called when we return
                        // false, so pop here.
                        stack.Pop();
                        return false;
                    }
                    return true;
                },
                [&](const Global* g) {  // Exit. Only called if Enter returned true.
                    sorted_.Add(g->node);
                    stack.Pop();
                });

            sorted_.Add(global->node);

            if (TINT_UNLIKELY(!stack.IsEmpty())) {
                // Each stack.push() must have a corresponding stack.pop_back().
                TINT_ICE(Resolver, diagnostics_)
                    << "stack not empty after returning from TraverseDependencies()";
            }
        }
    }

    /// DepInfoFor() looks up the global dependency information for the dependency
    /// of global `from` depending on `to`.
    /// @note will raise an ICE if the edge is not found.
    DependencyInfo DepInfoFor(const Global* from, const Global* to) const {
        auto info = dependency_edges_.Find(DependencyEdge{from, to});
        if (TINT_LIKELY(info)) {
            return *info;
        }
        TINT_ICE(Resolver, diagnostics_)
            << "failed to find dependency info for edge: '" << NameOf(from->node) << "' -> '"
            << NameOf(to->node) << "'";
        return {};
    }

    /// CyclicDependencyFound() emits an error diagnostic for a cyclic dependency.
    /// @param root is the global that starts the cyclic dependency, which must be
    /// found in `stack`.
    /// @param stack is the global dependency stack that contains a loop.
    void CyclicDependencyFound(const Global* root, utils::VectorRef<const Global*> stack) {
        utils::StringStream msg;
        msg << "cyclic dependency found: ";
        constexpr size_t kLoopNotStarted = ~0u;
        size_t loop_start = kLoopNotStarted;
        for (size_t i = 0; i < stack.Length(); i++) {
            auto* e = stack[i];
            if (loop_start == kLoopNotStarted && e == root) {
                loop_start = i;
            }
            if (loop_start != kLoopNotStarted) {
                msg << "'" << NameOf(e->node) << "' -> ";
            }
        }
        msg << "'" << NameOf(root->node) << "'";
        AddError(diagnostics_, msg.str(), root->node->source);
        for (size_t i = loop_start; i < stack.Length(); i++) {
            auto* from = stack[i];
            auto* to = (i + 1 < stack.Length()) ? stack[i + 1] : stack[loop_start];
            auto info = DepInfoFor(from, to);
            AddNote(diagnostics_,
                    KindOf(from->node) + " '" + NameOf(from->node) + "' references " +
                        KindOf(to->node) + " '" + NameOf(to->node) + "' here",
                    info.source);
        }
    }

    void DumpDependencyGraph() {
#if TINT_DUMP_DEPENDENCY_GRAPH == 0
        if ((true)) {
            return;
        }
#endif  // TINT_DUMP_DEPENDENCY_GRAPH
        printf("=========================\n");
        printf("------ declaration ------ \n");
        for (auto* global : declaration_order_) {
            printf("%s\n", NameOf(global->node).c_str());
        }
        printf("------ dependencies ------ \n");
        for (auto* node : sorted_) {
            auto symbol = SymbolOf(node);
            auto* global = *globals_.Find(symbol);
            printf("%s depends on:\n", symbol.Name().c_str());
            for (auto* dep : global->deps) {
                printf("  %s\n", NameOf(dep->node).c_str());
            }
        }
        printf("=========================\n");
    }

    /// Program diagnostics
    diag::List& diagnostics_;

    /// The resulting dependency graph
    DependencyGraph& graph_;

    /// Allocator of Globals
    utils::BlockAllocator<Global> allocator_;

    /// Global map, keyed by name. Populated by GatherGlobals().
    GlobalMap globals_;

    /// Map of DependencyEdge to DependencyInfo. Populated by DetermineDependencies().
    DependencyEdges dependency_edges_;

    /// Globals in declaration order. Populated by GatherGlobals().
    utils::Vector<Global*, 64> declaration_order_;

    /// Globals in sorted dependency order. Populated by SortGlobals().
    utils::UniqueVector<const ast::Node*, 64> sorted_;
};

}  // namespace

DependencyGraph::DependencyGraph() = default;
DependencyGraph::DependencyGraph(DependencyGraph&&) = default;
DependencyGraph::~DependencyGraph() = default;

bool DependencyGraph::Build(const ast::Module& module,
                            diag::List& diagnostics,
                            DependencyGraph& output) {
    DependencyAnalysis da{diagnostics, output};
    return da.Run(module);
}

std::string ResolvedIdentifier::String(diag::List& diagnostics) const {
    if (auto* node = Node()) {
        return Switch(
            node,
            [&](const ast::TypeDecl* n) {  //
                return "type '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Var* n) {  //
                return "var '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Let* n) {  //
                return "let '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Const* n) {  //
                return "const '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Override* n) {  //
                return "override '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Function* n) {  //
                return "function '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Parameter* n) {  //
                return "parameter '" + n->name->symbol.Name() + "'";
            },
            [&](Default) {
                TINT_UNREACHABLE(Resolver, diagnostics)
                    << "unhandled ast::Node: " << node->TypeInfo().name;
                return "<unknown>";
            });
    }
    if (auto builtin_fn = BuiltinFunction(); builtin_fn != builtin::Function::kNone) {
        return "builtin function '" + utils::ToString(builtin_fn) + "'";
    }
    if (auto builtin_ty = BuiltinType(); builtin_ty != builtin::Builtin::kUndefined) {
        return "builtin type '" + utils::ToString(builtin_ty) + "'";
    }
    if (auto builtin_val = BuiltinValue(); builtin_val != builtin::BuiltinValue::kUndefined) {
        return "builtin value '" + utils::ToString(builtin_val) + "'";
    }
    if (auto access = Access(); access != builtin::Access::kUndefined) {
        return "access '" + utils::ToString(access) + "'";
    }
    if (auto addr = AddressSpace(); addr != builtin::AddressSpace::kUndefined) {
        return "address space '" + utils::ToString(addr) + "'";
    }
    if (auto type = InterpolationType(); type != builtin::InterpolationType::kUndefined) {
        return "interpolation type '" + utils::ToString(type) + "'";
    }
    if (auto smpl = InterpolationSampling(); smpl != builtin::InterpolationSampling::kUndefined) {
        return "interpolation sampling '" + utils::ToString(smpl) + "'";
    }
    if (auto fmt = TexelFormat(); fmt != builtin::TexelFormat::kUndefined) {
        return "texel format '" + utils::ToString(fmt) + "'";
    }
    if (auto* unresolved = Unresolved()) {
        return "unresolved identifier '" + unresolved->name + "'";
    }

    TINT_UNREACHABLE(Resolver, diagnostics) << "unhandled ResolvedIdentifier";
    return "<unknown>";
}

}  // namespace tint::resolver
