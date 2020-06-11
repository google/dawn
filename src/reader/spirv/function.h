// Copyright 2020 The Tint Authors.
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

#ifndef SRC_READER_SPIRV_FUNCTION_H_
#define SRC_READER_SPIRV_FUNCTION_H_

#include <functional>
#include <memory>
#include <ostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "source/opt/basic_block.h"
#include "source/opt/constants.h"
#include "source/opt/function.h"
#include "source/opt/instruction.h"
#include "source/opt/ir_context.h"
#include "source/opt/type_manager.h"
#include "src/ast/case_statement.h"
#include "src/ast/expression.h"
#include "src/ast/module.h"
#include "src/ast/statement.h"
#include "src/reader/spirv/construct.h"
#include "src/reader/spirv/fail_stream.h"
#include "src/reader/spirv/namer.h"
#include "src/reader/spirv/parser_impl.h"

namespace tint {
namespace reader {
namespace spirv {

/// Kinds of CFG edges.
//
// The edge kinds are used in many ways.
//
// For example, consider the edges leaving a basic block and going to distinct
// targets. If the total number of kForward + kIfBreak + kCaseFallThrough edges
// is more than 1, then the block must be a structured header, i.e. it needs
// a merge instruction to declare the control flow divergence and associated
// reconvergence point.  Those those edge kinds count toward divergence
// because SPIR-v is designed to easily map back to structured control flow
// in GLSL (and C).  In GLSL and C, those forward-flow edges don't have a
// special statement to express them.  The other forward edges: kSwitchBreak,
// kLoopBreak, and kLoopContinue directly map to 'break', 'break', and
// 'continue', respectively.
enum class EdgeKind {
  // A back-edge: An edge from a node to one of its ancestors in a depth-first
  // search from the entry block.
  kBack,
  // An edge from a node to the merge block of the nearest enclosing switch,
  // where there is no intervening loop.
  kSwitchBreak,
  // An edge from a node to the merge block of the nearest enclosing loop, where
  // there is no intervening switch.
  // The source block is a "break block" as defined by SPIR-V.
  kLoopBreak,
  // An edge from a node in a loop body to the associated continue target, where
  // there are no other intervening loops or switches.
  // The source block is a "continue block" as defined by SPIR-V.
  kLoopContinue,
  // An edge from a node to the merge block of the nearest enclosing structured
  // construct, but which is neither a kSwitchBreak or a kLoopBreak.
  // This can only occur for an "if" selection, i.e. where the selection
  // header ends in OpBranchConditional.
  kIfBreak,
  // An edge from one switch case to the next sibling switch case.
  kCaseFallThrough,
  // None of the above.
  kForward
};

/// Bookkeeping info for a basic block.
struct BlockInfo {
  /// Constructor
  /// @param bb internal representation of the basic block
  explicit BlockInfo(const spvtools::opt::BasicBlock& bb);
  ~BlockInfo();

  /// The internal representation of the basic block.
  const spvtools::opt::BasicBlock* basic_block;

  /// The ID of the OpLabel instruction that starts this block.
  uint32_t id = 0;

  /// The position of this block in the reverse structured post-order.
  uint32_t pos = 0;

  /// If this block is a header, then this is the ID of the merge block.
  uint32_t merge_for_header = 0;
  /// If this block is a loop header, then this is the ID of the continue
  /// target.
  uint32_t continue_for_header = 0;
  /// If this block is a merge, then this is the ID of the header.
  uint32_t header_for_merge = 0;
  /// If this block is a continue target, then this is the ID of the loop
  /// header.
  uint32_t header_for_continue = 0;
  /// Is this block a single-block loop: A loop header that declares itself
  /// as its own continue target, and has branch to itself.
  bool is_single_block_loop = false;

  /// The immediately enclosing structured construct.
  const Construct* construct = nullptr;

  /// Maps the ID of a successor block (in the CFG) to its edge classification.
  std::unordered_map<uint32_t, EdgeKind> succ_edge;

  /// The following fields record relationships among blocks in a selection
  /// construct for an OpSwitch instruction.

  /// If not null, then the pointed-at construct is a selection for an OpSwitch,
  /// and this block is a case target for it.  We say this block "heads" the
  /// case construct.
  const Construct* case_head_for = nullptr;
  /// If not null, then the pointed-at construct is a selection for an OpSwitch,
  /// and this block is the default target for it.  We say this block "heads"
  /// the default case construct.
  const Construct* default_head_for = nullptr;
  /// Is this a default target for a switch, and is it also the merge for its
  /// switch?
  bool default_is_merge = false;
  /// The list of switch values that cause a branch to this block.
  std::unique_ptr<std::vector<uint64_t>> case_values;

  /// The following fields record relationships among blocks in a selection
  /// construct for an OpBranchConditional instruction.

  /// If not null, then the pointed-at construct is a selection for an
  /// OpBranchConditional and this block is the "true" target for it.  We say
  /// this block "heads" the true case.
  const Construct* true_head_for = nullptr;
  /// If not null, then the pointed-at construct is a selection for an
  /// OpBranchConditional and this block is the "false" target for it.  We say
  /// this block "heads" the false case.
  const Construct* false_head_for = nullptr;
  /// If not null, then the pointed-at construct is the first block at which
  /// control reconverges between the "then" and "else" clauses, but before
  /// the merge block for that selection.
  const Construct* premerge_head_for = nullptr;
  /// If not null, then this block is an if-selection header, and |true_head| is
  /// the target of the true branch on the OpBranchConditional.
  /// In particular, true_head->true_head_for == this
  const BlockInfo* true_head = nullptr;
  /// If not null, then this block is an if-selection header, and |false_head|
  /// is the target of the false branch on the OpBranchConditional.
  /// In particular, false_head->false_head_for == this
  const BlockInfo* false_head = nullptr;
  /// If not null, then this block is an if-selection header, and when following
  /// the flow via the true and false branches, control first reconverges at
  /// |premerge_head|, and |premerge_head| is still inside the if-selection.
  /// In particular, premerge_head->premerge_head_for == this
  const BlockInfo* premerge_head = nullptr;
};

inline std::ostream& operator<<(std::ostream& o, const BlockInfo& bi) {
  o << "BlockInfo{"
    << " id: " << bi.id << " pos: " << bi.pos
    << " merge_for_header: " << bi.merge_for_header
    << " continue_for_header: " << bi.continue_for_header
    << " header_for_merge: " << bi.header_for_merge
    << " header_for_merge: " << bi.header_for_merge
    << " single_block_loop: " << int(bi.is_single_block_loop) << "}";
  return o;
}

/// A FunctionEmitter emits a SPIR-V function onto a Tint AST module.
class FunctionEmitter {
 public:
  /// Creates a FunctionEmitter, and prepares to write to the AST module
  /// in |pi|.
  /// @param pi a ParserImpl which has already executed BuildInternalModule
  /// @param function the function to emit
  FunctionEmitter(ParserImpl* pi, const spvtools::opt::Function& function);
  /// Destructor
  ~FunctionEmitter();

  /// Emits the function to AST module.
  /// @return whether emission succeeded
  bool Emit();

  /// @returns true if emission has not yet failed.
  bool success() const { return fail_stream_.status(); }
  /// @returns true if emission has failed.
  bool failed() const { return !success(); }

  /// Returns the body of the function.  It is the bottom of the statement
  /// stack.
  /// @returns the body of the function.
  const ast::StatementList& ast_body();

  /// Records failure.
  /// @returns a FailStream on which to emit diagnostics.
  FailStream& Fail() { return fail_stream_.Fail(); }

  /// @returns the parser implementation
  ParserImpl* parser() { return &parser_impl_; }

  /// Emits the declaration, which comprises the name, parameters, and
  /// return type. The function AST node is appended to the module
  /// AST node.
  /// @returns true if emission has not yet failed.
  bool EmitFunctionDeclaration();

  /// Emits the function body, populating the bottom entry of the statements
  /// stack.
  /// @returns false if emission failed.
  bool EmitBody();

  /// Records a mapping from block ID to a BlockInfo struct.  Populates
  /// |block_info_|
  void RegisterBasicBlocks();

  /// Verifies that terminators only branch to labels in the current function.
  /// Assumes basic blocks have been registered.
  /// @returns true if terminators are sane
  bool TerminatorsAreSane();

  /// Populates merge-header cross-links and the |is_single_block_loop| member
  /// of BlockInfo.  Also verifies that merge instructions go to blocks in
  /// the same function.  Assumes basic blocks have been registered, and
  /// terminators are sane.
  /// @returns false if registration fails
  bool RegisterMerges();

  /// Determines the output order for the basic blocks in the function.
  /// Populates |block_order_| and the |pos| block info member.
  /// Assumes basic blocks have been registered.
  void ComputeBlockOrderAndPositions();

  /// @returns the reverse structured post order of the basic blocks in
  /// the function.
  const std::vector<uint32_t>& block_order() const { return block_order_; }

  /// Verifies that the orderings among a structured header, continue target,
  /// and merge block are valid. Assumes block order has been computed, and
  /// merges are valid and recorded.
  /// @returns false if invalid nesting was detected
  bool VerifyHeaderContinueMergeOrder();

  /// Labels each basic block with its nearest enclosing structured construct.
  /// Populates the |construct| member of BlockInfo, and the |constructs_| list.
  /// Assumes terminators are sane and merges have been registered, block
  /// order has been computed, and each block is labeled with its position.
  /// Checks nesting of structured control flow constructs.
  /// @returns false if bad nesting has been detected
  bool LabelControlFlowConstructs();

  /// @returns the structured constructs
  const ConstructList& constructs() const { return constructs_; }

  /// Marks blocks targets of a switch, either as the head of a case or
  /// as the default target.
  /// @returns false on failure
  bool FindSwitchCaseHeaders();

  /// Classifies the successor CFG edges for the ordered basic blocks.
  /// Also checks validity of each edge (populates the |succ_edge| field of
  /// BlockInfo). Implicitly checks dominance rules for headers and continue
  /// constructs. Assumes each block has been labeled with its control flow
  /// construct.
  /// @returns false on failure
  bool ClassifyCFGEdges();

  /// Marks the blocks within a selection construct that are the first blocks
  /// in the "then" clause, the "else" clause, and the "premerge" clause.
  /// The head of the premerge clause is the block, if it exists, at which
  /// control flow reconverges from the "then" and "else" clauses, but before
  /// before the merge block for that selection.   The existence of a premerge
  /// should be an exceptional case, but is allowed by the structured control
  /// flow rules.
  /// @returns false if bad nesting has been detected.
  bool FindIfSelectionInternalHeaders();

  /// Emits declarations of function variables.
  /// @returns false if emission failed.
  bool EmitFunctionVariables();

  /// Emits statements in the body.
  /// @returns false if emission failed.
  bool EmitFunctionBodyStatements();

  /// Emits a basic block.
  /// @param block_info the block to emit
  /// @returns false if emission failed.
  bool EmitBasicBlock(const BlockInfo& block_info);

  /// Emits an IfStatement, including its condition expression, and sets
  /// up the statement stack to accumulate subsequent basic blocks into
  /// the "then" and "else" clauses.
  /// @param block_info the if-selection header block
  /// @returns false if emission failed.
  bool EmitIfStart(const BlockInfo& block_info);

  /// Emits a LoopStatement, and pushes a new StatementBlock to accumulate
  /// the remaining instructions in the current block and subsequent blocks
  /// in the loop.
  /// @param construct the loop construct
  /// @returns false if emission failed.
  bool EmitLoopStart(const Construct* construct);

  /// Emits a ContinuingStatement, and pushes a new StatementBlock to accumulate
  /// the remaining instructions in the current block and subsequent blocks
  /// in the continue construct.
  /// @param construct the continue construct
  /// @returns false if emission failed.
  bool EmitContinuingStart(const Construct* construct);

  /// Emits the non-control-flow parts of a basic block, but only once.
  /// The |already_emitted| parameter indicates whether the code has already
  /// been emitted, and is used to signal that this invocation actually emitted
  /// it.
  /// @param block_info the block to emit
  /// @param already_emitted the block to emit
  /// @returns false if the code had not yet been emitted, but emission failed
  bool EmitStatementsInBasicBlock(const BlockInfo& block_info,
                                  bool* already_emitted);

  /// Emits code for terminators, but that aren't part of entering or
  /// resolving structured control flow. That is, if the basic block
  /// terminator calls for it, emit the fallthrough, break, continue, return,
  /// or kill commands.
  /// @param block_info the block with the terminator to emit (if any)
  /// @returns false if emission failed
  bool EmitNormalTerminator(const BlockInfo& block_info);

  /// Returns a new statement to represent the given branch representing a
  /// "normal" terminator, as in the sense of EmitNormalTerminator.  If no
  /// WGSL statement is required, the statement will be nullptr.
  /// @param src_info the source block
  /// @param dest_info the destination block
  /// @returns the new statement, or a null statement
  std::unique_ptr<ast::Statement> MakeBranch(const BlockInfo& src_info,
                                             const BlockInfo& dest_info) const;

  /// Returns a new if statement with the given statements as the then-clause
  /// and the else-clause.  Either or both clauses might be nullptr. If both
  /// are nullptr, then don't make a new statement and instead return nullptr.
  /// @param condition the branching condition
  /// @param then_stmt the statement for the then clause of the if, or nullptr
  /// @param else_stmt the statement for the else clause of the if, or nullptr
  /// @returns the new statement, or nullptr
  std::unique_ptr<ast::Statement> MakeSimpleIf(
      std::unique_ptr<ast::Expression> condition,
      std::unique_ptr<ast::Statement> then_stmt,
      std::unique_ptr<ast::Statement> else_stmt) const;

  /// Emits a normal instruction: not a terminator, label, or variable
  /// declaration.
  /// @param inst the instruction
  /// @returns false if emission failed.
  bool EmitStatement(const spvtools::opt::Instruction& inst);

  /// Emits a const definition for a SPIR-V value.
  /// @param inst the SPIR-V instruction defining the value
  /// @param ast_expr the already-computed AST expression for the value
  /// @returns false if emission failed.
  bool EmitConstDefinition(const spvtools::opt::Instruction& inst,
                           TypedExpression ast_expr);

  /// Makes an expression
  /// @param id the SPIR-V ID of the value
  /// @returns true if emission has not yet failed.
  TypedExpression MakeExpression(uint32_t id);

  /// Creates an expression and supporting statements for a combinatorial
  /// instruction, or returns null.  A SPIR-V instruction is combinatorial
  /// if it has no side effects and its result depends only on its operands,
  /// and not on accessing external state like memory or the state of other
  /// invocations.  Statements are only created if required to provide values
  /// to the expression. Supporting statements are not required to be
  /// combinatorial.
  /// @param inst a SPIR-V instruction representing an exrpression
  /// @returns an AST expression for the instruction, or nullptr.
  TypedExpression MaybeEmitCombinatorialValue(
      const spvtools::opt::Instruction& inst);

  /// Gets the block info for a block ID, if any exists
  /// @param id the SPIR-V ID of the OpLabel instruction starting the block
  /// @returns the block info for the given ID, if it exists, or nullptr
  BlockInfo* GetBlockInfo(uint32_t id) {
    auto where = block_info_.find(id);
    if (where == block_info_.end())
      return nullptr;
    return where->second.get();
  }

 private:
  /// @returns the store type for the OpVariable instruction, or
  /// null on failure.
  ast::type::Type* GetVariableStoreType(
      const spvtools::opt::Instruction& var_decl_inst);

  /// Returns an expression for an instruction operand. Signedness conversion is
  /// performed to match the result type of the SPIR-V instruction.
  /// @param inst the SPIR-V instruction
  /// @param operand_index the index of the operand, counting 0 as the first
  /// input operand
  /// @returns a new expression node
  TypedExpression MakeOperand(const spvtools::opt::Instruction& inst,
                              uint32_t operand_index);

  /// Returns an expression for a SPIR-V OpAccessChain or OpInBoundsAccessChain
  /// instruction.
  /// @param inst the SPIR-V instruction
  /// @returns an expression
  TypedExpression MakeAccessChain(const spvtools::opt::Instruction& inst);

  /// Finds the header block for a structured construct that we can "break"
  /// out from, from deeply nested control flow, if such a block exists.
  /// If the construct is:
  ///  - a switch selection: return the selection header (ending in OpSwitch)
  ///  - a loop construct: return the loop header block
  ///  - a continue construct: return the loop header block
  /// Otherwise, return nullptr.
  /// @param c a structured construct, or nullptr
  /// @returns the block info for the structured header we can "break" from,
  /// or nullptr
  BlockInfo* HeaderIfBreakable(const Construct* c);

  /// Appends a new statement to the top of the statement stack.
  /// Does nothing if the statement is null.
  /// @param statement the new statement
  /// @returns a pointer to the statement.
  ast::Statement* AddStatement(std::unique_ptr<ast::Statement> statement);

  /// @returns the last statetment in the top of the statement stack.
  ast::Statement* LastStatement();

  struct StatementBlock;
  using CompletionAction = std::function<void(StatementBlock*)>;

  // A StatementBlock represents a braced-list of statements while it is being
  // constructed.
  struct StatementBlock {
    StatementBlock(const Construct* construct,
                   uint32_t end_id,
                   CompletionAction completion_action,
                   ast::StatementList statements,
                   ast::CaseStatementList cases);
    StatementBlock(StatementBlock&&);
    ~StatementBlock();

    // The construct to which this construct constributes.
    const Construct* construct_;
    // The ID of the block at which the completion action should be triggerd
    // and this statement block discarded. This is often the |end_id| of
    // |construct| itself.
    uint32_t end_id_;
    // The completion action finishes processing this statement block.
    CompletionAction completion_action_;

    // Only one of |statements| or |cases| is active.

    // The list of statements being built.
    ast::StatementList statements_;
    // The list of cases being built, for a switch.
    ast::CaseStatementList cases_;
  };

  /// Pushes an empty statement block onto the statements stack.
  /// @param action the completion action for this block
  void PushNewStatementBlock(const Construct* construct,
                             uint32_t end_id,
                             CompletionAction action);

  ParserImpl& parser_impl_;
  ast::Module& ast_module_;
  spvtools::opt::IRContext& ir_context_;
  spvtools::opt::analysis::DefUseManager* def_use_mgr_;
  spvtools::opt::analysis::ConstantManager* constant_mgr_;
  spvtools::opt::analysis::TypeManager* type_mgr_;
  FailStream& fail_stream_;
  Namer& namer_;
  const spvtools::opt::Function& function_;

  // A stack of statement lists. Each list is contained in a construct in
  // the next deeper element of stack. The 0th entry represents the statements
  // for the entire function.  This stack is never empty.
  // The |construct| member for the 0th element is only valid during the
  // lifetime of the EmitFunctionBodyStatements method.
  std::vector<StatementBlock> statements_stack_;

  // The set of IDs that have already had an identifier name generated for it.
  std::unordered_set<uint32_t> identifier_values_;
  // Mapping from SPIR-V ID that is used at most once, to its AST expression.
  std::unordered_map<uint32_t, TypedExpression> singly_used_values_;

  // The IDs of basic blocks, in reverse structured post-order (RSPO).
  // This is the output order for the basic blocks.
  std::vector<uint32_t> block_order_;

  // Mapping from block ID to its bookkeeping info.
  std::unordered_map<uint32_t, std::unique_ptr<BlockInfo>> block_info_;

  // Structured constructs, where enclosing constructs precede their children.
  ConstructList constructs_;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_FUNCTION_H_
