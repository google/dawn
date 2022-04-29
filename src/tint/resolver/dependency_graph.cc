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
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/fallthrough_statement.h"
#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/scope_stack.h"
#include "src/tint/sem/builtin.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/scoped_assignment.h"
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
    Switch(
        global->node,
        [&](const ast::Struct* str) {
          Declare(str->name, str);
          for (auto* member : str->members) {
            TraverseType(member->type);
          }
        },
        [&](const ast::Alias* alias) {
          Declare(alias->name, alias);
          TraverseType(alias->type);
        },
        [&](const ast::Function* func) {
          Declare(func->symbol, func);
          TraverseAttributes(func->attributes);
          TraverseFunction(func);
        },
        [&](const ast::Variable* var) {
          Declare(var->symbol, var);
          TraverseType(var->type);
          if (var->constructor) {
            TraverseExpression(var->constructor);
          }
        },
        [&](const ast::Enable*) {
          // Enable directives do not effect the dependency graph.
        },
        [&](Default) { UnhandledNode(diagnostics_, global->node); });
  }

 private:
  /// Traverses the function, performing symbol resolution and determining
  /// global dependencies.
  void TraverseFunction(const ast::Function* func) {
    // Perform symbol resolution on all the parameter types before registering
    // the parameters themselves. This allows the case of declaring a parameter
    // with the same identifier as its type.
    for (auto* param : func->params) {
      TraverseType(param->type);
    }
    // Resolve the return type
    TraverseType(func->return_type);

    // Push the scope stack for the parameters and function body.
    scope_stack_.Push();
    TINT_DEFER(scope_stack_.Pop());

    for (auto* param : func->params) {
      if (auto* shadows = scope_stack_.Get(param->symbol)) {
        graph_.shadows.emplace(param, shadows);
      }
      Declare(param->symbol, param);
    }
    if (func->body) {
      TraverseStatements(func->body->statements);
    }
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
        [&](const ast::CallStatement* r) {  //
          TraverseExpression(r->expr);
        },
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
        [&](const ast::IncrementDecrementStatement* i) {
          TraverseExpression(i->lhs);
        },
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
        [&](const ast::ReturnStatement* r) {  //
          TraverseExpression(r->value);
        },
        [&](const ast::SwitchStatement* s) {
          TraverseExpression(s->condition);
          for (auto* c : s->body) {
            for (auto* sel : c->selectors) {
              TraverseExpression(sel);
            }
            TraverseStatement(c->body);
          }
        },
        [&](const ast::VariableDeclStatement* v) {
          if (auto* shadows = scope_stack_.Get(v->variable->symbol)) {
            graph_.shadows.emplace(v->variable, shadows);
          }
          TraverseType(v->variable->type);
          TraverseExpression(v->variable->constructor);
          Declare(v->variable->symbol, v->variable);
        },
        [&](Default) {
          if (!stmt->IsAnyOf<ast::BreakStatement, ast::ContinueStatement,
                             ast::DiscardStatement,
                             ast::FallthroughStatement>()) {
            UnhandledNode(diagnostics_, stmt);
          }
        });
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
          Switch(
              expr,
              [&](const ast::IdentifierExpression* ident) {
                AddDependency(ident, ident->symbol, "identifier", "references");
              },
              [&](const ast::CallExpression* call) {
                if (call->target.name) {
                  AddDependency(call->target.name, call->target.name->symbol,
                                "function", "calls");
                }
                if (call->target.type) {
                  TraverseType(call->target.type);
                }
              },
              [&](const ast::BitcastExpression* cast) {
                TraverseType(cast->type);
              });
          return ast::TraverseAction::Descend;
        });
  }

  /// Traverses the type node, performing symbol resolution and determining
  /// global dependencies.
  void TraverseType(const ast::Type* ty) {
    if (!ty) {
      return;
    }
    Switch(
        ty,  //
        [&](const ast::Array* arr) {
          TraverseType(arr->type);  //
          TraverseExpression(arr->count);
        },
        [&](const ast::Atomic* atomic) {  //
          TraverseType(atomic->type);
        },
        [&](const ast::Matrix* mat) {  //
          TraverseType(mat->type);
        },
        [&](const ast::Pointer* ptr) {  //
          TraverseType(ptr->type);
        },
        [&](const ast::TypeName* tn) {  //
          AddDependency(tn, tn->name, "type", "references");
        },
        [&](const ast::Vector* vec) {  //
          TraverseType(vec->type);
        },
        [&](const ast::SampledTexture* tex) {  //
          TraverseType(tex->type);
        },
        [&](const ast::MultisampledTexture* tex) {  //
          TraverseType(tex->type);
        },
        [&](Default) {
          if (!ty->IsAnyOf<ast::Void, ast::Bool, ast::I32, ast::U32, ast::F32,
                           ast::DepthTexture, ast::DepthMultisampledTexture,
                           ast::StorageTexture, ast::ExternalTexture,
                           ast::Sampler>()) {
            UnhandledNode(diagnostics_, ty);
          }
        });
  }

  /// Traverses the attribute list, performing symbol resolution and
  /// determining global dependencies.
  void TraverseAttributes(const ast::AttributeList& attrs) {
    for (auto* attr : attrs) {
      TraverseAttribute(attr);
    }
  }

  /// Traverses the attribute, performing symbol resolution and determining
  /// global dependencies.
  void TraverseAttribute(const ast::Attribute* attr) {
    if (auto* wg = attr->As<ast::WorkgroupAttribute>()) {
      TraverseExpression(wg->x);
      TraverseExpression(wg->y);
      TraverseExpression(wg->z);
      return;
    }
    if (attr->IsAnyOf<
            ast::BindingAttribute, ast::BuiltinAttribute, ast::GroupAttribute,
            ast::IdAttribute, ast::InternalAttribute, ast::InterpolateAttribute,
            ast::InvariantAttribute, ast::LocationAttribute,
            ast::StageAttribute, ast::StrideAttribute,
            ast::StructMemberAlignAttribute, ast::StructMemberOffsetAttribute,
            ast::StructMemberSizeAttribute>()) {
      return;
    }

    UnhandledNode(diagnostics_, attr);
  }

  /// Adds the dependency from `from` to `to`, erroring if `to` cannot be
  /// resolved.
  void AddDependency(const ast::Node* from,
                     Symbol to,
                     const char* use,
                     const char* action) {
    auto* resolved = scope_stack_.Get(to);
    if (!resolved) {
      if (!IsBuiltin(to)) {
        UnknownSymbol(to, from->source, use);
        return;
      }
    }

    if (auto* global = utils::Lookup(globals_, to);
        global && global->node == resolved) {
      if (dependency_edges_
              .emplace(DependencyEdge{current_global_, global},
                       DependencyInfo{from->source, action})
              .second) {
        current_global_->deps.emplace_back(global);
      }
    }

    graph_.resolved_symbols.emplace(from, resolved);
  }

  /// @returns true if `name` is the name of a builtin function
  bool IsBuiltin(Symbol name) const {
    return sem::ParseBuiltinType(symbols_.NameFor(name)) !=
           sem::BuiltinType::kNone;
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
  bool Run(const ast::Module& module) {
    // Collect all the named globals from the AST module
    GatherGlobals(module);

    // Traverse the named globals to build the dependency graph
    DetermineDependencies();

    // Sort the globals into dependency order
    SortGlobals();

    // Dump the dependency graph if TINT_DUMP_DEPENDENCY_GRAPH is non-zero
    DumpDependencyGraph();

    graph_.ordered_globals = std::move(sorted_);

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
        [&](const ast::TypeDecl* td) { return td->name; },
        [&](const ast::Function* func) { return func->symbol; },
        [&](const ast::Variable* var) { return var->symbol; },
        [&](Default) {
          UnhandledNode(diagnostics_, node);
          return Symbol{};
        });
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
    return Switch(
        node,  //
        [&](const ast::Struct*) { return "struct"; },
        [&](const ast::Alias*) { return "alias"; },
        [&](const ast::Function*) { return "function"; },
        [&](const ast::Variable* var) { return var->is_const ? "let" : "var"; },
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
      // Enable directives do not form a symbol. Skip them.
      if (!node->Is<ast::Enable>()) {
        globals_.emplace(SymbolOf(node), global);
      }
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
  utils::BlockAllocator<Global> allocator_;

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
                            DependencyGraph& output) {
  DependencyAnalysis da{symbols, diagnostics, output};
  return da.Run(module);
}

}  // namespace tint::resolver
