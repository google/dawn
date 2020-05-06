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
#include "src/ast/expression.h"
#include "src/ast/module.h"
#include "src/reader/spirv/construct.h"
#include "src/reader/spirv/fail_stream.h"
#include "src/reader/spirv/namer.h"
#include "src/reader/spirv/parser_impl.h"

namespace tint {
namespace reader {
namespace spirv {

/// Kinds of CFG edges.
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
  // TODO(dneto): Rename this to kIfBreak after we correctly classify edges
  // as kSwitchBreak.
  kToMerge,
  // An edge from one switch case to the next sibling switch case.
  kCaseFallThrough,
  // None of the above. By structured control flow rules, there can be at most
  // one forward edge leaving a basic block. Otherwise there must have been a
  // merge instruction declaring the divergence and associated reconvergence
  // point.
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

  /// Maps the ID of a successor block (in the CFG) to its edge classification.
  std::unordered_map<uint32_t, EdgeKind> succ_edge;
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

  /// @returns the body of the function.
  const ast::StatementList& ast_body() { return ast_body_; }

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

  /// Emits the function body, populating |ast_body_|
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

  /// Emits declarations of function variables.
  /// @returns false if emission failed.
  bool EmitFunctionVariables();

  /// Emits statements in the body.
  /// @returns false if emission failed.
  bool EmitFunctionBodyStatements();

  /// Emits a basic block
  /// @param bb internal representation of the basic block
  /// @returns false if emission failed.
  bool EmitStatementsInBasicBlock(const spvtools::opt::BasicBlock& bb);

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

  /// Finds the loop header block for a loop construct or continue construct.
  /// The loop header block is the block with the corresponding OpLoopMerge
  /// instruction.
  /// @param c the loop or continue construct
  /// @returns the block info for the loop header.
  BlockInfo* HeaderForLoopOrContinue(const Construct* c);

  ParserImpl& parser_impl_;
  ast::Module& ast_module_;
  spvtools::opt::IRContext& ir_context_;
  spvtools::opt::analysis::DefUseManager* def_use_mgr_;
  spvtools::opt::analysis::ConstantManager* constant_mgr_;
  spvtools::opt::analysis::TypeManager* type_mgr_;
  FailStream& fail_stream_;
  Namer& namer_;
  const spvtools::opt::Function& function_;
  ast::StatementList ast_body_;
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
