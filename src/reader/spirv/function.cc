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

#include "src/reader/spirv/function.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "source/opt/basic_block.h"
#include "source/opt/function.h"
#include "source/opt/instruction.h"
#include "source/opt/module.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/as_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/kill_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/storage_class.h"
#include "src/ast/switch_statement.h"
#include "src/ast/uint_literal.h"
#include "src/ast/unary_op.h"
#include "src/ast/unary_op_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/reader/spirv/construct.h"
#include "src/reader/spirv/fail_stream.h"
#include "src/reader/spirv/parser_impl.h"

// Terms:
//    CFG: the control flow graph of the function, where basic blocks are the
//    nodes, and branches form the directed arcs.  The function entry block is
//    the root of the CFG.
//
//    Suppose H is a header block (i.e. has an OpSelectionMerge or OpLoopMerge).
//    Then:
//    - Let M(H) be the merge block named by the merge instruction in H.
//    - If H is a loop header, i.e. has an OpLoopMerge instruction, then let
//      CT(H) be the continue target block named by the OpLoopMerge
//      instruction.
//    - If H is a selection construct whose header ends in
//      OpBranchConditional with true target %then and false target %else,
//      then  TT(H) = %then and FT(H) = %else
//
// Determining output block order:
//    The "structured post-order traversal" of the CFG is a post-order traversal
//    of the basic blocks in the CFG, where:
//      We visit the entry node of the function first.
//      When visiting a header block:
//        We next visit its merge block
//        Then if it's a loop header, we next visit the continue target,
//      Then we visit the block's successors (whether it's a header or not)
//        If the block ends in an OpBranchConditional, we visit the false target
//        before the true target.
//
//    The "reverse structured post-order traversal" of the CFG is the reverse
//    of the structured post-order traversal.
//    This is the order of basic blocks as they should be emitted to the WGSL
//    function. It is the order computed by ComputeBlockOrder, and stored in
//    the |FunctionEmiter::block_order_|.
//    Blocks not in this ordering are ignored by the rest of the algorithm.
//
//    Note:
//     - A block D in the function might not appear in this order because
//       no block in the order branches to D.
//     - An unreachable block D might still be in the order because some header
//       block in the order names D as its continue target, or merge block,
//       or D is reachable from one of those otherwise-unreachable continue
//       targets or merge blocks.
//
// Terms:
//    Let Pos(B) be the index position of a block B in the computed block order.
//
// CFG intervals and valid nesting:
//
//    A correctly structured CFG satisfies nesting rules that we can check by
//    comparing positions of related blocks.
//
//    If header block H is in the block order, then the following holds:
//
//      Pos(H) < Pos(M(H))
//
//      If CT(H) exists, then:
//
//         Pos(H) <= Pos(CT(H)), with equality exactly for single-block loops
//         Pos(CT(H)) < Pos(M)
//
//    This gives us the fundamental ordering of blocks in relation to a
//    structured construct:
//      The blocks before H in the block order, are not in the construct
//      The blocks at M(H) or later in the block order, are not in the construct
//      The blocks in a selection headed at H are in positions [ Pos(H),
//      Pos(M(H)) ) The blocks in a loop construct headed at H are in positions
//      [ Pos(H), Pos(CT(H)) ) The blocks in the continue construct for loop
//      headed at H are in
//        positions [ Pos(CT(H)), Pos(M(H)) )
//
//      Schematically, for a selection construct headed by H, the blocks are in
//      order from left to right:
//
//                 ...a-b-c H d-e-f M(H) n-o-p...
//
//           where ...a-b-c: blocks before the selection construct
//           where H and d-e-f: blocks in the selection construct
//           where M(H) and n-o-p...: blocks after the selection construct
//
//      Schematically, for a single-block loop construct headed by H, there are
//      blocks in order from left to right:
//
//                 ...a-b-c H M(H) n-o-p...
//
//           where ...a-b-c: blocks before the loop
//           where H is the continue construct; CT(H)=H, and the loop construct
//           is *empty* where M(H) and n-o-p...: blocks after the loop and
//           continue constructs
//
//      Schematically, for a multi-block loop construct headed by H, there are
//      blocks in order from left to right:
//
//                 ...a-b-c H d-e-f CT(H) j-k-l M(H) n-o-p...
//
//           where ...a-b-c: blocks before the loop
//           where H and d-e-f: blocks in the loop construct
//           where CT(H) and j-k-l: blocks in the continue construct
//           where M(H) and n-o-p...: blocks after the loop and continue
//           constructs
//

namespace tint {
namespace reader {
namespace spirv {

namespace {

// Gets the AST unary opcode for the given SPIR-V opcode, if any
// @param opcode SPIR-V opcode
// @param ast_unary_op return parameter
// @returns true if it was a unary operation
bool GetUnaryOp(SpvOp opcode, ast::UnaryOp* ast_unary_op) {
  switch (opcode) {
    case SpvOpSNegate:
    case SpvOpFNegate:
      *ast_unary_op = ast::UnaryOp::kNegation;
      return true;
    case SpvOpLogicalNot:
    case SpvOpNot:
      *ast_unary_op = ast::UnaryOp::kNot;
      return true;
    default:
      break;
  }
  return false;
}

// Converts a SPIR-V opcode to its corresponding AST binary opcode, if any
// @param opcode SPIR-V opcode
// @returns the AST binary op for the given opcode, or kNone
ast::BinaryOp ConvertBinaryOp(SpvOp opcode) {
  switch (opcode) {
    case SpvOpIAdd:
    case SpvOpFAdd:
      return ast::BinaryOp::kAdd;
    case SpvOpISub:
    case SpvOpFSub:
      return ast::BinaryOp::kSubtract;
    case SpvOpIMul:
    case SpvOpFMul:
      return ast::BinaryOp::kMultiply;
    case SpvOpUDiv:
    case SpvOpSDiv:
    case SpvOpFDiv:
      return ast::BinaryOp::kDivide;
    case SpvOpUMod:
    case SpvOpSMod:
    case SpvOpFMod:
      return ast::BinaryOp::kModulo;
    case SpvOpShiftLeftLogical:
      return ast::BinaryOp::kShiftLeft;
    case SpvOpShiftRightLogical:
    case SpvOpShiftRightArithmetic:
      return ast::BinaryOp::kShiftRight;
    case SpvOpLogicalEqual:
    case SpvOpIEqual:
    case SpvOpFOrdEqual:
      return ast::BinaryOp::kEqual;
    case SpvOpLogicalNotEqual:
    case SpvOpINotEqual:
    case SpvOpFOrdNotEqual:
      return ast::BinaryOp::kNotEqual;
    case SpvOpBitwiseAnd:
      return ast::BinaryOp::kAnd;
    case SpvOpBitwiseOr:
      return ast::BinaryOp::kOr;
    case SpvOpBitwiseXor:
      return ast::BinaryOp::kXor;
    case SpvOpLogicalAnd:
      return ast::BinaryOp::kLogicalAnd;
    case SpvOpLogicalOr:
      return ast::BinaryOp::kLogicalOr;
    case SpvOpUGreaterThan:
    case SpvOpSGreaterThan:
    case SpvOpFOrdGreaterThan:
      return ast::BinaryOp::kGreaterThan;
    case SpvOpUGreaterThanEqual:
    case SpvOpSGreaterThanEqual:
    case SpvOpFOrdGreaterThanEqual:
      return ast::BinaryOp::kGreaterThanEqual;
    case SpvOpULessThan:
    case SpvOpSLessThan:
    case SpvOpFOrdLessThan:
      return ast::BinaryOp::kLessThan;
    case SpvOpULessThanEqual:
    case SpvOpSLessThanEqual:
    case SpvOpFOrdLessThanEqual:
      return ast::BinaryOp::kLessThanEqual;
    default:
      break;
  }
  // It's not clear what OpSMod should map to.
  // https://bugs.chromium.org/p/tint/issues/detail?id=52
  return ast::BinaryOp::kNone;
}

// If the given SPIR-V opcode is a floating point unordered comparison,
// then returns the binary float comparison for which it is the negation.
// Othewrise returns BinaryOp::kNone.
// @param opcode SPIR-V opcode
// @returns operation corresponding to negated version of the SPIR-V opcode
ast::BinaryOp NegatedFloatCompare(SpvOp opcode) {
  switch (opcode) {
    case SpvOpFUnordEqual:
      return ast::BinaryOp::kNotEqual;
    case SpvOpFUnordNotEqual:
      return ast::BinaryOp::kEqual;
    case SpvOpFUnordLessThan:
      return ast::BinaryOp::kGreaterThanEqual;
    case SpvOpFUnordLessThanEqual:
      return ast::BinaryOp::kGreaterThan;
    case SpvOpFUnordGreaterThan:
      return ast::BinaryOp::kLessThanEqual;
    case SpvOpFUnordGreaterThanEqual:
      return ast::BinaryOp::kLessThan;
    default:
      break;
  }
  return ast::BinaryOp::kNone;
}

// @returns the merge block ID for the given basic block, or 0 if there is none.
uint32_t MergeFor(const spvtools::opt::BasicBlock& bb) {
  // Get the OpSelectionMerge or OpLoopMerge instruction, if any.
  auto* inst = bb.GetMergeInst();
  return inst == nullptr ? 0 : inst->GetSingleWordInOperand(0);
}

// @returns the continue target ID for the given basic block, or 0 if there
// is none.
uint32_t ContinueTargetFor(const spvtools::opt::BasicBlock& bb) {
  // Get the OpLoopMerge instruction, if any.
  auto* inst = bb.GetLoopMergeInst();
  return inst == nullptr ? 0 : inst->GetSingleWordInOperand(1);
}

// A structured traverser produces the reverse structured post-order of the
// CFG of a function.  The blocks traversed are the transitive closure (minimum
// fixed point) of:
//  - the entry block
//  - a block reached by a branch from another block in the set
//  - a block mentioned as a merge block or continue target for a block in the
//  set
class StructuredTraverser {
 public:
  explicit StructuredTraverser(const spvtools::opt::Function& function)
      : function_(function) {
    for (auto& block : function_) {
      id_to_block_[block.id()] = &block;
    }
  }

  // Returns the reverse postorder traversal of the CFG, where:
  //  - a merge block always follows its associated constructs
  //  - a continue target always follows the associated loop construct, if any
  // @returns the IDs of blocks in reverse structured post order
  std::vector<uint32_t> ReverseStructuredPostOrder() {
    visit_order_.clear();
    visited_.clear();
    VisitBackward(function_.entry()->id());

    std::vector<uint32_t> order(visit_order_.rbegin(), visit_order_.rend());
    return order;
  }

 private:
  // Executes a depth first search of the CFG, where right after we visit a
  // header, we will visit its merge block, then its continue target (if any).
  // Also records the post order ordering.
  void VisitBackward(uint32_t id) {
    if (id == 0)
      return;
    if (visited_.count(id))
      return;
    visited_.insert(id);

    const spvtools::opt::BasicBlock* bb =
        id_to_block_[id];  // non-null for valid modules
    VisitBackward(MergeFor(*bb));
    VisitBackward(ContinueTargetFor(*bb));

    // Visit successors. We will naturally skip the continue target and merge
    // blocks.
    auto* terminator = bb->terminator();
    auto opcode = terminator->opcode();
    if (opcode == SpvOpBranchConditional) {
      // Visit the false branch, then the true branch, to make them come
      // out in the natural order for an "if".
      VisitBackward(terminator->GetSingleWordInOperand(2));
      VisitBackward(terminator->GetSingleWordInOperand(1));
    } else if (opcode == SpvOpBranch) {
      VisitBackward(terminator->GetSingleWordInOperand(0));
    } else if (opcode == SpvOpSwitch) {
      // TODO(dneto): Consider visiting the labels in literal-value order.
      std::vector<uint32_t> successors;
      bb->ForEachSuccessorLabel([&successors](const uint32_t succ_id) {
        successors.push_back(succ_id);
      });
      for (auto succ_id : successors) {
        VisitBackward(succ_id);
      }
    }

    visit_order_.push_back(id);
  }

  const spvtools::opt::Function& function_;
  std::unordered_map<uint32_t, const spvtools::opt::BasicBlock*> id_to_block_;
  std::vector<uint32_t> visit_order_;
  std::unordered_set<uint32_t> visited_;
};

}  // namespace

BlockInfo::BlockInfo(const spvtools::opt::BasicBlock& bb)
    : basic_block(&bb), id(bb.id()) {}

BlockInfo::~BlockInfo() {}

FunctionEmitter::FunctionEmitter(ParserImpl* pi,
                                 const spvtools::opt::Function& function)
    : parser_impl_(*pi),
      ast_module_(pi->get_module()),
      ir_context_(*(pi->ir_context())),
      def_use_mgr_(ir_context_.get_def_use_mgr()),
      constant_mgr_(ir_context_.get_constant_mgr()),
      type_mgr_(ir_context_.get_type_mgr()),
      fail_stream_(pi->fail_stream()),
      namer_(pi->namer()),
      function_(function) {
  PushNewStatementBlock(nullptr, 0, nullptr);
}

FunctionEmitter::~FunctionEmitter() = default;

FunctionEmitter::StatementBlock::StatementBlock(
    const Construct* construct,
    uint32_t end_id,
    CompletionAction completion_action,
    ast::StatementList statements,
    ast::CaseStatementList cases)
    : construct_(construct),
      end_id_(end_id),
      completion_action_(completion_action),
      statements_(std::move(statements)),
      cases_(std::move(cases)) {}

FunctionEmitter::StatementBlock::StatementBlock(StatementBlock&&) = default;

FunctionEmitter::StatementBlock::~StatementBlock() = default;

void FunctionEmitter::PushNewStatementBlock(const Construct* construct,
                                            uint32_t end_id,
                                            CompletionAction action) {
  statements_stack_.emplace_back(StatementBlock(construct, end_id, action,
                                                ast::StatementList{},
                                                ast::CaseStatementList{}));
}

const ast::StatementList& FunctionEmitter::ast_body() {
  assert(!statements_stack_.empty());
  return statements_stack_[0].statements_;
}

ast::Statement* FunctionEmitter::AddStatement(
    std::unique_ptr<ast::Statement> statement) {
  assert(!statements_stack_.empty());
  auto* result = statement.get();
  if (result != nullptr) {
    statements_stack_.back().statements_.emplace_back(std::move(statement));
  }
  return result;
}

ast::Statement* FunctionEmitter::LastStatement() {
  assert(!statements_stack_.empty());
  const auto& statement_list = statements_stack_.back().statements_;
  assert(!statement_list.empty());
  return statement_list.back().get();
}

bool FunctionEmitter::Emit() {
  if (failed()) {
    return false;
  }
  // We only care about functions with bodies.
  if (function_.cbegin() == function_.cend()) {
    return true;
  }

  if (!EmitFunctionDeclaration()) {
    return false;
  }

  if (!EmitBody()) {
    return false;
  }

  // Set the body of the AST function node.
  if (statements_stack_.size() != 1) {
    return Fail() << "internal error: statement-list stack should have 1 "
                     "element but has "
                  << statements_stack_.size();
  }
  ast::StatementList body(std::move(statements_stack_[0].statements_));
  parser_impl_.get_module().functions().back()->set_body(std::move(body));
  // Maintain the invariant by repopulating the one and only element.
  statements_stack_.clear();
  PushNewStatementBlock(constructs_[0].get(), 0, nullptr);

  return success();
}

bool FunctionEmitter::EmitFunctionDeclaration() {
  if (failed()) {
    return false;
  }

  const auto name = namer_.Name(function_.result_id());
  // Surprisingly, the "type id" on an OpFunction is the result type of the
  // function, not the type of the function.  This is the one exceptional case
  // in SPIR-V where the type ID is not the type of the result ID.
  auto* ret_ty = parser_impl_.ConvertType(function_.type_id());
  if (failed()) {
    return false;
  }
  if (ret_ty == nullptr) {
    return Fail()
           << "internal error: unregistered return type for function with ID "
           << function_.result_id();
  }

  ast::VariableList ast_params;
  function_.ForEachParam(
      [this, &ast_params](const spvtools::opt::Instruction* param) {
        auto* ast_type = parser_impl_.ConvertType(param->type_id());
        if (ast_type != nullptr) {
          ast_params.emplace_back(parser_impl_.MakeVariable(
              param->result_id(), ast::StorageClass::kNone, ast_type));
        } else {
          // We've already logged an error and emitted a diagnostic. Do nothing
          // here.
        }
      });
  if (failed()) {
    return false;
  }

  auto ast_fn =
      std::make_unique<ast::Function>(name, std::move(ast_params), ret_ty);
  ast_module_.AddFunction(std::move(ast_fn));

  return success();
}

ast::type::Type* FunctionEmitter::GetVariableStoreType(
    const spvtools::opt::Instruction& var_decl_inst) {
  const auto type_id = var_decl_inst.type_id();
  auto* var_ref_type = type_mgr_->GetType(type_id);
  if (!var_ref_type) {
    Fail() << "internal error: variable type id " << type_id
           << " has no registered type";
    return nullptr;
  }
  auto* var_ref_ptr_type = var_ref_type->AsPointer();
  if (!var_ref_ptr_type) {
    Fail() << "internal error: variable type id " << type_id
           << " is not a pointer type";
    return nullptr;
  }
  auto var_store_type_id = type_mgr_->GetId(var_ref_ptr_type->pointee_type());
  return parser_impl_.ConvertType(var_store_type_id);
}

bool FunctionEmitter::EmitBody() {
  RegisterBasicBlocks();

  if (!TerminatorsAreSane()) {
    return false;
  }
  if (!RegisterMerges()) {
    return false;
  }

  ComputeBlockOrderAndPositions();
  if (!VerifyHeaderContinueMergeOrder()) {
    return false;
  }
  if (!LabelControlFlowConstructs()) {
    return false;
  }
  if (!FindSwitchCaseHeaders()) {
    return false;
  }
  if (!ClassifyCFGEdges()) {
    return false;
  }
  if (!FindIfSelectionInternalHeaders()) {
    return false;
  }

  // TODO(dneto): register phis
  // TODO(dneto): register SSA values which need to be hoisted

  if (!EmitFunctionVariables()) {
    return false;
  }
  if (!EmitFunctionBodyStatements()) {
    return false;
  }
  return success();
}

void FunctionEmitter::RegisterBasicBlocks() {
  for (auto& block : function_) {
    block_info_[block.id()] = std::make_unique<BlockInfo>(block);
  }
}

bool FunctionEmitter::TerminatorsAreSane() {
  if (failed()) {
    return false;
  }

  const auto entry_id = function_.begin()->id();
  for (const auto& block : function_) {
    if (!block.terminator()) {
      return Fail() << "Block " << block.id() << " has no terminator";
    }
  }
  for (const auto& block : function_) {
    block.WhileEachSuccessorLabel(
        [this, &block, entry_id](const uint32_t succ_id) -> bool {
          if (succ_id == entry_id) {
            return Fail() << "Block " << block.id()
                          << " branches to function entry block " << entry_id;
          }
          if (!GetBlockInfo(succ_id)) {
            return Fail() << "Block " << block.id() << " in function "
                          << function_.DefInst().result_id() << " branches to "
                          << succ_id << " which is not a block in the function";
          }
          return true;
        });
  }
  return success();
}

bool FunctionEmitter::RegisterMerges() {
  if (failed()) {
    return false;
  }

  const auto entry_id = function_.begin()->id();
  for (const auto& block : function_) {
    const auto block_id = block.id();
    auto* block_info = GetBlockInfo(block_id);
    if (!block_info) {
      return Fail() << "internal error: block " << block_id
                    << " missing; blocks should already "
                       "have been registered";
    }

    if (const auto* inst = block.GetMergeInst()) {
      auto terminator_opcode = block.terminator()->opcode();
      switch (inst->opcode()) {
        case SpvOpSelectionMerge:
          if ((terminator_opcode != SpvOpBranchConditional) &&
              (terminator_opcode != SpvOpSwitch)) {
            return Fail() << "Selection header " << block_id
                          << " does not end in an OpBranchConditional or "
                             "OpSwitch instruction";
          }
          break;
        case SpvOpLoopMerge:
          if ((terminator_opcode != SpvOpBranchConditional) &&
              (terminator_opcode != SpvOpBranch)) {
            return Fail() << "Loop header " << block_id
                          << " does not end in an OpBranch or "
                             "OpBranchConditional instruction";
          }
          break;
        default:
          break;
      }

      const uint32_t header = block.id();
      auto* header_info = block_info;
      const uint32_t merge = inst->GetSingleWordInOperand(0);
      auto* merge_info = GetBlockInfo(merge);
      if (!merge_info) {
        return Fail() << "Structured header block " << header
                      << " declares invalid merge block " << merge;
      }
      if (merge == header) {
        return Fail() << "Structured header block " << header
                      << " cannot be its own merge block";
      }
      if (merge_info->header_for_merge) {
        return Fail() << "Block " << merge
                      << " declared as merge block for more than one header: "
                      << merge_info->header_for_merge << ", " << header;
      }
      merge_info->header_for_merge = header;
      header_info->merge_for_header = merge;

      if (inst->opcode() == SpvOpLoopMerge) {
        if (header == entry_id) {
          return Fail() << "Function entry block " << entry_id
                        << " cannot be a loop header";
        }
        const uint32_t ct = inst->GetSingleWordInOperand(1);
        auto* ct_info = GetBlockInfo(ct);
        if (!ct_info) {
          return Fail() << "Structured header " << header
                        << " declares invalid continue target " << ct;
        }
        if (ct == merge) {
          return Fail() << "Invalid structured header block " << header
                        << ": declares block " << ct
                        << " as both its merge block and continue target";
        }
        if (ct_info->header_for_continue) {
          return Fail()
                 << "Block " << ct
                 << " declared as continue target for more than one header: "
                 << ct_info->header_for_continue << ", " << header;
        }
        ct_info->header_for_continue = header;
        header_info->continue_for_header = ct;
      }
    }

    // Check single-block loop cases.
    bool is_single_block_loop = false;
    block_info->basic_block->ForEachSuccessorLabel(
        [&is_single_block_loop, block_id](const uint32_t succ) {
          if (block_id == succ)
            is_single_block_loop = true;
        });
    block_info->is_single_block_loop = is_single_block_loop;
    const auto ct = block_info->continue_for_header;
    if (is_single_block_loop && ct != block_id) {
      return Fail() << "Block " << block_id
                    << " branches to itself but is not its own continue target";
    } else if (!is_single_block_loop && ct == block_id) {
      return Fail() << "Loop header block " << block_id
                    << " declares itself as its own continue target, but "
                       "does not branch to itself";
    }
  }
  return success();
}

void FunctionEmitter::ComputeBlockOrderAndPositions() {
  block_order_ = StructuredTraverser(function_).ReverseStructuredPostOrder();

  for (uint32_t i = 0; i < block_order_.size(); ++i) {
    GetBlockInfo(block_order_[i])->pos = i;
  }
}

bool FunctionEmitter::VerifyHeaderContinueMergeOrder() {
  // Verify interval rules for a structured header block:
  //
  //    If the CFG satisfies structured control flow rules, then:
  //    If header H is reachable, then the following "interval rules" hold,
  //    where M(H) is H's merge block, and CT(H) is H's continue target:
  //
  //      Pos(H) < Pos(M(H))
  //
  //      If CT(H) exists, then:
  //         Pos(H) <= Pos(CT(H)), with equality exactly for single-block loops
  //         Pos(CT(H)) < Pos(M)
  //
  for (auto block_id : block_order_) {
    const auto* block_info = GetBlockInfo(block_id);
    const auto merge = block_info->merge_for_header;
    if (merge == 0) {
      continue;
    }
    // This is a header.
    const auto header = block_id;
    const auto* header_info = block_info;
    const auto header_pos = header_info->pos;
    const auto merge_pos = GetBlockInfo(merge)->pos;

    // Pos(H) < Pos(M(H))
    // Note: When recording merges we made sure H != M(H)
    if (merge_pos <= header_pos) {
      return Fail() << "Header " << header
                    << " does not strictly dominate its merge block " << merge;
      // TODO(dneto): Report a path from the entry block to the merge block
      // without going through the header block.
    }

    const auto ct = block_info->continue_for_header;
    if (ct == 0) {
      continue;
    }
    // Furthermore, this is a loop header.
    const auto* ct_info = GetBlockInfo(ct);
    const auto ct_pos = ct_info->pos;
    // Pos(H) <= Pos(CT(H)), with equality only for single-block loops.
    if (header_info->is_single_block_loop && ct_pos != header_pos) {
      Fail() << "Internal error: Single block loop.  CT pos is not the "
                "header pos. Should have already checked this";
    }
    if (!header_info->is_single_block_loop && (ct_pos <= header_pos)) {
      Fail() << "Loop header " << header
             << " does not dominate its continue target " << ct;
    }
    // Pos(CT(H)) < Pos(M(H))
    // Note: When recording merges we made sure CT(H) != M(H)
    if (merge_pos <= ct_pos) {
      return Fail() << "Merge block " << merge << " for loop headed at block "
                    << header
                    << " appears at or before the loop's continue "
                       "construct headed by "
                       "block "
                    << ct;
    }
  }
  return success();
}

bool FunctionEmitter::LabelControlFlowConstructs() {
  // Label each block in the block order with its nearest enclosing structured
  // control flow construct. Populates the |construct| member of BlockInfo.

  //  Keep a stack of enclosing structured control flow constructs.  Start
  //  with the synthetic construct representing the entire function.
  //
  //  Scan from left to right in the block order, and check conditions
  //  on each block in the following order:
  //
  //        a. When you reach a merge block, the top of the stack should
  //           be the associated header. Pop it off.
  //        b. When you reach a header, push it on the stack.
  //        c. When you reach a continue target, push it on the stack.
  //           (A block can be both a header and a continue target, in the case
  //           of a single-block loop, in which case it should also be its
  //           own backedge block.)
  //        c. When you reach a block with an edge branching backward (in the
  //           structured order) to block T:
  //            T should be a loop header, and the top of the stack should be a
  //            continue target associated with T.
  //            This is the end of the continue construct. Pop the continue
  //            target off the stack.
  //       (Note: We pop the merge off first because a merge block that marks
  //       the end of one construct can be a single-block loop.  So that block
  //       is a merge, a header, a continue target, and a backedge block.
  //       But we want to finish processing of the merge before dealing with
  //       the loop.)
  //
  //      In the same scan, mark each basic block with the nearest enclosing
  //      header: the most recent header for which we haven't reached its merge
  //      block. Also mark the the most recent continue target for which we
  //      haven't reached the backedge block.

  assert(block_order_.size() > 0);
  constructs_.clear();
  const auto entry_id = block_order_[0];

  // The stack of enclosing constructs.
  std::vector<Construct*> enclosing;

  // Creates a control flow construct and pushes it onto the stack.
  // Its parent is the top of the stack, or nullptr if the stack is empty.
  // Returns the newly created construct.
  auto push_construct = [this, &enclosing](size_t depth, Construct::Kind k,
                                           uint32_t begin_id,
                                           uint32_t end_id) -> Construct* {
    const auto begin_pos = GetBlockInfo(begin_id)->pos;
    const auto end_pos =
        end_id == 0 ? uint32_t(block_order_.size()) : GetBlockInfo(end_id)->pos;
    const auto* parent = enclosing.empty() ? nullptr : enclosing.back();
    // A loop construct is added right after its associated continue construct.
    // In that case, adjust the parent up.
    if (k == Construct::kLoop) {
      assert(parent);
      assert(parent->kind == Construct::kContinue);
      parent = parent->parent;
    }
    constructs_.push_back(std::make_unique<Construct>(
        parent, int(depth), k, begin_id, end_id, begin_pos, end_pos));
    Construct* result = constructs_.back().get();
    enclosing.push_back(result);
    return result;
  };

  // Make a synthetic kFunction construct to enclose all blocks in the function.
  push_construct(0, Construct::kFunction, entry_id, 0);
  // The entry block can be a selection construct, so be sure to process
  // it anyway.

  for (uint32_t i = 0; i < block_order_.size(); ++i) {
    const auto block_id = block_order_[i];
    assert(block_id > 0);
    auto* block_info = GetBlockInfo(block_id);
    assert(block_info);

    if (enclosing.empty()) {
      return Fail() << "internal error: too many merge blocks before block "
                    << block_id;
    }
    const Construct* top = enclosing.back();

    while (block_id == top->end_id) {
      // We've reached a predeclared end of the construct.  Pop it off the
      // stack.
      enclosing.pop_back();
      if (enclosing.empty()) {
        return Fail() << "internal error: too many merge blocks before block "
                      << block_id;
      }
      top = enclosing.back();
    }

    const auto merge = block_info->merge_for_header;
    if (merge != 0) {
      // The current block is a header.
      const auto header = block_id;
      const auto* header_info = block_info;
      const auto depth = 1 + top->depth;
      const auto ct = header_info->continue_for_header;
      if (ct != 0) {
        // The current block is a loop header.
        // We should see the continue construct after the loop construct, so
        // push the loop construct last.

        // From the interval rule, the continue construct consists of blocks
        // in the block order, starting at the continue target, until just
        // before the merge block.
        top = push_construct(depth, Construct::kContinue, ct, merge);
        // A single block loop has an empty loop construct.
        if (!header_info->is_single_block_loop) {
          // From the interval rule, the loop construct consists of blocks
          // in the block order, starting at the header, until just
          // before the continue target.
          top = push_construct(depth, Construct::kLoop, header, ct);
        }
      } else {
        // From the interval rule, the selection construct consists of blocks
        // in the block order, starting at the header, until just before the
        // merge block.
        const auto branch_opcode =
            header_info->basic_block->terminator()->opcode();
        const auto kind = (branch_opcode == SpvOpBranchConditional)
                              ? Construct::kIfSelection
                              : Construct::kSwitchSelection;
        top = push_construct(depth, kind, header, merge);
      }
    }

    assert(top);
    block_info->construct = top;
  }

  // At the end of the block list, we should only have the kFunction construct
  // left.
  if (enclosing.size() != 1) {
    return Fail() << "internal error: unbalanced structured constructs when "
                     "labeling structured constructs: ended with "
                  << enclosing.size() - 1 << " unterminated constructs";
  }
  const auto* top = enclosing[0];
  if (top->kind != Construct::kFunction || top->depth != 0) {
    return Fail() << "internal error: outermost construct is not a function?!";
  }

  return success();
}

bool FunctionEmitter::FindSwitchCaseHeaders() {
  if (failed()) {
    return false;
  }
  for (auto& construct : constructs_) {
    if (construct->kind != Construct::kSwitchSelection) {
      continue;
    }
    const auto* branch =
        GetBlockInfo(construct->begin_id)->basic_block->terminator();

    // Mark the default block
    const auto default_id = branch->GetSingleWordInOperand(1);
    auto* default_block = GetBlockInfo(default_id);
    // A default target can't be a backedge.
    if (construct->begin_pos >= default_block->pos) {
      // An OpSwitch must dominate its cases.  Also, it can't be a self-loop
      // as that would be a backedge, and backedges can only target a loop,
      // and loops use an OpLoopMerge instruction, which can't preceded an
      // OpSwitch.
      return Fail() << "Switch branch from block " << construct->begin_id
                    << " to default target block " << default_id
                    << " can't be a back-edge";
    }
    // A default target can be the merge block, but can't go past it.
    if (construct->end_pos < default_block->pos) {
      return Fail() << "Switch branch from block " << construct->begin_id
                    << " to default block " << default_id
                    << " escapes the selection construct";
    }
    if (default_block->default_head_for) {
      // An OpSwitch must dominate its cases, including the default target.
      return Fail() << "Block " << default_id
                    << " is declared as the default target for two OpSwitch "
                       "instructions, at blocks "
                    << default_block->default_head_for->begin_id << " and "
                    << construct->begin_id;
    }
    if ((default_block->header_for_merge != 0) &&
        (default_block->header_for_merge != construct->begin_id)) {
      // The switch instruction for this default block is an alternate path to
      // the merge block, and hence the merge block is not dominated by its own
      // (different) header.
      return Fail() << "Block " << default_block->id
                    << " is the default block for switch-selection header "
                    << construct->begin_id << " and also the merge block for "
                    << default_block->header_for_merge
                    << " (violates dominance rule)";
    }

    default_block->default_head_for = construct.get();
    default_block->default_is_merge = default_block->pos == construct->end_pos;

    // Map a case target to the list of values selecting that case.
    std::unordered_map<uint32_t, std::vector<uint64_t>> block_to_values;
    std::vector<uint32_t> case_targets;
    std::unordered_set<uint64_t> case_values;

    // Process case targets.
    for (uint32_t iarg = 2; iarg + 1 < branch->NumInOperands(); iarg += 2) {
      const auto o = branch->GetInOperand(iarg);
      const auto value = branch->GetInOperand(iarg).AsLiteralUint64();
      const auto case_target_id = branch->GetSingleWordInOperand(iarg + 1);

      if (case_values.count(value)) {
        return Fail() << "Duplicate case value " << value
                      << " in OpSwitch in block " << construct->begin_id;
      }
      case_values.insert(value);
      if (block_to_values.count(case_target_id) == 0) {
        case_targets.push_back(case_target_id);
      }
      block_to_values[case_target_id].push_back(value);
    }

    for (uint32_t case_target_id : case_targets) {
      auto* case_block = GetBlockInfo(case_target_id);

      case_block->case_values = std::make_unique<std::vector<uint64_t>>(
          std::move(block_to_values[case_target_id]));

      // A case target can't be a back-edge.
      if (construct->begin_pos >= case_block->pos) {
        // An OpSwitch must dominate its cases.  Also, it can't be a self-loop
        // as that would be a backedge, and backedges can only target a loop,
        // and loops use an OpLoopMerge instruction, which can't preceded an
        // OpSwitch.
        return Fail() << "Switch branch from block " << construct->begin_id
                      << " to case target block " << case_target_id
                      << " can't be a back-edge";
      }
      // A case target can be the merge block, but can't go past it.
      if (construct->end_pos < case_block->pos) {
        return Fail() << "Switch branch from block " << construct->begin_id
                      << " to case target block " << case_target_id
                      << " escapes the selection construct";
      }
      if (case_block->header_for_merge != 0 &&
          case_block->header_for_merge != construct->begin_id) {
        // The switch instruction for this case block is an alternate path to
        // the merge block, and hence the merge block is not dominated by its
        // own (different) header.
        return Fail() << "Block " << case_block->id
                      << " is a case block for switch-selection header "
                      << construct->begin_id << " and also the merge block for "
                      << case_block->header_for_merge
                      << " (violates dominance rule)";
      }

      // Mark the target as a case target.
      if (case_block->case_head_for) {
        // An OpSwitch must dominate its cases.
        return Fail()
               << "Block " << case_target_id
               << " is declared as the switch case target for two OpSwitch "
                  "instructions, at blocks "
               << case_block->case_head_for->begin_id << " and "
               << construct->begin_id;
      }
      case_block->case_head_for = construct.get();
    }
  }
  return success();
}

BlockInfo* FunctionEmitter::HeaderIfBreakable(const Construct* c) {
  if (c == nullptr) {
    return nullptr;
  }
  switch (c->kind) {
    case Construct::kLoop:
    case Construct::kSwitchSelection:
      return GetBlockInfo(c->begin_id);
    case Construct::kContinue: {
      const auto* continue_target = GetBlockInfo(c->begin_id);
      return GetBlockInfo(continue_target->header_for_continue);
    }
    default:
      break;
  }
  return nullptr;
}

bool FunctionEmitter::ClassifyCFGEdges() {
  if (failed()) {
    return false;
  }

  // Checks validity of CFG edges leaving each basic block.  This implicitly
  // checks dominance rules for headers and continue constructs.
  //
  // For each branch encountered, classify each edge (S,T) as:
  //    - a back-edge
  //    - a structured exit (specific ways of branching to enclosing construct)
  //    - a normal (forward) edge, either natural control flow or a case
  //    fallthrough
  //
  // If more than one block is targeted by a normal edge, then S must be a
  // structured header.
  //
  // Term: NEC(B) is the nearest enclosing construct for B.
  //
  // If edge (S,T) is a normal edge, and NEC(S) != NEC(T), then
  //    T is the header block of its NEC(T), and
  //    NEC(S) is the parent of NEC(T).

  for (const auto src : block_order_) {
    assert(src > 0);
    auto* src_info = GetBlockInfo(src);
    assert(src_info);
    const auto src_pos = src_info->pos;
    const auto& src_construct = *(src_info->construct);

    // Compute the ordered list of unique successors.
    std::vector<uint32_t> successors;
    {
      std::unordered_set<uint32_t> visited;
      src_info->basic_block->ForEachSuccessorLabel(
          [&successors, &visited](const uint32_t succ) {
            if (visited.count(succ) == 0) {
              successors.push_back(succ);
              visited.insert(succ);
            }
          });
    }

    // There should only be one backedge per backedge block.
    uint32_t num_backedges = 0;

    // Track destinations for normal forward edges, either kForward,
    // kCaseFallThrough, or kIfBreak. These count toward the need
    // to have a merge instruction.
    std::vector<uint32_t> normal_forward_edges;

    if (successors.empty() && src_construct.enclosing_continue) {
      // Kill and return are not allowed in a continue construct.
      return Fail() << "Invalid function exit at block " << src
                    << " from continue construct starting at "
                    << src_construct.enclosing_continue->begin_id;
    }

    for (const auto dest : successors) {
      const auto* dest_info = GetBlockInfo(dest);
      // We've already checked terminators are sane.
      assert(dest_info);
      const auto dest_pos = dest_info->pos;

      // Insert the edge kind entry and keep a handle to update
      // its classification.
      EdgeKind& edge_kind = src_info->succ_edge[dest];

      if (src_pos >= dest_pos) {
        // This is a backedge.
        edge_kind = EdgeKind::kBack;
        num_backedges++;
        const auto* continue_construct = src_construct.enclosing_continue;
        if (!continue_construct) {
          return Fail() << "Invalid backedge (" << src << "->" << dest
                        << "): " << src << " is not in a continue construct";
        }
        if (src_pos != continue_construct->end_pos - 1) {
          return Fail() << "Invalid exit (" << src << "->" << dest
                        << ") from continue construct: " << src
                        << " is not the last block in the continue construct "
                           "starting at "
                        << src_construct.begin_id
                        << " (violates post-dominance rule)";
        }
        const auto* ct_info = GetBlockInfo(continue_construct->begin_id);
        assert(ct_info);
        if (ct_info->header_for_continue != dest) {
          return Fail()
                 << "Invalid backedge (" << src << "->" << dest
                 << "): does not branch to the corresponding loop header, "
                    "expected "
                 << ct_info->header_for_continue;
        }
      } else {
        // This is a forward edge.
        // For now, classify it that way, but we might update it.
        edge_kind = EdgeKind::kForward;

        // Exit from a continue construct can only be from the last block.
        const auto* continue_construct = src_construct.enclosing_continue;
        if (continue_construct != nullptr) {
          if (continue_construct->ContainsPos(src_pos) &&
              !continue_construct->ContainsPos(dest_pos) &&
              (src_pos != continue_construct->end_pos - 1)) {
            return Fail() << "Invalid exit (" << src << "->" << dest
                          << ") from continue construct: " << src
                          << " is not the last block in the continue construct "
                             "starting at "
                          << continue_construct->begin_id
                          << " (violates post-dominance rule)";
          }
        }

        // Check valid structured exit cases.

        if (edge_kind == EdgeKind::kForward) {
          // Check for a 'break' from a loop or from a switch.
          const auto* breakable_header = HeaderIfBreakable(
              src_construct.enclosing_loop_or_continue_or_switch);
          if (breakable_header != nullptr) {
            if (dest == breakable_header->merge_for_header) {
              // It's a break.
              edge_kind = (breakable_header->construct->kind ==
                           Construct::kSwitchSelection)
                              ? EdgeKind::kSwitchBreak
                              : EdgeKind::kLoopBreak;
            }
          }
        }

        if (edge_kind == EdgeKind::kForward) {
          // Check for a 'continue' from within a loop.
          const auto* loop_header =
              HeaderIfBreakable(src_construct.enclosing_loop);
          if (loop_header != nullptr) {
            if (dest == loop_header->continue_for_header) {
              // It's a continue.
              edge_kind = EdgeKind::kLoopContinue;
            }
          }
        }

        if (edge_kind == EdgeKind::kForward) {
          const auto& header_info = *GetBlockInfo(src_construct.begin_id);
          if (dest == header_info.merge_for_header) {
            // Branch to construct's merge block.  The loop break and
            // switch break cases have already been covered.
            edge_kind = EdgeKind::kIfBreak;
          }
        }

        // A forward edge into a case construct that comes from something
        // other than the OpSwitch is actually a fallthrough.
        if (edge_kind == EdgeKind::kForward) {
          const auto* switch_construct =
              (dest_info->case_head_for ? dest_info->case_head_for
                                        : dest_info->default_head_for);
          if (switch_construct != nullptr) {
            if (src != switch_construct->begin_id) {
              edge_kind = EdgeKind::kCaseFallThrough;
            }
          }
        }

        // The edge-kind has been finalized.

        if ((edge_kind == EdgeKind::kForward) ||
            (edge_kind == EdgeKind::kCaseFallThrough) ||
            (edge_kind == EdgeKind::kIfBreak)) {
          normal_forward_edges.push_back(dest);
        }

        if ((edge_kind == EdgeKind::kForward) ||
            (edge_kind == EdgeKind::kCaseFallThrough)) {
          // Check for an invalid forward exit out of this construct.
          if (dest_info->pos >= src_construct.end_pos) {
            // In most cases we're bypassing the merge block for the source
            // construct.
            auto end_block = src_construct.end_id;
            const char* end_block_desc = "merge block";
            if (src_construct.kind == Construct::kLoop) {
              // For a loop construct, we have two valid places to go: the
              // continue target or the merge for the loop header, which is
              // further down.
              const auto loop_merge =
                  GetBlockInfo(src_construct.begin_id)->merge_for_header;
              if (dest_info->pos >= GetBlockInfo(loop_merge)->pos) {
                // We're bypassing the loop's merge block.
                end_block = loop_merge;
              } else {
                // We're bypassing the loop's continue target, and going into
                // the middle of the continue construct.
                end_block_desc = "continue target";
              }
            }
            return Fail()
                   << "Branch from block " << src << " to block " << dest
                   << " is an invalid exit from construct starting at block "
                   << src_construct.begin_id << "; branch bypasses "
                   << end_block_desc << " " << end_block;
          }

          // Check dominance.

          //      Look for edges that violate the dominance condition: a branch
          //      from X to Y where:
          //        If Y is in a nearest enclosing continue construct headed by
          //        CT:
          //          Y is not CT, and
          //          In the structured order, X appears before CT order or
          //          after CT's backedge block.
          //        Otherwise, if Y is in a nearest enclosing construct
          //        headed by H:
          //          Y is not H, and
          //          In the structured order, X appears before H or after H's
          //          merge block.

          const auto& dest_construct = *(dest_info->construct);
          if (dest != dest_construct.begin_id &&
              !dest_construct.ContainsPos(src_pos)) {
            return Fail() << "Branch from " << src << " to " << dest
                          << " bypasses "
                          << (dest_construct.kind == Construct::kContinue
                                  ? "continue target "
                                  : "header ")
                          << dest_construct.begin_id
                          << " (dominance rule violated)";
          }
        }
      }  // end forward edge
    }    // end successor

    if (num_backedges > 1) {
      return Fail() << "Block " << src
                    << " has too many backedges: " << num_backedges;
    }
    if ((normal_forward_edges.size() > 1) &&
        (src_info->merge_for_header == 0)) {
      return Fail() << "Control flow diverges at block " << src << " (to "
                    << normal_forward_edges[0] << ", "
                    << normal_forward_edges[1]
                    << ") but it is not a structured header (it has no merge "
                       "instruction)";
    }
  }

  return success();
}

bool FunctionEmitter::FindIfSelectionInternalHeaders() {
  if (failed()) {
    return false;
  }
  for (auto& construct : constructs_) {
    if (construct->kind != Construct::kIfSelection) {
      continue;
    }
    auto* if_header_info = GetBlockInfo(construct->begin_id);
    const auto* branch = if_header_info->basic_block->terminator();
    const auto true_head = branch->GetSingleWordInOperand(1);
    const auto false_head = branch->GetSingleWordInOperand(2);

    auto* true_head_info = GetBlockInfo(true_head);
    auto* false_head_info = GetBlockInfo(false_head);
    const auto true_head_pos = true_head_info->pos;
    const auto false_head_pos = false_head_info->pos;

    const bool contains_true = construct->ContainsPos(true_head_pos);
    const bool contains_false = construct->ContainsPos(false_head_pos);

    if (contains_true) {
      if_header_info->true_head = true_head;
    }
    if (contains_false) {
      if_header_info->false_head = false_head;
    }

    if ((true_head_info->header_for_merge != 0) &&
        (true_head_info->header_for_merge != construct->begin_id)) {
      // The OpBranchConditional instruction for the true head block is an
      // alternate path to the merge block, and hence the merge block is not
      // dominated by its own (different) header.
      return Fail() << "Block " << true_head
                    << " is the true branch for if-selection header "
                    << construct->begin_id << " and also the merge block for header block "
                    << true_head_info->header_for_merge
                    << " (violates dominance rule)";
    }
    if ((false_head_info->header_for_merge != 0) &&
        (false_head_info->header_for_merge != construct->begin_id)) {
      // The OpBranchConditional instruction for the false head block is an
      // alternate path to the merge block, and hence the merge block is not
      // dominated by its own (different) header.
      return Fail() << "Block " << false_head
                    << " is the false branch for if-selection header "
                    << construct->begin_id << " and also the merge block for header block "
                    << false_head_info->header_for_merge
                    << " (violates dominance rule)";
    }

    if (contains_true && contains_false && (true_head_pos != false_head_pos)) {
      // This construct has both a "then" clause and an "else" clause.
      //
      // We have this structure:
      //
      //   Option 1:
      //
      //     * condbranch
      //        * true-head (start of then-clause)
      //        ...
      //        * end-then-clause
      //        * false-head (start of else-clause)
      //        ...
      //        * end-false-clause
      //        * premerge-head
      //        ...
      //     * selection merge
      //
      //   Option 2:
      //
      //     * condbranch
      //        * true-head (start of then-clause)
      //        ...
      //        * end-then-clause
      //        * false-head (start of else-clause) and also premerge-head
      //        ...
      //        * end-false-clause
      //     * selection merge
      //
      //   Option 3:
      //
      //     * condbranch
      //        * false-head (start of else-clause)
      //        ...
      //        * end-else-clause
      //        * true-head (start of then-clause) and also premerge-head
      //        ...
      //        * end-then-clause
      //     * selection merge
      //
      // The premerge-head exists if there is a kForward branch from the end
      // of the first clause to a block within the surrounding selection.
      // The first clause might be a then-clause or an else-clause.
      const auto second_head = std::max(true_head_pos, false_head_pos);
      const auto end_first_clause_pos = second_head - 1;
      assert(end_first_clause_pos < block_order_.size());
      const auto end_first_clause = block_order_[end_first_clause_pos];
      uint32_t premerge_id = 0;
      uint32_t if_break_id = 0;
      for (auto& then_succ_iter : GetBlockInfo(end_first_clause)->succ_edge) {
        const uint32_t dest_id = then_succ_iter.first;
        const auto edge_kind = then_succ_iter.second;
        switch (edge_kind) {
          case EdgeKind::kIfBreak:
            if_break_id = dest_id;
            break;
          case EdgeKind::kForward: {
            if (construct->ContainsPos(GetBlockInfo(dest_id)->pos)) {
              // It's a premerge.
              if (premerge_id != 0) {
                // TODO(dneto): I think this is impossible to trigger at this
                // point in the flow. It would require a merge instruction to
                // get past the check of "at-most-one-forward-edge".
                return Fail()
                       << "invalid structure: then-clause headed by block "
                       << true_head << " ending at block " << end_first_clause
                       << " has two forward edges to within selection"
                       << " going to " << premerge_id << " and " << dest_id;
              }
              premerge_id = dest_id;
              auto* dest_block_info = GetBlockInfo(dest_id);
              if_header_info->premerge_head = dest_id;
              if (dest_block_info->header_for_merge != 0) {
                // Premerge has two edges coming into it, from the then-clause
                // and the else-clause. It's also, by construction, not the
                // merge block of the if-selection.  So it must not be a merge
                // block itself. The OpBranchConditional instruction for the
                // false head block is an alternate path to the merge block, and
                // hence the merge block is not dominated by its own (different)
                // header.
                return Fail()
                       << "Block " << premerge_id
                       << " is the merge block for " << dest_block_info->header_for_merge
                       << " but has alternate paths reaching it, starting from"
                       << " blocks " << true_head << " and " << false_head
                       << " which are the true and false branches for the"
                       << " if-selection header block " << construct->begin_id
                       << " (violates dominance rule)";
              }
            }
            break;
          }
          default:
            break;
        }
      }
      if (if_break_id != 0 && premerge_id != 0) {
        return Fail() << "Block " << end_first_clause
                      << " in if-selection headed at block "
                      << construct->begin_id
                      << " branches to both the merge block " << if_break_id
                      << " and also to block " << premerge_id
                      << " later in the selection";
      }
    }
  }
  return success();
}

bool FunctionEmitter::EmitFunctionVariables() {
  if (failed()) {
    return false;
  }
  for (auto& inst : *function_.entry()) {
    if (inst.opcode() != SpvOpVariable) {
      continue;
    }
    auto* var_store_type = GetVariableStoreType(inst);
    if (failed()) {
      return false;
    }
    auto var = parser_impl_.MakeVariable(
        inst.result_id(), ast::StorageClass::kFunction, var_store_type);
    if (inst.NumInOperands() > 1) {
      // SPIR-V initializers are always constants.
      // (OpenCL also allows the ID of an OpVariable, but we don't handle that
      // here.)
      var->set_constructor(
          parser_impl_.MakeConstantExpression(inst.GetSingleWordInOperand(1))
              .expr);
    }
    // TODO(dneto): Add the initializer via Variable::set_constructor.
    auto var_decl_stmt =
        std::make_unique<ast::VariableDeclStatement>(std::move(var));
    AddStatement(std::move(var_decl_stmt));
    // Save this as an already-named value.
    identifier_values_.insert(inst.result_id());
  }
  return success();
}

TypedExpression FunctionEmitter::MakeExpression(uint32_t id) {
  if (failed()) {
    return {};
  }
  if (identifier_values_.count(id)) {
    return TypedExpression(
        parser_impl_.ConvertType(def_use_mgr_->GetDef(id)->type_id()),
        std::make_unique<ast::IdentifierExpression>(namer_.Name(id)));
  }
  if (singly_used_values_.count(id)) {
    auto expr = std::move(singly_used_values_[id]);
    singly_used_values_.erase(id);
    return expr;
  }
  const auto* spirv_constant = constant_mgr_->FindDeclaredConstant(id);
  if (spirv_constant) {
    return parser_impl_.MakeConstantExpression(id);
  }
  const auto* inst = def_use_mgr_->GetDef(id);
  if (inst == nullptr) {
    Fail() << "ID " << id << " does not have a defining SPIR-V instruction";
    return {};
  }
  switch (inst->opcode()) {
    case SpvOpVariable:
      // This occurs for module-scope variables.
      return TypedExpression(parser_impl_.ConvertType(inst->type_id()),
                             std::make_unique<ast::IdentifierExpression>(
                                 namer_.Name(inst->result_id())));
    default:
      break;
  }
  Fail() << "unhandled expression for ID " << id << "\n" << inst->PrettyPrint();
  return {};
}

bool FunctionEmitter::EmitFunctionBodyStatements() {
  // Dump the basic blocks in order, grouped by construct.

  // We maintain a stack of StatementBlock objects, where new statements
  // are always written to the topmost entry of the stack. By this point in
  // processing, we have already recorded the interesting control flow
  // boundaries in the BlockInfo and associated Construct objects. As we
  // enter a new statement grouping, we push onto the stack, and also schedule
  // the statement block's completion and removal at a future block's ID.

  // Upon entry, the statement stack has one entry representing the whole
  // function.
  assert(!constructs_.empty());
  Construct* function_construct = constructs_[0].get();
  assert(function_construct != nullptr);
  assert(function_construct->kind == Construct::kFunction);
  // Make the first entry valid by filling in the construct field, which
  // had not been computed at the time the entry was first created.
  // TODO(dneto): refactor how the first construct is created vs.
  // this statements stack entry is populated.
  assert(statements_stack_.size() == 1);
  statements_stack_[0].construct_ = function_construct;

  for (auto block_id : block_order()) {
    if (!EmitBasicBlock(*GetBlockInfo(block_id))) {
      return false;
    }
  }
  return success();
}

bool FunctionEmitter::EmitBasicBlock(const BlockInfo& block_info) {
  // Close off previous constructs.
  while (!statements_stack_.empty() &&
         (statements_stack_.back().end_id_ == block_info.id)) {
    StatementBlock& sb = statements_stack_.back();
    sb.completion_action_(&sb);
    statements_stack_.pop_back();
  }
  if (statements_stack_.empty()) {
    return Fail() << "internal error: statements stack empty at block "
                  << block_info.id;
  }

  // Enter new constructs.

  std::vector<const Construct*> entering_constructs;  // inner most comes first
  {
    auto* here = block_info.construct;
    auto* const top_construct = statements_stack_.back().construct_;
    while (here != top_construct) {
      // Only enter a construct at its header block.
      if (here->begin_id == block_info.id) {
        entering_constructs.push_back(here);
      }
      here = here->parent;
    }
  }
  // What constructs can we have entered?
  // - It can't be kFunction, because there is only one of those, and it was
  //   already on the stack at the outermost level.
  // - We have at most one of kIfSelection, kSwitchSelection, or kLoop because
  //   each of those is headed by a block with a merge instruction, and the
  //   kIfSelection and kSwitchSelection header blocks end in different branch
  //   instructions.
  // - A kContinue can contain a kContinue
  //   This is possible in Vulkan SPIR-V, but Tint disallows this by the rule
  //   that a block can be continue target for at most one header block. See
  //   test DISABLED_BlockIsContinueForMoreThanOneHeader. If we generalize this,
  //   then by a dominance argument, the inner loop continue target can only be
  //   a single-block loop.
  //   TODO(dneto): Handle this case.
  // - All that's left is a kContinue and one of kIfSelection, kSwitchSelection,
  //   kLoop.
  //
  //   The kContinue can be the parent of the other.  For example, a selection
  //   starting at the first block of a continue construct.
  //
  //   The kContinue can't be the child of the other because either:
  //     - Either it would be a single block loop but in that case there is no
  //       kLoop construct for it, by construction.
  //     - The kContinue is in a loop that is not single-block; and the
  //       selection contains the kContinue block but not the loop block. That
  //       breaks dominance rules. That is, the continue target is dominated by
  //       that loop header, and so gets found on the outside before the
  //       selection is found. The selection is inside the outer loop.
  //
  // So we fall into one of the following cases:
  //  - We are entering 0 or 1 constructs, or
  //  - We are entering 2 constructs, with the outer one being a kContinue, the
  //    inner one is not a continue.
  if (entering_constructs.size() > 2) {
    return Fail() << "internal error: bad construct nesting found";
  }
  if (entering_constructs.size() == 2) {
    auto inner_kind = entering_constructs[0]->kind;
    auto outer_kind = entering_constructs[1]->kind;
    if (outer_kind != Construct::kContinue) {
      return Fail() << "internal error: bad construct nesting. Only Continue "
                       "construct can be outer construct on same block.  Got "
                       "outer kind "
                    << int(outer_kind) << " inner kind " << int(inner_kind);
    }
    if (inner_kind == Construct::kContinue) {
      return Fail() << "internal error: unsupported construct nesting: "
                       "Continue around Continue";
    }
    if (inner_kind != Construct::kIfSelection &&
        inner_kind != Construct::kSwitchSelection &&
        inner_kind != Construct::kLoop) {
      return Fail() << "internal error: bad construct nesting. Continue around "
                       "something other than if, switch, or loop";
    }
  }

  // Enter constructs from outermost to innermost.
  // kLoop and kContinue push a new statement-block onto the stack before
  // emitting statements in the block.
  // kIfSelection and kSwitchSelection emit statements in the block and then
  // emit push a new statement-block. Only emit the statements in the block
  // once.

  // Have we emitted the statements for this block?
  bool emitted = false;

  // When entering an if-selection or switch-selection, we will emit the WGSL
  // construct to cause the divergent branching.  But otherwise, we will
  // emit a "normal" block terminator, which occurs at the end of this method.
  bool has_normal_terminator = true;

  for (auto iter = entering_constructs.rbegin();
       iter != entering_constructs.rend(); ++iter) {
    const Construct* construct = *iter;

    switch (construct->kind) {
      case Construct::kFunction:
        return Fail() << "internal error: nested function construct";

      case Construct::kLoop:
        if (!EmitLoopStart(construct)) {
          return false;
        }
        if (!EmitStatementsInBasicBlock(block_info, &emitted)) {
          return false;
        }
        break;

      case Construct::kContinue:
        if (block_info.is_single_block_loop) {
          if (!EmitLoopStart(construct)) {
            return false;
          }
          if (!EmitStatementsInBasicBlock(block_info, &emitted)) {
            return false;
          }
        } else {
          if (!EmitContinuingStart(construct)) {
            return false;
          }
        }
        break;

      case Construct::kIfSelection:
        if (!EmitStatementsInBasicBlock(block_info, &emitted)) {
          return false;
        }
        if (!EmitIfStart(block_info)) {
          return false;
        }
        has_normal_terminator = false;
        break;

      case Construct::kSwitchSelection:
        has_normal_terminator = false;
        return Fail() << "unhandled: switch construct";
    }
  }

  // If we aren't starting or transitioning, then emit the normal
  // statements now.
  if (!EmitStatementsInBasicBlock(block_info, &emitted)) {
    return false;
  }

  if (has_normal_terminator) {
    if (!EmitNormalTerminator(block_info)) {
      return false;
    }
  }
  return success();
}

bool FunctionEmitter::EmitIfStart(const BlockInfo& block_info) {
  // The block is the if-header block.  So its construct is the if construct.
  auto* construct = block_info.construct;
  assert(construct->kind == Construct::kIfSelection);
  assert(construct->begin_id == block_info.id);

  const uint32_t false_head = block_info.false_head;
  const uint32_t premerge_head = block_info.premerge_head;

  auto* const if_stmt =
      AddStatement(std::make_unique<ast::IfStatement>())->AsIf();
  const auto condition_id =
      block_info.basic_block->terminator()->GetSingleWordInOperand(0);
  // Generate the code for the condition.
  if_stmt->set_condition(std::move(MakeExpression(condition_id).expr));

  // Compute the block IDs that should end the then-clause and the else-clause.

  // We need to know where the *emitted* selection should end, i.e. the intended
  // merge block id.  That should be the current premerge block, if it exists,
  // or otherwise the declared merge block.
  //
  // This is another way to think about it:
  //   If there is a premerge, then there are three cases:
  //    - premerge_head is different from the true_head and false_head:
  //      - Premerge comes last. In effect, move the selection merge up
  //        to where the premerge begins.
  //    - premerge_head is the same as the false_head
  //      - This is really an if-then without an else clause.
  //        Move the merge up to where the premerge is.
  //    - premerge_head is the same as the true_head
  //      - This is really an if-else without an then clause.
  //        Emit it as:   if (cond) {} else {....}
  //        Move the merge up to where the premerge is.
  const uint32_t intended_merge =
      premerge_head ? premerge_head : construct->end_id;

  // then-clause:
  //   If true_head exists:
  //     spans from true head to the earlier of the false head (if it exists)
  //     or the selection merge.
  //   Otherwise:
  //     ends at from the false head (if it exists), otherwise the selection
  //     end.
  const uint32_t then_end = false_head ? false_head : intended_merge;

  // else-clause:
  //   ends at the premerge head (if it exists) or at the selection end.
  const uint32_t else_end = premerge_head ? premerge_head : intended_merge;

  // Push statement blocks for the then-clause and the else-clause.
  // But make sure we do it in the right order.

  auto push_then = [this, if_stmt, then_end, construct]() {
    // Push the then clause onto the stack.
    PushNewStatementBlock(construct, then_end, [if_stmt](StatementBlock* s) {
      // The "then" consists of the statement list
      // from the top of statments stack, without an
      // elseif condition.
      if_stmt->set_body(std::move(s->statements_));
    });
  };

  auto push_else = [this, if_stmt, else_end, construct]() {
    // Push the else clause onto the stack first.
    PushNewStatementBlock(construct, else_end, [if_stmt](StatementBlock* s) {
      // Only set the else-clause if there are statements to fill it.
      if (!s->statements_.empty()) {
        // The "else" consists of the statement list from the top of statments
        // stack, without an elseif condition.
        ast::ElseStatementList else_stmts;
        else_stmts.emplace_back(std::make_unique<ast::ElseStatement>(
            nullptr, std::move(s->statements_)));
        if_stmt->set_else_statements(std::move(else_stmts));
      }
    });
  };

  if (GetBlockInfo(else_end)->pos < GetBlockInfo(then_end)->pos) {
    // Process the else-clause first.  The then-clause will be empty so avoid
    // pushing onto the stack at all.
    push_else();
  } else {
    // Blocks for the then-clause appear before blocks for the else-clause.
    // So push the else-clause handling onto the stack first. The else-clause
    // might be empty, but this works anyway.
    push_else();
    push_then();
  }

  return success();
}

bool FunctionEmitter::EmitLoopStart(const Construct* construct) {
  auto* loop = AddStatement(std::make_unique<ast::LoopStatement>())->AsLoop();
  PushNewStatementBlock(
      construct, construct->end_id,
      [loop](StatementBlock* s) { loop->set_body(std::move(s->statements_)); });
  return success();
}

bool FunctionEmitter::EmitContinuingStart(const Construct* construct) {
  // A continue construct has the same depth as its associated loop
  // construct. Start a continue construct.
  auto* loop_candidate = LastStatement();
  if (!loop_candidate->IsLoop()) {
    return Fail() << "internal error: starting continue construct, "
                     "expected loop on top of stack";
  }
  auto* loop = loop_candidate->AsLoop();
  PushNewStatementBlock(construct, construct->end_id,
                        [loop](StatementBlock* s) {
                          loop->set_continuing(std::move(s->statements_));
                        });
  return success();
}

bool FunctionEmitter::EmitNormalTerminator(const BlockInfo& block_info) {
  const auto& terminator = *(block_info.basic_block->terminator());
  switch (terminator.opcode()) {
    case SpvOpReturn:
      AddStatement(std::make_unique<ast::ReturnStatement>());
      return true;
    case SpvOpReturnValue: {
      auto value = MakeExpression(terminator.GetSingleWordInOperand(0));
      AddStatement(
          std::make_unique<ast::ReturnStatement>(std::move(value.expr)));
    }
      return true;
    case SpvOpKill:
      // For now, assume SPIR-V OpKill has same semantics as WGSL kill.
      // TODO(dneto): https://github.com/gpuweb/gpuweb/issues/676
      AddStatement(std::make_unique<ast::KillStatement>());
      return true;
    case SpvOpUnreachable:
      // Translate as if it's a return. This avoids the problem where WGSL
      // requires a return statement at the end of the function body.
      {
        const auto* result_type = type_mgr_->GetType(function_.type_id());
        if (result_type->AsVoid() != nullptr) {
          AddStatement(std::make_unique<ast::ReturnStatement>());
        } else {
          auto* ast_type = parser_impl_.ConvertType(function_.type_id());
          AddStatement(std::make_unique<ast::ReturnStatement>(
              parser_impl_.MakeNullValue(ast_type)));
        }
      }
      return true;
    case SpvOpBranch: {
      const auto dest_id = terminator.GetSingleWordInOperand(0);
      AddStatement(MakeBranch(block_info, *GetBlockInfo(dest_id)));
      return true;
    }
    case SpvOpBranchConditional: {
      // If both destinations are the same, then do the same as we would
      // for an unconditional branch (OpBranch).
      const auto true_dest = terminator.GetSingleWordInOperand(1);
      const auto false_dest = terminator.GetSingleWordInOperand(2);
      if (true_dest == false_dest) {
        // This is like an uncondtional branch.
        AddStatement(MakeBranch(block_info, *GetBlockInfo(true_dest)));
        return true;
      }

      const EdgeKind true_kind = block_info.succ_edge.find(true_dest)->second;
      const EdgeKind false_kind = block_info.succ_edge.find(false_dest)->second;
      auto* const true_info = GetBlockInfo(true_dest);
      auto* const false_info = GetBlockInfo(false_dest);
      auto cond = MakeExpression(terminator.GetSingleWordInOperand(0)).expr;

      // We have two distinct destinations. But we only get here if this
      // is a normal terminator; in particular the source block is *not* the
      // start of an if-selection or a switch-selection.  So at most one branch
      // is a kForward, kCaseFallThrough, or kIfBreak.

      // The fallthrough case is special because WGSL requires the fallthrough
      // statement to be last in the case clause.
      if (true_kind == EdgeKind::kCaseFallThrough ||
          false_kind == EdgeKind::kCaseFallThrough) {
        return Fail() << "fallthrough is unhandled";
      }

      // At this point, at most one edge is kForward or kIfBreak.

      // Emit an 'if' statement to express the *other* branch as a conditional
      // break or continue.  Either or both of these could be nullptr.
      // (A nullptr is generated for kIfBreak, kForward, or kBack.)
      auto true_branch = MakeBranch(block_info, *true_info);
      auto false_branch = MakeBranch(block_info, *false_info);

      AddStatement(MakeSimpleIf(std::move(cond), std::move(true_branch),
                                std::move(false_branch)));
      return true;
    }
    case SpvOpSwitch:
      // TODO(dneto)
      break;
    default:
      break;
  }
  // TODO(dneto): emit fallthrough
  return success();
}

std::unique_ptr<ast::Statement> FunctionEmitter::MakeBranch(
    const BlockInfo& src_info,
    const BlockInfo& dest_info) const {
  auto kind = src_info.succ_edge.find(dest_info.id)->second;
  switch (kind) {
    case EdgeKind::kBack:
      // Nothing to do. The loop backedge is implicit.
      break;
    case EdgeKind::kSwitchBreak:
    case EdgeKind::kLoopBreak:
      return std::make_unique<ast::BreakStatement>();
    case EdgeKind::kLoopContinue:
      // An unconditional continue to the next block is redundant and ugly.
      // Skip it in that case.
      if (dest_info.pos == 1 + src_info.pos) {
        break;
      }
      // Otherwise, emit a regular continue statement.
      return std::make_unique<ast::ContinueStatement>();
    case EdgeKind::kIfBreak:
      // For an unconditional branch, the break out to an if-selection
      // merge block is implicit.
      break;
    case EdgeKind::kCaseFallThrough:
      return std::make_unique<ast::FallthroughStatement>();
    case EdgeKind::kForward:
      // Unconditional forward branch is implicit.
      break;
  }
  return {nullptr};
}

std::unique_ptr<ast::Statement> FunctionEmitter::MakeSimpleIf(
    std::unique_ptr<ast::Expression> condition,
    std::unique_ptr<ast::Statement> then_stmt,
    std::unique_ptr<ast::Statement> else_stmt) const {
  if ((then_stmt == nullptr) && (else_stmt == nullptr)) {
    return nullptr;
  }
  auto if_stmt = std::make_unique<ast::IfStatement>();
  if_stmt->set_condition(std::move(condition));
  if (then_stmt != nullptr) {
    ast::StatementList stmts;
    stmts.emplace_back(std::move(then_stmt));
    if_stmt->set_body(std::move(stmts));
  }
  if (else_stmt != nullptr) {
    ast::StatementList stmts;
    stmts.emplace_back(std::move(else_stmt));
    ast::ElseStatementList else_stmts;
    else_stmts.emplace_back(
        std::make_unique<ast::ElseStatement>(nullptr, std::move(stmts)));
    if_stmt->set_else_statements(std::move(else_stmts));
  }
  return if_stmt;
}

bool FunctionEmitter::EmitStatementsInBasicBlock(const BlockInfo& block_info,
                                                 bool* already_emitted) {
  if (*already_emitted) {
    // Only emit this part of the basic block once.
    return true;
  }
  const spvtools::opt::BasicBlock& bb = *(block_info.basic_block);
  const auto* terminator = bb.terminator();
  const auto* merge = bb.GetMergeInst();  // Might be nullptr
  // Emit regular statements.
  for (auto& inst : bb) {
    if (&inst == terminator || &inst == merge || inst.opcode() == SpvOpLabel ||
        inst.opcode() == SpvOpVariable) {
      continue;
    }
    if (!EmitStatement(inst)) {
      return false;
    }
  }
  *already_emitted = true;
  return true;
}

bool FunctionEmitter::EmitConstDefinition(
    const spvtools::opt::Instruction& inst,
    TypedExpression ast_expr) {
  if (!ast_expr.expr) {
    return false;
  }
  auto ast_const =
      parser_impl_.MakeVariable(inst.result_id(), ast::StorageClass::kNone,
                                parser_impl_.ConvertType(inst.type_id()));
  if (!ast_const) {
    return false;
  }
  ast_const->set_constructor(std::move(ast_expr.expr));
  ast_const->set_is_const(true);
  AddStatement(
      std::make_unique<ast::VariableDeclStatement>(std::move(ast_const)));
  // Save this as an already-named value.
  identifier_values_.insert(inst.result_id());
  return success();
}

bool FunctionEmitter::EmitStatement(const spvtools::opt::Instruction& inst) {
  // Handle combinatorial instructions first.
  auto combinatorial_expr = MaybeEmitCombinatorialValue(inst);
  if (combinatorial_expr.expr != nullptr) {
    if (def_use_mgr_->NumUses(&inst) == 1) {
      // If it's used once, then defer emitting the expression until it's
      // used. Any supporting statements have already been emitted.
      singly_used_values_.insert(
          std::make_pair(inst.result_id(), std::move(combinatorial_expr)));
      return success();
    }
    // Otherwise, generate a const definition for it now and later use
    // the const's name at the uses of the value.
    return EmitConstDefinition(inst, std::move(combinatorial_expr));
  }
  if (failed()) {
    return false;
  }

  switch (inst.opcode()) {
    case SpvOpStore: {
      // TODO(dneto): Order of evaluation?
      auto lhs = MakeExpression(inst.GetSingleWordInOperand(0));
      auto rhs = MakeExpression(inst.GetSingleWordInOperand(1));
      AddStatement(std::make_unique<ast::AssignmentStatement>(
          std::move(lhs.expr), std::move(rhs.expr)));
      return success();
    }
    case SpvOpLoad:
      // Memory accesses must be issued in SPIR-V program order.
      // So represent a load by a new const definition.
      return EmitConstDefinition(
          inst, MakeExpression(inst.GetSingleWordInOperand(0)));
    case SpvOpFunctionCall:
      // TODO(dneto): Fill this out.  Make this pass, for existing tests
      return success();
    default:
      break;
  }
  return Fail() << "unhandled instruction with opcode " << inst.opcode();
}

TypedExpression FunctionEmitter::MakeOperand(
    const spvtools::opt::Instruction& inst,
    uint32_t operand_index) {
  auto expr = this->MakeExpression(inst.GetSingleWordInOperand(operand_index));
  return parser_impl_.RectifyOperandSignedness(inst.opcode(), std::move(expr));
}

TypedExpression FunctionEmitter::MaybeEmitCombinatorialValue(
    const spvtools::opt::Instruction& inst) {
  if (inst.result_id() == 0) {
    return {};
  }

  const auto opcode = inst.opcode();

  ast::type::Type* ast_type =
      inst.type_id() != 0 ? parser_impl_.ConvertType(inst.type_id()) : nullptr;

  auto binary_op = ConvertBinaryOp(opcode);
  if (binary_op != ast::BinaryOp::kNone) {
    auto arg0 = MakeOperand(inst, 0);
    auto arg1 = MakeOperand(inst, 1);
    auto binary_expr = std::make_unique<ast::BinaryExpression>(
        binary_op, std::move(arg0.expr), std::move(arg1.expr));
    auto* forced_result_ty = parser_impl_.ForcedResultType(opcode, arg0.type);
    if (forced_result_ty && forced_result_ty != ast_type) {
      return {ast_type, std::make_unique<ast::AsExpression>(
                            ast_type, std::move(binary_expr))};
    }
    return {ast_type, std::move(binary_expr)};
  }

  auto unary_op = ast::UnaryOp::kNegation;
  if (GetUnaryOp(opcode, &unary_op)) {
    auto arg0 = MakeOperand(inst, 0);
    auto unary_expr = std::make_unique<ast::UnaryOpExpression>(
        unary_op, std::move(arg0.expr));
    auto* forced_result_ty = parser_impl_.ForcedResultType(opcode, arg0.type);
    if (forced_result_ty && forced_result_ty != ast_type) {
      return {ast_type, std::make_unique<ast::AsExpression>(
                            ast_type, std::move(unary_expr))};
    }
    return {ast_type, std::move(unary_expr)};
  }

  if (opcode == SpvOpAccessChain || opcode == SpvOpInBoundsAccessChain) {
    return MakeAccessChain(inst);
  }

  if (opcode == SpvOpBitcast) {
    return {ast_type, std::make_unique<ast::AsExpression>(
                          ast_type, MakeOperand(inst, 0).expr)};
  }

  auto negated_op = NegatedFloatCompare(opcode);
  if (negated_op != ast::BinaryOp::kNone) {
    auto arg0 = MakeOperand(inst, 0);
    auto arg1 = MakeOperand(inst, 1);
    auto binary_expr = std::make_unique<ast::BinaryExpression>(
        negated_op, std::move(arg0.expr), std::move(arg1.expr));
    auto negated_expr = std::make_unique<ast::UnaryOpExpression>(
        ast::UnaryOp::kNot, std::move(binary_expr));
    return {ast_type, std::move(negated_expr)};
  }

  // builtin readonly function
  // glsl.std.450 readonly function

  // Instructions:
  //    OpCopyObject
  //    OpUndef
  //    OpBitcast
  //    OpSatConvertSToU
  //    OpSatConvertUToS
  //    OpConvertFToS
  //    OpConvertFToU
  //    OpConvertSToF
  //    OpConvertUToF
  //    OpUConvert // Only needed when multiple widths supported
  //    OpSConvert // Only needed when multiple widths supported
  //    OpFConvert // Only needed when multiple widths supported
  //    OpConvertPtrToU // Not in WebGPU
  //    OpConvertUToPtr // Not in WebGPU
  //    OpPtrCastToGeneric // Not in Vulkan
  //    OpGenericCastToPtr // Not in Vulkan
  //    OpGenericCastToPtrExplicit // Not in Vulkan
  //
  //    OpArrayLength
  //    OpVectorExtractDynamic
  //    OpVectorInsertDynamic
  //    OpCompositeExtract
  //    OpCompositeInsert

  return {};
}

TypedExpression FunctionEmitter::MakeAccessChain(
    const spvtools::opt::Instruction& inst) {
  if (inst.NumInOperands() < 1) {
    // Binary parsing will fail on this anyway.
    Fail() << "invalid access chain: has no input operands";
    return {};
  }

  // A SPIR-V access chain is a single instruction with multiple indices
  // walking down into composites.  The Tint AST represents this as
  // ever-deeper nested indexing expresions. Start off with an expression
  // for the base, and then bury that inside nested indexing expressions.
  TypedExpression current_expr(MakeOperand(inst, 0));

  const auto constants = constant_mgr_->GetOperandConstants(&inst);
  static const char* swizzles[] = {"x", "y", "z", "w"};

  const auto base_id = inst.GetSingleWordInOperand(0);
  const auto ptr_ty_id = def_use_mgr_->GetDef(base_id)->type_id();
  const auto* ptr_type = type_mgr_->GetType(ptr_ty_id);
  if (!ptr_type || !ptr_type->AsPointer()) {
    Fail() << "Access chain %" << inst.result_id()
           << " base pointer is not of pointer type";
    return {};
  }
  const auto* pointee_type = ptr_type->AsPointer()->pointee_type();
  const auto num_in_operands = inst.NumInOperands();
  for (uint32_t index = 1; index < num_in_operands; ++index) {
    const auto* index_const =
        constants[index] ? constants[index]->AsIntConstant() : nullptr;
    const int64_t index_const_val =
        index_const ? index_const->GetSignExtendedValue() : 0;
    std::unique_ptr<ast::Expression> next_expr;
    switch (pointee_type->kind()) {
      case spvtools::opt::analysis::Type::kVector:
        if (index_const) {
          // Try generating a MemberAccessor expression.
          if (index_const_val < 0 ||
              pointee_type->AsVector()->element_count() <= index_const_val) {
            Fail() << "Access chain %" << inst.result_id() << " index %"
                   << inst.GetSingleWordInOperand(index) << " value "
                   << index_const_val << " is out of bounds for vector of "
                   << pointee_type->AsVector()->element_count() << " elements";
            return {};
          }
          if (uint64_t(index_const_val) >=
              sizeof(swizzles) / sizeof(swizzles[0])) {
            Fail() << "internal error: swizzle index " << index_const_val
                   << " is too big. Max handled index is "
                   << ((sizeof(swizzles) / sizeof(swizzles[0])) - 1);
          }
          auto letter_index = std::make_unique<ast::IdentifierExpression>(
              swizzles[index_const_val]);
          next_expr = std::make_unique<ast::MemberAccessorExpression>(
              std::move(current_expr.expr), std::move(letter_index));
        } else {
          // Non-constant index. Use array syntax
          next_expr = std::make_unique<ast::ArrayAccessorExpression>(
              std::move(current_expr.expr),
              std::move(MakeOperand(inst, index).expr));
        }
        pointee_type = pointee_type->AsVector()->element_type();
        break;
      case spvtools::opt::analysis::Type::kMatrix:
        // Use array syntax.
        next_expr = std::make_unique<ast::ArrayAccessorExpression>(
            std::move(current_expr.expr),
            std::move(MakeOperand(inst, index).expr));
        pointee_type = pointee_type->AsMatrix()->element_type();
        break;
      case spvtools::opt::analysis::Type::kArray:
        next_expr = std::make_unique<ast::ArrayAccessorExpression>(
            std::move(current_expr.expr),
            std::move(MakeOperand(inst, index).expr));
        pointee_type = pointee_type->AsArray()->element_type();
        break;
      case spvtools::opt::analysis::Type::kRuntimeArray:
        next_expr = std::make_unique<ast::ArrayAccessorExpression>(
            std::move(current_expr.expr),
            std::move(MakeOperand(inst, index).expr));
        pointee_type = pointee_type->AsRuntimeArray()->element_type();
        break;
      case spvtools::opt::analysis::Type::kStruct: {
        if (!index_const) {
          Fail() << "Access chain %" << inst.result_id() << " index %"
                 << inst.GetSingleWordInOperand(index)
                 << " is a non-constant index into a structure %"
                 << type_mgr_->GetId(pointee_type);
          return {};
        }
        if ((index_const_val < 0) ||
            pointee_type->AsStruct()->element_types().size() <=
                uint64_t(index_const_val)) {
          Fail() << "Access chain %" << inst.result_id() << " index value "
                 << index_const_val << " is out of bounds for structure %"
                 << type_mgr_->GetId(pointee_type) << " having "
                 << pointee_type->AsStruct()->element_types().size()
                 << " elements";
          return {};
        }
        auto member_access =
            std::make_unique<ast::IdentifierExpression>(namer_.GetMemberName(
                type_mgr_->GetId(pointee_type), uint32_t(index_const_val)));

        next_expr = std::make_unique<ast::MemberAccessorExpression>(
            std::move(current_expr.expr), std::move(member_access));
        pointee_type =
            pointee_type->AsStruct()->element_types()[index_const_val];
        break;
      }
      default:
        Fail() << "Access chain with unknown pointee type %"
               << type_mgr_->GetId(pointee_type) << " " << pointee_type->str();
        return {};
    }
    current_expr.reset(TypedExpression(
        parser_impl_.ConvertType(type_mgr_->GetId(pointee_type)),
        std::move(next_expr)));
  }
  return current_expr;
}

}  // namespace spirv
}  // namespace reader
}  // namespace tint
