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

#include "src/resolver/dependency_graph.h"

#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/traverse_expressions.h"
#include "src/scope_stack.h"
#include "src/sem/intrinsic.h"
#include "src/utils/defer.h"
#include "src/utils/map.h"
#include "src/utils/scoped_assignment.h"
#include "src/utils/unique_vector.h"

#define TINT_DUMP_DEPENDENCY_GRAPH 0

namespace tint {
namespace resolver {
namespace {

// Forward declaration
struct Global;

/// Dependency describes how one global depends on another global
struct DependencyInfo {
  /// The source of the symbol that forms the dependency
  Source source;
  /// A string describing how the dependency is referenced. e.g. 'calls'
  const char* action = nullptr;
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
using DependencyEdges = std::unordered_map<DependencyEdge,
                                           DependencyInfo,
                                           DependencyEdgeCmp,
                                           DependencyEdgeCmp>;

/// Global describes a module-scope variable, type or function.
struct Global {
  explicit Global(const ast::Node* n) : node(n) {}

  /// The declaration ast::Node
  const ast::Node* node;
  /// A list of dependencies that this global depends on
  std::vector<Global*> deps;
};

/// A map of global name to Global
using GlobalMap = std::unordered_map<Symbol, Global*>;

/// Raises an ICE that a global ast::Node type was not handled by this system.
void UnhandledNode(diag::List& diagnostics, const ast::Node* node) {
  TINT_ICE(Resolver, diagnostics)
      << "unhandled node type: " << node->TypeInfo().name;
}

/// Raises an error diagnostic with the given message and source.
void AddError(diag::List& diagnostics,
              const std::string& msg,
              const Source& source) {
  diagnostics.add_error(diag::System::Resolver, msg, source);
}

/// Raises a note diagnostic with the given message and source.
void AddNote(diag::List& diagnostics,
             const std::string& msg,
             const Source& source) {
  diagnostics.add_note(diag::System::Resolver, msg, source);
}

/// DependencyScanner is used to traverse a module to build the list of
/// global-to-global dependencies.
class DependencyScanner {
 public:
  /// Constructor
  /// @param syms the program symbol table
  /// @param globals_by_name map of global symbol to Global pointer
  /// @param diagnostics diagnostic messages, appended with any errors found
  /// @param graph the dependency graph to populate with resolved symbols
  /// @param edges the map of globals-to-global dependency edges, which will
  /// be populated by calls to Scan()
  DependencyScanner(const SymbolTable& syms,
                    const GlobalMap& globals_by_name,
                    diag::List& diagnostics,
                    DependencyGraph& graph,
                    DependencyEdges& edges)
      : symbols_(syms),
        globals_(globals_by_name),
        diagnostics_(diagnostics),
        graph_(graph),
        dependency_edges_(edges) {
    // Register all the globals at global-scope
    for (auto it : globals_by_name) {
      scope_stack_.Set(it.first, it.second->node);
    }
  }

  /// Walks the global declarations, resolving symbols, and determining the
  /// dependencies of each global.
  void Scan(Global* global) {
    TINT_SCOPED_ASSIGNMENT(current_global_, global);

    if (auto* str = global->node->As<ast::Struct>()) {
      Declare(str->name, str);
      for (auto* member : str->members) {
        TraverseType(member->type);
      }
      return;
    }
    if (auto* alias = global->node->As<ast::Alias>()) {
      Declare(alias->name, alias);
      TraverseType(alias->type);
      return;
    }
    if (auto* func = global->node->As<ast::Function>()) {
      Declare(func->symbol, func);
      TraverseDecorations(func->decorations);
      TraverseFunction(func);
      return;
    }
    if (auto* var = global->node->As<ast::Variable>()) {
      Declare(var->symbol, var);
      TraverseType(var->type);
      if (var->constructor) {
        TraverseExpression(var->constructor);
      }
      return;
    }
    UnhandledNode(diagnostics_, global->node);
  }

 private:
  /// Traverses the function, performing symbol resolution and determining
  /// global dependencies.
  void TraverseFunction(const ast::Function* func) {
    scope_stack_.Push();
    TINT_DEFER(scope_stack_.Pop());

    for (auto* param : func->params) {
      if (auto* shadows = scope_stack_.Get(param->symbol)) {
        graph_.shadows.emplace(param, shadows);
      }
      Declare(param->symbol, param);
      TraverseType(param->type);
    }
    if (func->body) {
      TraverseStatements(func->body->statements);
    }
    TraverseType(func->return_type);
  }

  /// Traverses the statements, performing symbol resolution and determining
  /// global dependencies.
  void TraverseStatements(const ast::StatementList& stmts) {
    for (auto* s : stmts) {
      TraverseStatement(s);
    }
  }

  /// Traverses the statement, performing symbol resolution and determining
  /// global dependencies.
  void TraverseStatement(const ast::Statement* stmt) {
    if (stmt == nullptr) {
      return;
    }
    if (auto* b = stmt->As<ast::AssignmentStatement>()) {
      TraverseExpression(b->lhs);
      TraverseExpression(b->rhs);
      return;
    }
    if (auto* b = stmt->As<ast::BlockStatement>()) {
      scope_stack_.Push();
      TINT_DEFER(scope_stack_.Pop());
      TraverseStatements(b->statements);
      return;
    }
    if (auto* r = stmt->As<ast::CallStatement>()) {
      TraverseExpression(r->expr);
      return;
    }
    if (auto* l = stmt->As<ast::ForLoopStatement>()) {
      scope_stack_.Push();
      TINT_DEFER(scope_stack_.Pop());
      TraverseStatement(l->initializer);
      TraverseExpression(l->condition);
      TraverseStatement(l->continuing);
      TraverseStatement(l->body);
      return;
    }
    if (auto* l = stmt->As<ast::LoopStatement>()) {
      scope_stack_.Push();
      TINT_DEFER(scope_stack_.Pop());
      TraverseStatements(l->body->statements);
      TraverseStatement(l->continuing);
      return;
    }
    if (auto* i = stmt->As<ast::IfStatement>()) {
      TraverseExpression(i->condition);
      TraverseStatement(i->body);
      for (auto* e : i->else_statements) {
        TraverseExpression(e->condition);
        TraverseStatement(e->body);
      }
      return;
    }
    if (auto* r = stmt->As<ast::ReturnStatement>()) {
      TraverseExpression(r->value);
      return;
    }
    if (auto* s = stmt->As<ast::SwitchStatement>()) {
      TraverseExpression(s->condition);
      for (auto* c : s->body) {
        for (auto* sel : c->selectors) {
          TraverseExpression(sel);
        }
        TraverseStatement(c->body);
      }
      return;
    }
    if (auto* v = stmt->As<ast::VariableDeclStatement>()) {
      if (auto* shadows = scope_stack_.Get(v->variable->symbol)) {
        graph_.shadows.emplace(v->variable, shadows);
      }
      TraverseType(v->variable->type);
      TraverseExpression(v->variable->constructor);
      Declare(v->variable->symbol, v->variable);
      return;
    }
    if (stmt->IsAnyOf<ast::BreakStatement, ast::ContinueStatement,
                      ast::DiscardStatement, ast::FallthroughStatement>()) {
      return;
    }

    UnhandledNode(diagnostics_, stmt);
  }

  /// Adds the symbol definition to the current scope, raising an error if two
  /// symbols collide within the same scope.
  void Declare(Symbol symbol, const ast::Node* node) {
    auto* old = scope_stack_.Set(symbol, node);
    if (old != nullptr && node != old) {
      auto name = symbols_.NameFor(symbol);
      AddError(diagnostics_, "redeclaration of '" + name + "'", node->source);
      AddNote(diagnostics_, "'" + name + "' previously declared here",
              old->source);
    }
  }

  /// Traverses the expression, performing symbol resolution and determining
  /// global dependencies.
  void TraverseExpression(const ast::Expression* root) {
    if (!root) {
      return;
    }
    ast::TraverseExpressions(
        root, diagnostics_, [&](const ast::Expression* expr) {
          if (auto* ident = expr->As<ast::IdentifierExpression>()) {
            auto* node = scope_stack_.Get(ident->symbol);
            if (node == nullptr) {
              if (!IsIntrinsic(ident->symbol)) {
                UnknownSymbol(ident->symbol, ident->source, "identifier");
              }
              return ast::TraverseAction::Descend;
            }
            auto global_it = globals_.find(ident->symbol);
            if (global_it != globals_.end() &&
                node == global_it->second->node) {
              AddGlobalDependency(ident, ident->symbol, "identifier",
                                  "references");
            } else {
              graph_.resolved_symbols.emplace(ident, node);
            }
          }
          if (auto* call = expr->As<ast::CallExpression>()) {
            if (call->target.name) {
              if (!IsIntrinsic(call->target.name->symbol)) {
                AddGlobalDependency(call->target.name,
                                    call->target.name->symbol, "function",
                                    "calls");
                graph_.resolved_symbols.emplace(
                    call,
                    utils::Lookup(graph_.resolved_symbols, call->target.name));
              }
            }
            if (call->target.type) {
              TraverseType(call->target.type);
              graph_.resolved_symbols.emplace(
                  call,
                  utils::Lookup(graph_.resolved_symbols, call->target.type));
            }
          }
          if (auto* cast = expr->As<ast::BitcastExpression>()) {
            TraverseType(cast->type);
          }
          return ast::TraverseAction::Descend;
        });
  }

  /// Traverses the type node, performing symbol resolution and determining
  /// global dependencies.
  void TraverseType(const ast::Type* ty) {
    if (ty == nullptr) {
      return;
    }
    if (auto* arr = ty->As<ast::Array>()) {
      TraverseType(arr->type);
      TraverseExpression(arr->count);
      return;
    }
    if (auto* atomic = ty->As<ast::Atomic>()) {
      TraverseType(atomic->type);
      return;
    }
    if (auto* mat = ty->As<ast::Matrix>()) {
      TraverseType(mat->type);
      return;
    }
    if (auto* ptr = ty->As<ast::Pointer>()) {
      TraverseType(ptr->type);
      return;
    }
    if (auto* tn = ty->As<ast::TypeName>()) {
      AddGlobalDependency(tn, tn->name, "type", "references");
      return;
    }
    if (auto* vec = ty->As<ast::Vector>()) {
      TraverseType(vec->type);
      return;
    }
    if (auto* tex = ty->As<ast::SampledTexture>()) {
      TraverseType(tex->type);
      return;
    }
    if (auto* tex = ty->As<ast::MultisampledTexture>()) {
      TraverseType(tex->type);
      return;
    }
    if (ty->IsAnyOf<ast::Void, ast::Bool, ast::I32, ast::U32, ast::F32,
                    ast::DepthTexture, ast::DepthMultisampledTexture,
                    ast::StorageTexture, ast::ExternalTexture,
                    ast::Sampler>()) {
      return;
    }

    UnhandledNode(diagnostics_, ty);
  }

  /// Traverses the decoration list, performing symbol resolution and
  /// determining global dependencies.
  void TraverseDecorations(const ast::DecorationList& decos) {
    for (auto* deco : decos) {
      TraverseDecoration(deco);
    }
  }

  /// Traverses the decoration, performing symbol resolution and determining
  /// global dependencies.
  void TraverseDecoration(const ast::Decoration* deco) {
    if (auto* wg = deco->As<ast::WorkgroupDecoration>()) {
      TraverseExpression(wg->x);
      TraverseExpression(wg->y);
      TraverseExpression(wg->z);
      return;
    }
    if (deco->IsAnyOf<ast::BindingDecoration, ast::BuiltinDecoration,
                      ast::GroupDecoration, ast::InternalDecoration,
                      ast::InterpolateDecoration, ast::InvariantDecoration,
                      ast::LocationDecoration, ast::OverrideDecoration,
                      ast::StageDecoration, ast::StrideDecoration,
                      ast::StructBlockDecoration,
                      ast::StructMemberAlignDecoration,
                      ast::StructMemberOffsetDecoration,
                      ast::StructMemberSizeDecoration>()) {
      return;
    }

    UnhandledNode(diagnostics_, deco);
  }

  /// Adds the dependency to the currently processed global
  void AddGlobalDependency(const ast::Node* from,
                           Symbol to,
                           const char* use,
                           const char* action) {
    auto global_it = globals_.find(to);
    if (global_it != globals_.end()) {
      auto* global = global_it->second;
      if (dependency_edges_
              .emplace(DependencyEdge{current_global_, global},
                       DependencyInfo{from->source, action})
              .second) {
        current_global_->deps.emplace_back(global);
      }
      graph_.resolved_symbols.emplace(from, global->node);
    } else {
      UnknownSymbol(to, from->source, use);
    }
  }

  /// @returns true if `name` is the name of an intrinsic function
  bool IsIntrinsic(Symbol name) const {
    return sem::ParseIntrinsicType(symbols_.NameFor(name)) !=
           sem::IntrinsicType::kNone;
  }

  /// Appends an error to the diagnostics that the given symbol cannot be
  /// resolved.
  void UnknownSymbol(Symbol name, Source source, const char* use) {
    AddError(
        diagnostics_,
        "unknown " + std::string(use) + ": '" + symbols_.NameFor(name) + "'",
        source);
  }

  using VariableMap = std::unordered_map<Symbol, const ast::Variable*>;
  const SymbolTable& symbols_;
  const GlobalMap& globals_;
  diag::List& diagnostics_;
  DependencyGraph& graph_;
  DependencyEdges& dependency_edges_;

  ScopeStack<const ast::Node*> scope_stack_;
  Global* current_global_ = nullptr;
};

/// The global dependency analysis system
struct DependencyAnalysis {
 public:
  /// Constructor
  DependencyAnalysis(const SymbolTable& symbols,
                     diag::List& diagnostics,
                     DependencyGraph& graph)
      : symbols_(symbols), diagnostics_(diagnostics), graph_(graph) {}

  /// Performs global dependency analysis on the module, emitting any errors to
  /// #diagnostics.
  /// @returns true if analysis found no errors, otherwise false.
  bool Run(const ast::Module& module, bool allow_out_of_order_decls) {
    // Collect all the named globals from the AST module
    GatherGlobals(module);

    // Traverse the named globals to build the dependency graph
    DetermineDependencies();

    // Sort the globals into dependency order
    SortGlobals();

    // Dump the dependency graph if TINT_DUMP_DEPENDENCY_GRAPH is non-zero
    DumpDependencyGraph();

    if (!allow_out_of_order_decls) {
      // Prevent out-of-order declarations.
      ErrorOnOutOfOrderDeclarations();
    }

    graph_.ordered_globals = std::move(sorted_);

    return !diagnostics_.contains_errors();
  }

 private:
  /// @param node the ast::Node of the global declaration
  /// @returns the symbol of the global declaration node
  /// @note will raise an ICE if the node is not a type, function or variable
  /// declaration
  Symbol SymbolOf(const ast::Node* node) const {
    if (auto* td = node->As<ast::TypeDecl>()) {
      return td->name;
    }
    if (auto* func = node->As<ast::Function>()) {
      return func->symbol;
    }
    if (auto* var = node->As<ast::Variable>()) {
      return var->symbol;
    }
    UnhandledNode(diagnostics_, node);
    return {};
  }

  /// @param node the ast::Node of the global declaration
  /// @returns the name of the global declaration node
  /// @note will raise an ICE if the node is not a type, function or variable
  /// declaration
  std::string NameOf(const ast::Node* node) const {
    return symbols_.NameFor(SymbolOf(node));
  }

  /// @param node the ast::Node of the global declaration
  /// @returns a string representation of the global declaration kind
  /// @note will raise an ICE if the node is not a type, function or variable
  /// declaration
  std::string KindOf(const ast::Node* node) {
    if (node->Is<ast::Struct>()) {
      return "struct";
    }
    if (node->Is<ast::Alias>()) {
      return "alias";
    }
    if (node->Is<ast::Function>()) {
      return "function";
    }
    if (auto* var = node->As<ast::Variable>()) {
      return var->is_const ? "let" : "var";
    }
    UnhandledNode(diagnostics_, node);
    return {};
  }

  /// Traverses `module`, collecting all the global declarations and populating
  /// the #globals and #declaration_order fields.
  void GatherGlobals(const ast::Module& module) {
    for (auto* node : module.GlobalDeclarations()) {
      auto* global = allocator_.Create(node);
      globals_.emplace(SymbolOf(node), global);
      declaration_order_.emplace_back(global);
    }
  }

  /// Walks the global declarations, determining the dependencies of each global
  /// and adding these to each global's Global::deps field.
  void DetermineDependencies() {
    DependencyScanner scanner(symbols_, globals_, diagnostics_, graph_,
                              dependency_edges_);
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

    std::vector<Entry> stack{Entry{root, 0}};
    while (true) {
      auto& entry = stack.back();
      // Have we exhausted the dependencies of entry.global?
      if (entry.dep_idx < entry.global->deps.size()) {
        // No, there's more dependencies to traverse.
        auto& dep = entry.global->deps[entry.dep_idx];
        // Does the caller want to enter this dependency?
        if (enter(dep)) {                  // Yes.
          stack.push_back(Entry{dep, 0});  // Enter the dependency.
        } else {
          entry.dep_idx++;  // No. Skip this node.
        }
      } else {
        // Yes. Time to back up.
        // Exit this global, pop the stack, and if there's another parent node,
        // increment its dependency index, and loop again.
        exit(entry.global);
        stack.pop_back();
        if (stack.empty()) {
          return;  // All done.
        }
        stack.back().dep_idx++;
      }
    }
  }

  /// SortGlobals sorts the globals into dependency order, erroring if cyclic
  /// dependencies are found. The sorted dependencies are assigned to #sorted.
  void SortGlobals() {
    if (diagnostics_.contains_errors()) {
      return;  // This code assumes there are no undeclared identifiers.
    }

    std::unordered_set<const Global*> visited;
    for (auto* global : declaration_order_) {
      utils::UniqueVector<const Global*> stack;
      TraverseDependencies(
          global,
          [&](const Global* g) {  // Enter
            if (!stack.add(g)) {
              CyclicDependencyFound(g, stack);
              return false;
            }
            if (sorted_.contains(g->node)) {
              // Visited this global already.
              // stack was pushed, but exit() will not be called when we return
              // false, so pop here.
              stack.pop_back();
              return false;
            }
            return true;
          },
          [&](const Global* g) {  // Exit. Only called if Enter returned true.
            sorted_.add(g->node);
            stack.pop_back();
          });

      sorted_.add(global->node);

      if (!stack.empty()) {
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
    auto it = dependency_edges_.find(DependencyEdge{from, to});
    if (it != dependency_edges_.end()) {
      return it->second;
    }
    TINT_ICE(Resolver, diagnostics_)
        << "failed to find dependency info for edge: '" << NameOf(from->node)
        << "' -> '" << NameOf(to->node) << "'";
    return {};
  }

  // TODO(crbug.com/tint/1266): Errors if there are any uses of globals before
  // their declaration. Out-of-order declarations was added to the WGSL
  // specification with https://github.com/gpuweb/gpuweb/pull/2244, but Mozilla
  // have objections to this change so this feature is currently disabled via
  // this function.
  void ErrorOnOutOfOrderDeclarations() {
    if (diagnostics_.contains_errors()) {
      // Might have already errored about cyclic dependencies. No need to report
      // out-of-order errors as well.
      return;
    }
    std::unordered_set<const Global*> seen;
    for (auto* global : declaration_order_) {
      for (auto* dep : global->deps) {
        if (!seen.count(dep)) {
          auto info = DepInfoFor(global, dep);
          auto name = NameOf(dep->node);
          AddError(diagnostics_,
                   KindOf(dep->node) + " '" + name +
                       "' used before it has been declared",
                   info.source);
          AddNote(diagnostics_,
                  KindOf(dep->node) + " '" + name + "' declared here",
                  dep->node->source);
        }
      }
      seen.emplace(global);
    }
  }

  /// CyclicDependencyFound() emits an error diagnostic for a cyclic dependency.
  /// @param root is the global that starts the cyclic dependency, which must be
  /// found in `stack`.
  /// @param stack is the global dependency stack that contains a loop.
  void CyclicDependencyFound(const Global* root,
                             const std::vector<const Global*>& stack) {
    std::stringstream msg;
    msg << "cyclic dependency found: ";
    constexpr size_t kLoopNotStarted = ~0u;
    size_t loop_start = kLoopNotStarted;
    for (size_t i = 0; i < stack.size(); i++) {
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
    for (size_t i = loop_start; i < stack.size(); i++) {
      auto* from = stack[i];
      auto* to = (i + 1 < stack.size()) ? stack[i + 1] : stack[loop_start];
      auto info = DepInfoFor(from, to);
      AddNote(diagnostics_,
              KindOf(from->node) + " '" + NameOf(from->node) + "' " +
                  info.action + " " + KindOf(to->node) + " '" +
                  NameOf(to->node) + "' here",
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
      auto* global = globals_.at(symbol);
      printf("%s depends on:\n", symbols_.NameFor(symbol).c_str());
      for (auto* dep : global->deps) {
        printf("  %s\n", NameOf(dep->node).c_str());
      }
    }
    printf("=========================\n");
  }

  /// Program symbols
  const SymbolTable& symbols_;

  /// Program diagnostics
  diag::List& diagnostics_;

  /// The resulting dependency graph
  DependencyGraph& graph_;

  /// Allocator of Globals
  BlockAllocator<Global> allocator_;

  /// Global map, keyed by name. Populated by GatherGlobals().
  GlobalMap globals_;

  /// Map of DependencyEdge to DependencyInfo. Populated by
  /// DetermineDependencies().
  DependencyEdges dependency_edges_;

  /// Globals in declaration order. Populated by GatherGlobals().
  std::vector<Global*> declaration_order_;

  /// Globals in sorted dependency order. Populated by SortGlobals().
  utils::UniqueVector<const ast::Node*> sorted_;
};

}  // namespace

DependencyGraph::DependencyGraph() = default;
DependencyGraph::DependencyGraph(DependencyGraph&&) = default;
DependencyGraph::~DependencyGraph() = default;

bool DependencyGraph::Build(const ast::Module& module,
                            const SymbolTable& symbols,
                            diag::List& diagnostics,
                            DependencyGraph& output,
                            bool allow_out_of_order_decls) {
  DependencyAnalysis da{symbols, diagnostics, output};
  return da.Run(module, allow_out_of_order_decls);
}

}  // namespace resolver
}  // namespace tint
