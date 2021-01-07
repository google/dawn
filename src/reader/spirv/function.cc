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

#include <algorithm>
#include <array>
#include <cassert>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "source/opt/basic_block.h"
#include "source/opt/function.h"
#include "source/opt/instruction.h"
#include "source/opt/module.h"
#include "spirv/unified1/GLSL.std.450.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/bool_literal.h"
#include "src/ast/break_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/intrinsic.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/stage_decoration.h"
#include "src/ast/storage_class.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/depth_texture_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/storage_texture_type.h"
#include "src/ast/type/texture_type.h"
#include "src/ast/type/type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
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
//         Pos(H) <= Pos(CT(H))
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
//      Schematically, for a loop construct headed by H that is its own
//      continue construct, the blocks in order from left to right:
//
//                 ...a-b-c H=CT(H) d-e-f M(H) n-o-p...
//
//           where ...a-b-c: blocks before the loop
//           where H is the continue construct; CT(H)=H, and the loop construct
//           is *empty*
//           where d-e-f... are other blocks in the continue construct
//           where M(H) and n-o-p...: blocks after the continue construct
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

constexpr uint32_t kMaxVectorLen = 4;

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

/// Converts a SPIR-V opcode for a WGSL builtin function, if there is a
/// direct translation. Returns nullptr otherwise.
/// @returns the WGSL builtin function name for the given opcode, or nullptr.
const char* GetUnaryBuiltInFunctionName(SpvOp opcode) {
  switch (opcode) {
    case SpvOpAny:
      return "any";
    case SpvOpAll:
      return "all";
    case SpvOpIsNan:
      return "isNan";
    case SpvOpIsInf:
      return "isInf";
    default:
      break;
  }
  return nullptr;
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
    case SpvOpVectorTimesScalar:
    case SpvOpMatrixTimesScalar:
    case SpvOpVectorTimesMatrix:
    case SpvOpMatrixTimesVector:
    case SpvOpMatrixTimesMatrix:
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

// Returns the WGSL standard library function for the given
// GLSL.std.450 extended instruction operation code.  Unknown
// and invalid opcodes map to the empty string.
// @returns the WGSL standard function name, or an empty string.
std::string GetGlslStd450FuncName(uint32_t ext_opcode) {
  switch (ext_opcode) {
    case GLSLstd450FAbs:
    case GLSLstd450SAbs:
      return "abs";
    case GLSLstd450Acos:
      return "acos";
    case GLSLstd450Asin:
      return "asin";
    case GLSLstd450Atan:
      return "atan";
    case GLSLstd450Atan2:
      return "atan2";
    case GLSLstd450Ceil:
      return "ceil";
    case GLSLstd450UClamp:
    case GLSLstd450SClamp:
    case GLSLstd450NClamp:
    case GLSLstd450FClamp:  // FClamp is less prescriptive about NaN operands
      return "clamp";
    case GLSLstd450Cos:
      return "cos";
    case GLSLstd450Cosh:
      return "cosh";
    case GLSLstd450Cross:
      return "cross";
    case GLSLstd450Distance:
      return "distance";
    case GLSLstd450Exp:
      return "exp";
    case GLSLstd450Exp2:
      return "exp2";
    case GLSLstd450FaceForward:
      return "faceForward";
    case GLSLstd450Floor:
      return "floor";
    case GLSLstd450Fma:
      return "fma";
    case GLSLstd450Fract:
      return "fract";
    case GLSLstd450InverseSqrt:
      return "inverseSqrt";
    case GLSLstd450Ldexp:
      return "ldexp";
    case GLSLstd450Length:
      return "length";
    case GLSLstd450Log:
      return "log";
    case GLSLstd450Log2:
      return "log2";
    case GLSLstd450NMax:
    case GLSLstd450FMax:  // FMax is less prescriptive about NaN operands
    case GLSLstd450UMax:
    case GLSLstd450SMax:
      return "max";
    case GLSLstd450NMin:
    case GLSLstd450FMin:  // FMin is less prescriptive about NaN operands
    case GLSLstd450UMin:
    case GLSLstd450SMin:
      return "min";
    case GLSLstd450FMix:
      return "mix";
    case GLSLstd450Normalize:
      return "normalize";
    case GLSLstd450Pow:
      return "pow";
    case GLSLstd450FSign:
      return "sign";
    case GLSLstd450Reflect:
      return "reflect";
    case GLSLstd450Round:
      return "round";
    case GLSLstd450Sin:
      return "sin";
    case GLSLstd450Sinh:
      return "sinh";
    case GLSLstd450SmoothStep:
      return "smoothStep";
    case GLSLstd450Sqrt:
      return "sqrt";
    case GLSLstd450Step:
      return "step";
    case GLSLstd450Tan:
      return "tan";
    case GLSLstd450Tanh:
      return "tanh";
    case GLSLstd450Trunc:
      return "trunc";

    default:
    // TODO(dneto) - The following are not implemented.
    // They are grouped semantically, as in GLSL.std.450.h.
    case GLSLstd450RoundEven:
    case GLSLstd450SSign:

    case GLSLstd450Radians:
    case GLSLstd450Degrees:
    case GLSLstd450Asinh:
    case GLSLstd450Acosh:
    case GLSLstd450Atanh:

    case GLSLstd450Determinant:
    case GLSLstd450MatrixInverse:

    case GLSLstd450Modf:
    case GLSLstd450ModfStruct:
    case GLSLstd450IMix:

    case GLSLstd450Frexp:
    case GLSLstd450FrexpStruct:

    case GLSLstd450PackSnorm4x8:
    case GLSLstd450PackUnorm4x8:
    case GLSLstd450PackSnorm2x16:
    case GLSLstd450PackUnorm2x16:
    case GLSLstd450PackHalf2x16:
    case GLSLstd450PackDouble2x32:
    case GLSLstd450UnpackSnorm2x16:
    case GLSLstd450UnpackUnorm2x16:
    case GLSLstd450UnpackHalf2x16:
    case GLSLstd450UnpackSnorm4x8:
    case GLSLstd450UnpackUnorm4x8:
    case GLSLstd450UnpackDouble2x32:

    case GLSLstd450Refract:

    case GLSLstd450FindILsb:
    case GLSLstd450FindSMsb:
    case GLSLstd450FindUMsb:

    case GLSLstd450InterpolateAtCentroid:
    case GLSLstd450InterpolateAtSample:
    case GLSLstd450InterpolateAtOffset:
      break;
  }
  return "";
}

// Returns the WGSL standard library function instrinsic for the
// given instruction, or ast::Intrinsic::kNone
ast::Intrinsic GetIntrinsic(SpvOp opcode) {
  switch (opcode) {
    case SpvOpBitCount:
      return ast::Intrinsic::kCountOneBits;
    case SpvOpBitReverse:
      return ast::Intrinsic::kReverseBits;
    case SpvOpDot:
      return ast::Intrinsic::kDot;
    case SpvOpOuterProduct:
      return ast::Intrinsic::kOuterProduct;
    default:
      break;
  }
  return ast::Intrinsic::kNone;
}

// @param opcode a SPIR-V opcode
// @returns true if the given instruction is an image access instruction
// whose first input operand is an OpSampledImage value.
bool IsSampledImageAccess(SpvOp opcode) {
  switch (opcode) {
    case SpvOpImageSampleImplicitLod:
    case SpvOpImageSampleExplicitLod:
    case SpvOpImageSampleDrefImplicitLod:
    case SpvOpImageSampleDrefExplicitLod:
    case SpvOpImageGather:
    case SpvOpImageDrefGather:
    case SpvOpImageQueryLod:
      return true;
    default:
      // WGSL doesn't have *Proj* texturing.
      break;
  }
  return false;
}

// @param opcode a SPIR-V opcode
// @returns true if the given instruction is an image sampling operation.
bool IsImageSampling(SpvOp opcode) {
  switch (opcode) {
    case SpvOpImageSampleImplicitLod:
    case SpvOpImageSampleExplicitLod:
    case SpvOpImageSampleDrefImplicitLod:
    case SpvOpImageSampleDrefExplicitLod:
      return true;
    default:
      // WGSL doesn't have *Proj* texturing.
      break;
  }
  return false;
}

// @param opcode a SPIR-V opcode
// @returns true if the given instruction is an image access instruction
// whose first input operand is an OpImage value.
bool IsRawImageAccess(SpvOp opcode) {
  switch (opcode) {
    case SpvOpImageRead:
    case SpvOpImageWrite:
    case SpvOpImageFetch:
    case SpvOpImageQuerySizeLod:
    case SpvOpImageQueryLevels:
    case SpvOpImageQuerySamples:
      return true;
    default:
      break;
  }
  return false;
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

/// A StatementBuilder for ast::SwitchStatement
/// @see StatementBuilder
struct SwitchStatementBuilder
    : public Castable<SwitchStatementBuilder, StatementBuilder> {
  /// Constructor
  /// @param cond the switch statement condition
  explicit SwitchStatementBuilder(ast::Expression* cond) : condition(cond) {}

  /// @param mod the ast Module to build into
  /// @returns the built ast::SwitchStatement
  ast::SwitchStatement* Build(ast::Module* mod) const override {
    // We've listed cases in reverse order in the switch statement.
    // Reorder them to match the presentation order in WGSL.
    auto reversed_cases = cases;
    std::reverse(reversed_cases.begin(), reversed_cases.end());

    return mod->create<ast::SwitchStatement>(Source{}, condition,
                                             reversed_cases);
  }

  /// Switch statement condition
  ast::Expression* const condition;
  /// Switch statement cases
  ast::CaseStatementList cases;
};

/// A StatementBuilder for ast::IfStatement
/// @see StatementBuilder
struct IfStatementBuilder
    : public Castable<IfStatementBuilder, StatementBuilder> {
  /// Constructor
  /// @param c the if-statement condition
  explicit IfStatementBuilder(ast::Expression* c) : cond(c) {}

  /// @param mod the ast Module to build into
  /// @returns the built ast::IfStatement
  ast::IfStatement* Build(ast::Module* mod) const override {
    return mod->create<ast::IfStatement>(Source{}, cond, body, else_stmts);
  }

  /// If-statement condition
  ast::Expression* const cond;
  /// If-statement block body
  ast::BlockStatement* body = nullptr;
  /// Optional if-statement else statements
  ast::ElseStatementList else_stmts;
};

/// A StatementBuilder for ast::LoopStatement
/// @see StatementBuilder
struct LoopStatementBuilder
    : public Castable<LoopStatementBuilder, StatementBuilder> {
  /// @param mod the ast Module to build into
  /// @returns the built ast::LoopStatement
  ast::LoopStatement* Build(ast::Module* mod) const override {
    return mod->create<ast::LoopStatement>(Source{}, body, continuing);
  }

  /// Loop-statement block body
  ast::BlockStatement* body = nullptr;
  /// Loop-statement continuing body
  ast::BlockStatement* continuing = nullptr;
};

}  // namespace

BlockInfo::BlockInfo(const spvtools::opt::BasicBlock& bb)
    : basic_block(&bb), id(bb.id()) {}

BlockInfo::~BlockInfo() = default;

DefInfo::DefInfo(const spvtools::opt::Instruction& def_inst,
                 uint32_t the_block_pos,
                 size_t the_index)
    : inst(def_inst), block_pos(the_block_pos), index(the_index) {}

DefInfo::~DefInfo() = default;

bool StatementBuilder::IsValid() const {
  return true;
}
ast::Node* StatementBuilder::Clone(ast::CloneContext*) const {
  return nullptr;
}
void StatementBuilder::to_str(std::ostream& out, size_t indent) const {
  make_indent(out, indent);
  out << "StatementBuilder" << std::endl;
}

FunctionEmitter::FunctionEmitter(ParserImpl* pi,
                                 const spvtools::opt::Function& function,
                                 const EntryPointInfo* ep_info)
    : parser_impl_(*pi),
      ast_module_(pi->get_module()),
      ir_context_(*(pi->ir_context())),
      def_use_mgr_(ir_context_.get_def_use_mgr()),
      constant_mgr_(ir_context_.get_constant_mgr()),
      type_mgr_(ir_context_.get_type_mgr()),
      fail_stream_(pi->fail_stream()),
      namer_(pi->namer()),
      function_(function),
      i32_(ast_module_.create<ast::type::I32>()),
      ep_info_(ep_info) {
  PushNewStatementBlock(nullptr, 0, nullptr);
}

FunctionEmitter::FunctionEmitter(ParserImpl* pi,
                                 const spvtools::opt::Function& function)
    : FunctionEmitter(pi, function, nullptr) {}

FunctionEmitter::~FunctionEmitter() = default;

FunctionEmitter::StatementBlock::StatementBlock(
    const Construct* construct,
    uint32_t end_id,
    FunctionEmitter::CompletionAction completion_action)
    : construct_(construct),
      end_id_(end_id),
      completion_action_(completion_action) {}

FunctionEmitter::StatementBlock::StatementBlock(StatementBlock&& other) =
    default;

FunctionEmitter::StatementBlock::~StatementBlock() = default;

void FunctionEmitter::StatementBlock::Finalize(ast::Module* mod) {
  assert(!finalized_ /* Finalize() must only be called once */);

  for (size_t i = 0; i < statements_.size(); i++) {
    if (auto* builder = statements_[i]->As<StatementBuilder>()) {
      statements_[i] = builder->Build(mod);
    }
  }

  if (completion_action_ != nullptr) {
    completion_action_(statements_);
  }

  finalized_ = true;
}

void FunctionEmitter::StatementBlock::Add(ast::Statement* statement) {
  assert(!finalized_ /* Add() must not be called after Finalize() */);
  statements_.emplace_back(statement);
}

void FunctionEmitter::PushNewStatementBlock(const Construct* construct,
                                            uint32_t end_id,
                                            CompletionAction action) {
  statements_stack_.emplace_back(StatementBlock{construct, end_id, action});
}

void FunctionEmitter::PushGuard(const std::string& guard_name,
                                uint32_t end_id) {
  assert(!statements_stack_.empty());
  assert(!guard_name.empty());
  // Guard control flow by the guard variable.  Introduce a new
  // if-selection with a then-clause ending at the same block
  // as the statement block at the top of the stack.
  const auto& top = statements_stack_.back();

  auto* cond = create<ast::IdentifierExpression>(
      Source{}, ast_module_.RegisterSymbol(guard_name), guard_name);
  auto* builder = AddStatementBuilder<IfStatementBuilder>(cond);

  PushNewStatementBlock(
      top.GetConstruct(), end_id, [=](const ast::StatementList& stmts) {
        builder->body = create<ast::BlockStatement>(Source{}, stmts);
      });
}

void FunctionEmitter::PushTrueGuard(uint32_t end_id) {
  assert(!statements_stack_.empty());
  const auto& top = statements_stack_.back();

  auto* cond = MakeTrue(Source{});
  auto* builder = AddStatementBuilder<IfStatementBuilder>(cond);

  PushNewStatementBlock(
      top.GetConstruct(), end_id, [=](const ast::StatementList& stmts) {
        builder->body = create<ast::BlockStatement>(Source{}, stmts);
      });
}

const ast::StatementList FunctionEmitter::ast_body() {
  assert(!statements_stack_.empty());
  auto& entry = statements_stack_[0];
  entry.Finalize(&ast_module_);
  return entry.GetStatements();
}

ast::Statement* FunctionEmitter::AddStatement(ast::Statement* statement) {
  assert(!statements_stack_.empty());
  if (statement != nullptr) {
    statements_stack_.back().Add(statement);
  }
  return statement;
}

ast::Statement* FunctionEmitter::LastStatement() {
  assert(!statements_stack_.empty());
  auto& statement_list = statements_stack_.back().GetStatements();
  assert(!statement_list.empty());
  return statement_list.back();
}

bool FunctionEmitter::Emit() {
  if (failed()) {
    return false;
  }
  // We only care about functions with bodies.
  if (function_.cbegin() == function_.cend()) {
    return true;
  }

  FunctionDeclaration decl;
  if (!ParseFunctionDeclaration(&decl)) {
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

  statements_stack_[0].Finalize(&ast_module_);

  auto& statements = statements_stack_[0].GetStatements();
  auto* body = create<ast::BlockStatement>(Source{}, statements);
  ast_module_.AddFunction(
      create<ast::Function>(decl.source, ast_module_.RegisterSymbol(decl.name),
                            decl.name, std::move(decl.params), decl.return_type,
                            body, std::move(decl.decorations)));

  // Maintain the invariant by repopulating the one and only element.
  statements_stack_.clear();
  PushNewStatementBlock(constructs_[0].get(), 0, nullptr);

  return success();
}

bool FunctionEmitter::ParseFunctionDeclaration(FunctionDeclaration* decl) {
  if (failed()) {
    return false;
  }

  std::string name;
  if (ep_info_ == nullptr) {
    name = namer_.Name(function_.result_id());
  } else {
    name = ep_info_->name;
  }

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
          auto* ast_param = parser_impl_.MakeVariable(
              param->result_id(), ast::StorageClass::kNone, ast_type, true,
              nullptr, ast::VariableDecorationList{});
          // Parameters are treated as const declarations.
          ast_params.emplace_back(ast_param);
          // The value is accessible by name.
          identifier_values_.insert(param->result_id());
        } else {
          // We've already logged an error and emitted a diagnostic. Do nothing
          // here.
        }
      });
  if (failed()) {
    return false;
  }
  ast::FunctionDecorationList decos;
  if (ep_info_ != nullptr) {
    decos.emplace_back(create<ast::StageDecoration>(Source{}, ep_info_->stage));
  }

  decl->name = name;
  decl->params = std::move(ast_params);
  decl->return_type = ret_ty;
  decl->decorations = std::move(decos);

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

  if (!TerminatorsAreValid()) {
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

  if (!RegisterIgnoredBuiltInVariables()) {
    return false;
  }
  if (!RegisterLocallyDefinedValues()) {
    return false;
  }
  FindValuesNeedingNamedOrHoistedDefinition();

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

bool FunctionEmitter::TerminatorsAreValid() {
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
    const auto ct = block_info->continue_for_header;
    block_info->is_continue_entire_loop = ct == block_id;
    if (is_single_block_loop && !block_info->is_continue_entire_loop) {
      return Fail() << "Block " << block_id
                    << " branches to itself but is not its own continue target";
    }
    // It's valid for a the header of a multi-block loop header to declare
    // itself as its own continue target.
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
  //         Pos(H) <= Pos(CT(H))
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
    // Pos(H) <= Pos(CT(H))
    if (ct_pos < header_pos) {
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
  //           (A block can be both a header and a continue target.)
  //        c. When you reach a block with an edge branching backward (in the
  //           structured order) to block T:
  //            T should be a loop header, and the top of the stack should be a
  //            continue target associated with T.
  //            This is the end of the continue construct. Pop the continue
  //            target off the stack.
  //
  //       Note: A loop header can declare itself as its own continue target.
  //
  //       Note: For a single-block loop, that block is a header, its own
  //       continue target, and its own backedge block.
  //
  //       Note: We pop the merge off first because a merge block that marks
  //       the end of one construct can be a single-block loop.  So that block
  //       is a merge, a header, a continue target, and a backedge block.
  //       But we want to finish processing of the merge before dealing with
  //       the loop.
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
    auto scope_end_pos = end_pos;
    // A loop construct is added right after its associated continue construct.
    // In that case, adjust the parent up.
    if (k == Construct::kLoop) {
      assert(parent);
      assert(parent->kind == Construct::kContinue);
      scope_end_pos = parent->end_pos;
      parent = parent->parent;
    }
    constructs_.push_back(std::make_unique<Construct>(
        parent, static_cast<int>(depth), k, begin_id, end_id, begin_pos,
        end_pos, scope_end_pos));
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
        // A loop header that is its own continue target will have an
        // empty loop construct. Only create a loop construct when
        // the continue target is *not* the same as the loop header.
        if (header != ct) {
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

const Construct* FunctionEmitter::SiblingLoopConstruct(
    const Construct* c) const {
  if (c == nullptr || c->kind != Construct::kContinue) {
    return nullptr;
  }
  const uint32_t continue_target_id = c->begin_id;
  const auto* continue_target = GetBlockInfo(continue_target_id);
  const uint32_t header_id = continue_target->header_for_continue;
  if (continue_target_id == header_id) {
    // The continue target is the whole loop.
    return nullptr;
  }
  const auto* candidate = GetBlockInfo(header_id)->construct;
  // Walk up the construct tree until we hit the loop.  In future
  // we might handle the corner case where the same block is both a
  // loop header and a selection header. For example, where the
  // loop header block has a conditional branch going to distinct
  // targets inside the loop body.
  while (candidate && candidate->kind != Construct::kLoop) {
    candidate = candidate->parent;
  }
  return candidate;
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

    // Track destinations for normal forward edges, either kForward
    // or kCaseFallThroughkIfBreak. These count toward the need
    // to have a merge instruction.  We also track kIfBreak edges
    // because when used with normal forward edges, we'll need
    // to generate a flow guard variable.
    std::vector<uint32_t> normal_forward_edges;
    std::vector<uint32_t> if_break_edges;

    if (successors.empty() && src_construct.enclosing_continue) {
      // Kill and return are not allowed in a continue construct.
      return Fail() << "Invalid function exit at block " << src
                    << " from continue construct starting at "
                    << src_construct.enclosing_continue->begin_id;
    }

    for (const auto dest : successors) {
      const auto* dest_info = GetBlockInfo(dest);
      // We've already checked terminators are valid.
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
            (edge_kind == EdgeKind::kCaseFallThrough)) {
          normal_forward_edges.push_back(dest);
        }
        if (edge_kind == EdgeKind::kIfBreak) {
          if_break_edges.push_back(dest);
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
    if ((normal_forward_edges.size() + if_break_edges.size() > 1) &&
        (src_info->merge_for_header == 0)) {
      // There is a branch to the merge of an if-selection combined
      // with an other normal forward branch.  Control within the
      // if-selection needs to be gated by a flow predicate.
      for (auto if_break_dest : if_break_edges) {
        auto* head_info =
            GetBlockInfo(GetBlockInfo(if_break_dest)->header_for_merge);
        // Generate a guard name, but only once.
        if (head_info->flow_guard_name.empty()) {
          const std::string guard = "guard" + std::to_string(head_info->id);
          head_info->flow_guard_name = namer_.MakeDerivedName(guard);
        }
      }
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
                    << construct->begin_id
                    << " and also the merge block for header block "
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
                    << construct->begin_id
                    << " and also the merge block for header block "
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
                       << "Block " << premerge_id << " is the merge block for "
                       << dest_block_info->header_for_merge
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
    ast::Expression* constructor = nullptr;
    if (inst.NumInOperands() > 1) {
      // SPIR-V initializers are always constants.
      // (OpenCL also allows the ID of an OpVariable, but we don't handle that
      // here.)
      constructor =
          parser_impl_.MakeConstantExpression(inst.GetSingleWordInOperand(1))
              .expr;
    }
    auto* var = parser_impl_.MakeVariable(
        inst.result_id(), ast::StorageClass::kFunction, var_store_type, false,
        constructor, ast::VariableDecorationList{});
    auto* var_decl_stmt = create<ast::VariableDeclStatement>(Source{}, var);
    AddStatement(var_decl_stmt);
    // Save this as an already-named value.
    identifier_values_.insert(inst.result_id());
  }
  return success();
}

TypedExpression FunctionEmitter::MakeExpression(uint32_t id) {
  if (failed()) {
    return {};
  }
  switch (GetSkipReason(id)) {
    case SkipReason::kDontSkip:
      break;
    case SkipReason::kOpaqueObject:
      Fail() << "internal error: unhandled use of opaque object with ID: "
             << id;
      return {};
    case SkipReason::kPointSizeBuiltinValue: {
      auto* f32 = create<ast::type::F32>();
      return {f32,
              create<ast::ScalarConstructorExpression>(
                  Source{}, create<ast::FloatLiteral>(Source{}, f32, 1.0f))};
    }
    case SkipReason::kPointSizeBuiltinPointer:
      Fail() << "unhandled use of a pointer to the PointSize builtin, with ID: "
             << id;
      return {};
  }
  if (identifier_values_.count(id) || parser_impl_.IsScalarSpecConstant(id)) {
    auto name = namer_.Name(id);
    return TypedExpression{
        parser_impl_.ConvertType(def_use_mgr_->GetDef(id)->type_id()),
        create<ast::IdentifierExpression>(
            Source{}, ast_module_.RegisterSymbol(name), name)};
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
    case SpvOpVariable: {
      // This occurs for module-scope variables.
      auto name = namer_.Name(inst->result_id());
      return TypedExpression{
          parser_impl_.ConvertType(inst->type_id()),
          create<ast::IdentifierExpression>(
              Source{}, ast_module_.RegisterSymbol(name), name)};
    }
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
  statements_stack_[0].SetConstruct(function_construct);

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
         (statements_stack_.back().GetEndId() == block_info.id)) {
    statements_stack_.back().Finalize(&ast_module_);
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
    auto* const top_construct = statements_stack_.back().GetConstruct();
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
  //   each of those is headed by a block with a merge instruction (OpLoopMerge
  //   for kLoop, and OpSelectionMerge for the others), and the kIfSelection and
  //   kSwitchSelection header blocks end in different branch instructions.
  // - A kContinue can contain a kContinue
  //   This is possible in Vulkan SPIR-V, but Tint disallows this by the rule
  //   that a block can be continue target for at most one header block. See
  //   test DISABLED_BlockIsContinueForMoreThanOneHeader. If we generalize this,
  //   then by a dominance argument, the inner loop continue target can only be
  //   a single-block loop.
  // TODO(dneto): Handle this case.
  // - All that's left is a kContinue and one of kIfSelection, kSwitchSelection,
  //   kLoop.
  //
  //   The kContinue can be the parent of the other.  For example, a selection
  //   starting at the first block of a continue construct.
  //
  //   The kContinue can't be the child of the other because either:
  //     - The other can't be kLoop because:
  //        - If the kLoop is for a different loop then the kContinue, then
  //          the kContinue must be its own loop header, and so the same
  //          block is two different loops. That's a contradiction.
  //        - If the kLoop is for a the same loop, then this is a contradiction
  //          because a kContinue and its kLoop have disjoint block sets.
  //     - The other construct can't be a selection because:
  //       - The kContinue construct is the entire loop, i.e. the continue
  //         target is its own loop header block.  But then the continue target
  //         has an OpLoopMerge instruction, which contradicts this block being
  //         a selection header.
  //       - The kContinue is in a multi-block loop that is has a non-empty
  //         kLoop; and the selection contains the kContinue block but not the
  //         loop block. That breaks dominance rules. That is, the continue
  //         target is dominated by that loop header, and so gets found by the
  //         block traversal on the outside before the selection is found. The
  //         selection is inside the outer loop.
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
        if (block_info.is_continue_entire_loop) {
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
        if (!EmitStatementsInBasicBlock(block_info, &emitted)) {
          return false;
        }
        if (!EmitSwitchStart(block_info)) {
          return false;
        }
        has_normal_terminator = false;
        break;
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

  const uint32_t true_head = block_info.true_head;
  const uint32_t false_head = block_info.false_head;
  const uint32_t premerge_head = block_info.premerge_head;

  const std::string guard_name = block_info.flow_guard_name;
  if (!guard_name.empty()) {
    // Declare the guard variable just before the "if", initialized to true.
    auto* guard_var =
        create<ast::Variable>(Source{},                                // source
                              ast_module_.RegisterSymbol(guard_name),  // symbol
                              guard_name,                              // name
                              ast::StorageClass::kFunction,    // storage_class
                              parser_impl_.Bool(),             // type
                              false,                           // is_const
                              MakeTrue(Source{}),              // constructor
                              ast::VariableDecorationList{});  // decorations
    auto* guard_decl = create<ast::VariableDeclStatement>(Source{}, guard_var);
    AddStatement(guard_decl);
  }

  const auto condition_id =
      block_info.basic_block->terminator()->GetSingleWordInOperand(0);
  auto* cond = MakeExpression(condition_id).expr;

  // Generate the code for the condition.
  auto* builder = AddStatementBuilder<IfStatementBuilder>(cond);

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
  auto push_else = [this, builder, else_end, construct]() {
    // Push the else clause onto the stack first.
    PushNewStatementBlock(
        construct, else_end, [=](const ast::StatementList& stmts) {
          // Only set the else-clause if there are statements to fill it.
          if (!stmts.empty()) {
            // The "else" consists of the statement list from the top of
            // statements stack, without an elseif condition.
            auto* else_body = create<ast::BlockStatement>(Source{}, stmts);
            builder->else_stmts.emplace_back(
                create<ast::ElseStatement>(Source{}, nullptr, else_body));
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

    // Handle the premerge, if it exists.
    if (premerge_head) {
      // The top of the stack is the statement block that is the parent of the
      // if-statement. Adding statements now will place them after that 'if'.
      if (guard_name.empty()) {
        // We won't have a flow guard for the premerge.
        // Insert a trivial if(true) { ... } around the blocks from the
        // premerge head until the end of the if-selection.  This is needed
        // to ensure uniform reconvergence occurs at the end of the if-selection
        // just like in the original SPIR-V.
        PushTrueGuard(construct->end_id);
      } else {
        // Add a flow guard around the blocks in the premrege area.
        PushGuard(guard_name, construct->end_id);
      }
    }

    push_else();
    if (true_head && false_head && !guard_name.empty()) {
      // There are non-trivial then and else clauses.
      // We have to guard the start of the else.
      PushGuard(guard_name, else_end);
    }

    // Push the then clause onto the stack.
    PushNewStatementBlock(
        construct, then_end, [=](const ast::StatementList& stmts) {
          builder->body = create<ast::BlockStatement>(Source{}, stmts);
        });
  }

  return success();
}

bool FunctionEmitter::EmitSwitchStart(const BlockInfo& block_info) {
  // The block is the if-header block.  So its construct is the if construct.
  auto* construct = block_info.construct;
  assert(construct->kind == Construct::kSwitchSelection);
  assert(construct->begin_id == block_info.id);
  const auto* branch = block_info.basic_block->terminator();

  const auto selector_id = branch->GetSingleWordInOperand(0);
  // Generate the code for the selector.
  auto selector = MakeExpression(selector_id);

  // First, push the statement block for the entire switch.
  auto* swch = AddStatementBuilder<SwitchStatementBuilder>(selector.expr);

  // Grab a pointer to the case list.  It will get buried in the statement block
  // stack.
  PushNewStatementBlock(construct, construct->end_id, nullptr);

  // We will push statement-blocks onto the stack to gather the statements in
  // the default clause and cases clauses. Determine the list of blocks
  // that start each clause.
  std::vector<const BlockInfo*> clause_heads;

  // Collect the case clauses, even if they are just the merge block.
  // First the default clause.
  const auto default_id = branch->GetSingleWordInOperand(1);
  const auto* default_info = GetBlockInfo(default_id);
  clause_heads.push_back(default_info);
  // Now the case clauses.
  for (uint32_t iarg = 2; iarg + 1 < branch->NumInOperands(); iarg += 2) {
    const auto case_target_id = branch->GetSingleWordInOperand(iarg + 1);
    clause_heads.push_back(GetBlockInfo(case_target_id));
  }

  std::stable_sort(clause_heads.begin(), clause_heads.end(),
                   [](const BlockInfo* lhs, const BlockInfo* rhs) {
                     return lhs->pos < rhs->pos;
                   });
  // Remove duplicates
  {
    // Use read index r, and write index w.
    // Invariant: w <= r;
    size_t w = 0;
    for (size_t r = 0; r < clause_heads.size(); ++r) {
      if (clause_heads[r] != clause_heads[w]) {
        ++w;  // Advance the write cursor.
      }
      clause_heads[w] = clause_heads[r];
    }
    // We know it's not empty because it always has at least a default clause.
    assert(!clause_heads.empty());
    clause_heads.resize(w + 1);
  }

  // Push them on in reverse order.
  const auto last_clause_index = clause_heads.size() - 1;
  for (size_t i = last_clause_index;; --i) {
    // Create a list of integer literals for the selector values leading to
    // this case clause.
    ast::CaseSelectorList selectors;
    const auto* values_ptr = clause_heads[i]->case_values.get();
    const bool has_selectors = (values_ptr && !values_ptr->empty());
    if (has_selectors) {
      std::vector<uint64_t> values(values_ptr->begin(), values_ptr->end());
      std::stable_sort(values.begin(), values.end());
      for (auto value : values) {
        // The rest of this module can handle up to 64 bit switch values.
        // The Tint AST handles 32-bit values.
        const uint32_t value32 = uint32_t(value & 0xFFFFFFFF);
        if (selector.type->is_unsigned_scalar_or_vector()) {
          selectors.emplace_back(
              create<ast::UintLiteral>(Source{}, selector.type, value32));
        } else {
          selectors.emplace_back(
              create<ast::SintLiteral>(Source{}, selector.type, value32));
        }
      }
    }

    // Where does this clause end?
    const auto end_id = (i + 1 < clause_heads.size()) ? clause_heads[i + 1]->id
                                                      : construct->end_id;

    // Reserve the case clause slot in swch->cases, push the new statement block
    // for the case, and fill the case clause once the block is generated.
    auto case_idx = swch->cases.size();
    swch->cases.emplace_back(nullptr);
    PushNewStatementBlock(
        construct, end_id, [=](const ast::StatementList& stmts) {
          auto* body = create<ast::BlockStatement>(Source{}, stmts);
          swch->cases[case_idx] =
              create<ast::CaseStatement>(Source{}, selectors, body);
        });

    if ((default_info == clause_heads[i]) && has_selectors &&
        construct->ContainsPos(default_info->pos)) {
      // Generate a default clause with a just fallthrough.
      auto* stmts = create<ast::BlockStatement>(
          Source{}, ast::StatementList{
                        create<ast::FallthroughStatement>(Source{}),
                    });
      auto* case_stmt =
          create<ast::CaseStatement>(Source{}, ast::CaseSelectorList{}, stmts);
      swch->cases.emplace_back(case_stmt);
    }

    if (i == 0) {
      break;
    }
  }

  return success();
}

bool FunctionEmitter::EmitLoopStart(const Construct* construct) {
  auto* builder = AddStatementBuilder<LoopStatementBuilder>();
  PushNewStatementBlock(
      construct, construct->end_id, [=](const ast::StatementList& stmts) {
        builder->body = create<ast::BlockStatement>(Source{}, stmts);
      });
  return success();
}

bool FunctionEmitter::EmitContinuingStart(const Construct* construct) {
  // A continue construct has the same depth as its associated loop
  // construct. Start a continue construct.
  auto* loop_candidate = LastStatement();
  auto* loop = loop_candidate->As<LoopStatementBuilder>();
  if (loop == nullptr) {
    return Fail() << "internal error: starting continue construct, "
                     "expected loop on top of stack";
  }
  PushNewStatementBlock(
      construct, construct->end_id, [=](const ast::StatementList& stmts) {
        loop->continuing = create<ast::BlockStatement>(Source{}, stmts);
      });

  return success();
}

bool FunctionEmitter::EmitNormalTerminator(const BlockInfo& block_info) {
  const auto& terminator = *(block_info.basic_block->terminator());
  switch (terminator.opcode()) {
    case SpvOpReturn:
      AddStatement(create<ast::ReturnStatement>(Source{}));
      return true;
    case SpvOpReturnValue: {
      auto value = MakeExpression(terminator.GetSingleWordInOperand(0));
      AddStatement(create<ast::ReturnStatement>(Source{}, value.expr));
    }
      return true;
    case SpvOpKill:
      // For now, assume SPIR-V OpKill has same semantics as WGSL discard.
      // TODO(dneto): https://github.com/gpuweb/gpuweb/issues/676
      AddStatement(create<ast::DiscardStatement>(Source{}));
      return true;
    case SpvOpUnreachable:
      // Translate as if it's a return. This avoids the problem where WGSL
      // requires a return statement at the end of the function body.
      {
        const auto* result_type = type_mgr_->GetType(function_.type_id());
        if (result_type->AsVoid() != nullptr) {
          AddStatement(create<ast::ReturnStatement>(Source{}));
        } else {
          auto* ast_type = parser_impl_.ConvertType(function_.type_id());
          AddStatement(create<ast::ReturnStatement>(
              Source{}, parser_impl_.MakeNullValue(ast_type)));
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
      auto* cond = MakeExpression(terminator.GetSingleWordInOperand(0)).expr;

      // We have two distinct destinations. But we only get here if this
      // is a normal terminator; in particular the source block is *not* the
      // start of an if-selection or a switch-selection.  So at most one branch
      // is a kForward, kCaseFallThrough, or kIfBreak.

      // The fallthrough case is special because WGSL requires the fallthrough
      // statement to be last in the case clause.
      if (true_kind == EdgeKind::kCaseFallThrough) {
        return EmitConditionalCaseFallThrough(block_info, cond, false_kind,
                                              *false_info, true);
      } else if (false_kind == EdgeKind::kCaseFallThrough) {
        return EmitConditionalCaseFallThrough(block_info, cond, true_kind,
                                              *true_info, false);
      }

      // At this point, at most one edge is kForward or kIfBreak.

      // Emit an 'if' statement to express the *other* branch as a conditional
      // break or continue.  Either or both of these could be nullptr.
      // (A nullptr is generated for kIfBreak, kForward, or kBack.)
      // Also if one of the branches is an if-break out of an if-selection
      // requiring a flow guard, then get that flow guard name too.  It will
      // come from at most one of these two branches.
      std::string flow_guard;
      auto* true_branch =
          MakeBranchDetailed(block_info, *true_info, false, &flow_guard);
      auto* false_branch =
          MakeBranchDetailed(block_info, *false_info, false, &flow_guard);

      AddStatement(MakeSimpleIf(cond, true_branch, false_branch));
      if (!flow_guard.empty()) {
        PushGuard(flow_guard, statements_stack_.back().GetEndId());
      }
      return true;
    }
    case SpvOpSwitch:
      // TODO(dneto)
      break;
    default:
      break;
  }
  return success();
}

ast::Statement* FunctionEmitter::MakeBranchDetailed(
    const BlockInfo& src_info,
    const BlockInfo& dest_info,
    bool forced,
    std::string* flow_guard_name_ptr) const {
  auto kind = src_info.succ_edge.find(dest_info.id)->second;
  switch (kind) {
    case EdgeKind::kBack:
      // Nothing to do. The loop backedge is implicit.
      break;
    case EdgeKind::kSwitchBreak: {
      if (forced) {
        return create<ast::BreakStatement>(Source{});
      }
      // Unless forced, don't bother with a break at the end of a case/default
      // clause.
      const auto header = dest_info.header_for_merge;
      assert(header != 0);
      const auto* exiting_construct = GetBlockInfo(header)->construct;
      assert(exiting_construct->kind == Construct::kSwitchSelection);
      const auto candidate_next_case_pos = src_info.pos + 1;
      // Leaving the last block from the last case?
      if (candidate_next_case_pos == dest_info.pos) {
        // No break needed.
        return nullptr;
      }
      // Leaving the last block from not-the-last-case?
      if (exiting_construct->ContainsPos(candidate_next_case_pos)) {
        const auto* candidate_next_case =
            GetBlockInfo(block_order_[candidate_next_case_pos]);
        if (candidate_next_case->case_head_for == exiting_construct ||
            candidate_next_case->default_head_for == exiting_construct) {
          // No break needed.
          return nullptr;
        }
      }
      // We need a break.
      return create<ast::BreakStatement>(Source{});
    }
    case EdgeKind::kLoopBreak:
      return create<ast::BreakStatement>(Source{});
    case EdgeKind::kLoopContinue:
      // An unconditional continue to the next block is redundant and ugly.
      // Skip it in that case.
      if (dest_info.pos == 1 + src_info.pos) {
        break;
      }
      // Otherwise, emit a regular continue statement.
      return create<ast::ContinueStatement>(Source{});
    case EdgeKind::kIfBreak: {
      const auto& flow_guard =
          GetBlockInfo(dest_info.header_for_merge)->flow_guard_name;
      if (!flow_guard.empty()) {
        if (flow_guard_name_ptr != nullptr) {
          *flow_guard_name_ptr = flow_guard;
        }
        // Signal an exit from the branch.
        return create<ast::AssignmentStatement>(
            Source{},
            create<ast::IdentifierExpression>(
                Source{}, ast_module_.RegisterSymbol(flow_guard), flow_guard),
            MakeFalse(Source{}));
      }

      // For an unconditional branch, the break out to an if-selection
      // merge block is implicit.
      break;
    }
    case EdgeKind::kCaseFallThrough:
      return create<ast::FallthroughStatement>(Source{});
    case EdgeKind::kForward:
      // Unconditional forward branch is implicit.
      break;
  }
  return nullptr;
}

ast::Statement* FunctionEmitter::MakeSimpleIf(ast::Expression* condition,
                                              ast::Statement* then_stmt,
                                              ast::Statement* else_stmt) const {
  if ((then_stmt == nullptr) && (else_stmt == nullptr)) {
    return nullptr;
  }
  ast::ElseStatementList else_stmts;
  if (else_stmt != nullptr) {
    ast::StatementList stmts{else_stmt};
    else_stmts.emplace_back(create<ast::ElseStatement>(
        Source{}, nullptr, create<ast::BlockStatement>(Source{}, stmts)));
  }
  ast::StatementList if_stmts;
  if (then_stmt != nullptr) {
    if_stmts.emplace_back(then_stmt);
  }
  auto* if_block = create<ast::BlockStatement>(Source{}, if_stmts);
  auto* if_stmt =
      create<ast::IfStatement>(Source{}, condition, if_block, else_stmts);

  return if_stmt;
}

bool FunctionEmitter::EmitConditionalCaseFallThrough(
    const BlockInfo& src_info,
    ast::Expression* cond,
    EdgeKind other_edge_kind,
    const BlockInfo& other_dest,
    bool fall_through_is_true_branch) {
  // In WGSL, the fallthrough statement must come last in the case clause.
  // So we'll emit an if statement for the other branch, and then emit
  // the fallthrough.

  // We have two distinct destinations. But we only get here if this
  // is a normal terminator; in particular the source block is *not* the
  // start of an if-selection.  So at most one branch is a kForward or
  // kCaseFallThrough.
  if (other_edge_kind == EdgeKind::kForward) {
    return Fail()
           << "internal error: normal terminator OpBranchConditional has "
              "both forward and fallthrough edges";
  }
  if (other_edge_kind == EdgeKind::kIfBreak) {
    return Fail()
           << "internal error: normal terminator OpBranchConditional has "
              "both IfBreak and fallthrough edges.  Violates nesting rule";
  }
  if (other_edge_kind == EdgeKind::kBack) {
    return Fail()
           << "internal error: normal terminator OpBranchConditional has "
              "both backedge and fallthrough edges.  Violates nesting rule";
  }
  auto* other_branch = MakeForcedBranch(src_info, other_dest);
  if (other_branch == nullptr) {
    return Fail() << "internal error: expected a branch for edge-kind "
                  << int(other_edge_kind);
  }
  if (fall_through_is_true_branch) {
    AddStatement(MakeSimpleIf(cond, nullptr, other_branch));
  } else {
    AddStatement(MakeSimpleIf(cond, other_branch, nullptr));
  }
  AddStatement(create<ast::FallthroughStatement>(Source{}));

  return success();
}

bool FunctionEmitter::EmitStatementsInBasicBlock(const BlockInfo& block_info,
                                                 bool* already_emitted) {
  if (*already_emitted) {
    // Only emit this part of the basic block once.
    return true;
  }
  // Returns the given list of local definition IDs, sorted by their index.
  auto sorted_by_index = [this](const std::vector<uint32_t>& ids) {
    auto sorted = ids;
    std::stable_sort(sorted.begin(), sorted.end(),
                     [this](const uint32_t lhs, const uint32_t rhs) {
                       return GetDefInfo(lhs)->index < GetDefInfo(rhs)->index;
                     });
    return sorted;
  };

  // Emit declarations of hoisted variables, in index order.
  for (auto id : sorted_by_index(block_info.hoisted_ids)) {
    const auto* def_inst = def_use_mgr_->GetDef(id);
    assert(def_inst);
    auto* ast_type =
        RemapStorageClass(parser_impl_.ConvertType(def_inst->type_id()), id);
    AddStatement(create<ast::VariableDeclStatement>(
        Source{}, parser_impl_.MakeVariable(id, ast::StorageClass::kFunction,
                                            ast_type, false, nullptr,
                                            ast::VariableDecorationList{})));
    // Save this as an already-named value.
    identifier_values_.insert(id);
  }
  // Emit declarations of phi state variables, in index order.
  for (auto id : sorted_by_index(block_info.phis_needing_state_vars)) {
    const auto* def_inst = def_use_mgr_->GetDef(id);
    assert(def_inst);
    const auto phi_var_name = GetDefInfo(id)->phi_var;
    assert(!phi_var_name.empty());
    auto* var = create<ast::Variable>(
        Source{},                                       // source
        ast_module_.RegisterSymbol(phi_var_name),       // symbol
        phi_var_name,                                   // name
        ast::StorageClass::kFunction,                   // storage_class
        parser_impl_.ConvertType(def_inst->type_id()),  // type
        false,                                          // is_const
        nullptr,                                        // constructor
        ast::VariableDecorationList{});                 // decorations
    AddStatement(create<ast::VariableDeclStatement>(Source{}, var));
  }

  // Emit regular statements.
  const spvtools::opt::BasicBlock& bb = *(block_info.basic_block);
  const auto* terminator = bb.terminator();
  const auto* merge = bb.GetMergeInst();  // Might be nullptr
  for (auto& inst : bb) {
    if (&inst == terminator || &inst == merge || inst.opcode() == SpvOpLabel ||
        inst.opcode() == SpvOpVariable) {
      continue;
    }
    if (!EmitStatement(inst)) {
      return false;
    }
  }

  // Emit assignments to carry values to phi nodes in potential destinations.
  // Do it in index order.
  if (!block_info.phi_assignments.empty()) {
    auto sorted = block_info.phi_assignments;
    std::stable_sort(sorted.begin(), sorted.end(),
                     [this](const BlockInfo::PhiAssignment& lhs,
                            const BlockInfo::PhiAssignment& rhs) {
                       return GetDefInfo(lhs.phi_id)->index <
                              GetDefInfo(rhs.phi_id)->index;
                     });
    for (auto assignment : block_info.phi_assignments) {
      const auto var_name = GetDefInfo(assignment.phi_id)->phi_var;
      auto expr = MakeExpression(assignment.value);
      AddStatement(create<ast::AssignmentStatement>(
          Source{},
          create<ast::IdentifierExpression>(
              Source{}, ast_module_.RegisterSymbol(var_name), var_name),
          expr.expr));
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
  auto* ast_const = parser_impl_.MakeVariable(
      inst.result_id(), ast::StorageClass::kNone, ast_expr.type, true,
      ast_expr.expr, ast::VariableDecorationList{});
  if (!ast_const) {
    return false;
  }
  AddStatement(create<ast::VariableDeclStatement>(Source{}, ast_const));
  // Save this as an already-named value.
  identifier_values_.insert(inst.result_id());
  return success();
}

bool FunctionEmitter::EmitConstDefOrWriteToHoistedVar(
    const spvtools::opt::Instruction& inst,
    TypedExpression ast_expr) {
  const auto result_id = inst.result_id();
  const auto* def_info = GetDefInfo(result_id);
  if (def_info && def_info->requires_hoisted_def) {
    auto name = namer_.Name(result_id);
    // Emit an assignment of the expression to the hoisted variable.
    AddStatement(create<ast::AssignmentStatement>(
        Source{},
        create<ast::IdentifierExpression>(
            Source{}, ast_module_.RegisterSymbol(name), namer_.Name(result_id)),
        ast_expr.expr));
    return true;
  }
  return EmitConstDefinition(inst, ast_expr);
}

bool FunctionEmitter::EmitStatement(const spvtools::opt::Instruction& inst) {
  if (failed()) {
    return false;
  }
  const auto result_id = inst.result_id();
  const auto type_id = inst.type_id();

  if (type_id != 0) {
    const auto& builtin_position_info = parser_impl_.GetBuiltInPositionInfo();
    if (type_id == builtin_position_info.struct_type_id) {
      return Fail() << "operations producing a per-vertex structure are not "
                       "supported: "
                    << inst.PrettyPrint();
    }
    if (type_id == builtin_position_info.pointer_type_id) {
      return Fail() << "operations producing a pointer to a per-vertex "
                       "structure are not "
                       "supported: "
                    << inst.PrettyPrint();
    }
  }

  // Handle combinatorial instructions.
  const auto* def_info = GetDefInfo(result_id);
  if (def_info) {
    TypedExpression combinatorial_expr;
    if (def_info->skip == SkipReason::kDontSkip) {
      combinatorial_expr = MaybeEmitCombinatorialValue(inst);
    }
    // An access chain or OpCopyObject can generate a skip.
    if (def_info->skip != SkipReason::kDontSkip) {
      return true;
    }

    if (combinatorial_expr.expr != nullptr) {
      if (def_info->requires_hoisted_def ||
          def_info->requires_named_const_def || def_info->num_uses != 1) {
        // Generate a const definition or an assignment to a hoisted definition
        // now and later use the const or variable name at the uses of this
        // value.
        return EmitConstDefOrWriteToHoistedVar(inst, combinatorial_expr);
      }
      // It is harmless to defer emitting the expression until it's used.
      // Any supporting statements have already been emitted.
      singly_used_values_.insert(std::make_pair(result_id, combinatorial_expr));
      return success();
    }
  }
  if (failed()) {
    return false;
  }

  if (IsSampledImageAccess(inst.opcode()) || IsRawImageAccess(inst.opcode())) {
    return EmitImageAccess(inst);
  }

  switch (inst.opcode()) {
    case SpvOpNop:
      return true;

    case SpvOpStore: {
      const auto ptr_id = inst.GetSingleWordInOperand(0);
      const auto value_id = inst.GetSingleWordInOperand(1);

      // Handle exceptional cases
      if (GetSkipReason(ptr_id) == SkipReason::kPointSizeBuiltinPointer) {
        if (const auto* c = constant_mgr_->FindDeclaredConstant(value_id)) {
          // If we're writing a constant 1.0, then skip the write.  That's all
          // that WebGPU handles.
          auto* ct = c->type();
          if (ct->AsFloat() && (ct->AsFloat()->width() == 32) &&
              (c->GetFloat() == 1.0f)) {
            // Don't store to PointSize
            return true;
          }
        }
        return Fail() << "cannot store a value other than constant 1.0 to "
                         "PointSize builtin: "
                      << inst.PrettyPrint();
      }
      const auto ptr_type_id = def_use_mgr_->GetDef(ptr_id)->type_id();
      const auto& builtin_position_info = parser_impl_.GetBuiltInPositionInfo();
      if (ptr_type_id == builtin_position_info.pointer_type_id) {
        return Fail()
               << "storing to the whole per-vertex structure is not supported: "
               << inst.PrettyPrint();
      }

      // Handle an ordinary store as an assignment.
      // TODO(dneto): Order of evaluation?
      auto lhs = MakeExpression(ptr_id);
      auto rhs = MakeExpression(value_id);
      AddStatement(
          create<ast::AssignmentStatement>(Source{}, lhs.expr, rhs.expr));
      return success();
    }

    case SpvOpLoad: {
      // Memory accesses must be issued in SPIR-V program order.
      // So represent a load by a new const definition.
      const auto ptr_id = inst.GetSingleWordInOperand(0);
      if (GetSkipReason(ptr_id) == SkipReason::kPointSizeBuiltinPointer) {
        GetDefInfo(inst.result_id())->skip = SkipReason::kPointSizeBuiltinValue;
        return true;
      }
      auto expr = MakeExpression(ptr_id);
      // The load result type is the pointee type of its operand.
      assert(expr.type->Is<ast::type::Pointer>());
      expr.type = expr.type->As<ast::type::Pointer>()->type();
      return EmitConstDefOrWriteToHoistedVar(inst, expr);
    }

    case SpvOpCopyObject: {
      // Arguably, OpCopyObject is purely combinatorial. On the other hand,
      // it exists to make a new name for something. So we choose to make
      // a new named constant definition.
      auto value_id = inst.GetSingleWordInOperand(0);
      const auto skip = GetSkipReason(value_id);
      if (skip != SkipReason::kDontSkip) {
        GetDefInfo(inst.result_id())->skip = skip;
        return true;
      }
      auto expr = MakeExpression(value_id);
      expr.type = RemapStorageClass(expr.type, result_id);
      return EmitConstDefOrWriteToHoistedVar(inst, expr);
    }

    case SpvOpPhi: {
      // Emit a read from the associated state variable.
      TypedExpression expr{
          parser_impl_.ConvertType(inst.type_id()),
          create<ast::IdentifierExpression>(
              Source{}, ast_module_.RegisterSymbol(def_info->phi_var),
              def_info->phi_var)};
      return EmitConstDefOrWriteToHoistedVar(inst, expr);
    }

    case SpvOpFunctionCall:
      return EmitFunctionCall(inst);

    default:
      break;
  }
  return Fail() << "unhandled instruction with opcode " << inst.opcode() << ": "
                << inst.PrettyPrint();
}

TypedExpression FunctionEmitter::MakeOperand(
    const spvtools::opt::Instruction& inst,
    uint32_t operand_index) {
  auto expr = this->MakeExpression(inst.GetSingleWordInOperand(operand_index));
  return parser_impl_.RectifyOperandSignedness(inst, std::move(expr));
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
    auto* binary_expr = create<ast::BinaryExpression>(Source{}, binary_op,
                                                      arg0.expr, arg1.expr);
    TypedExpression result{ast_type, binary_expr};
    return parser_impl_.RectifyForcedResultType(result, inst, arg0.type);
  }

  auto unary_op = ast::UnaryOp::kNegation;
  if (GetUnaryOp(opcode, &unary_op)) {
    auto arg0 = MakeOperand(inst, 0);
    auto* unary_expr =
        create<ast::UnaryOpExpression>(Source{}, unary_op, arg0.expr);
    TypedExpression result{ast_type, unary_expr};
    return parser_impl_.RectifyForcedResultType(result, inst, arg0.type);
  }

  const char* unary_builtin_name = GetUnaryBuiltInFunctionName(opcode);
  if (unary_builtin_name != nullptr) {
    ast::ExpressionList params;
    params.emplace_back(MakeOperand(inst, 0).expr);
    return {ast_type,
            create<ast::CallExpression>(
                Source{},
                create<ast::IdentifierExpression>(
                    Source{}, ast_module_.RegisterSymbol(unary_builtin_name),
                    unary_builtin_name),
                std::move(params))};
  }

  const auto intrinsic = GetIntrinsic(opcode);
  if (intrinsic != ast::Intrinsic::kNone) {
    return MakeIntrinsicCall(inst);
  }

  if (opcode == SpvOpAccessChain || opcode == SpvOpInBoundsAccessChain) {
    return MakeAccessChain(inst);
  }

  if (opcode == SpvOpBitcast) {
    return {ast_type, create<ast::BitcastExpression>(
                          Source{}, ast_type, MakeOperand(inst, 0).expr)};
  }

  auto negated_op = NegatedFloatCompare(opcode);
  if (negated_op != ast::BinaryOp::kNone) {
    auto arg0 = MakeOperand(inst, 0);
    auto arg1 = MakeOperand(inst, 1);
    auto* binary_expr = create<ast::BinaryExpression>(Source{}, negated_op,
                                                      arg0.expr, arg1.expr);
    auto* negated_expr = create<ast::UnaryOpExpression>(
        Source{}, ast::UnaryOp::kNot, binary_expr);
    return {ast_type, negated_expr};
  }

  if (opcode == SpvOpExtInst) {
    const auto import = inst.GetSingleWordInOperand(0);
    if (parser_impl_.glsl_std_450_imports().count(import) == 0) {
      Fail() << "unhandled extended instruction import with ID " << import;
      return {};
    }
    return EmitGlslStd450ExtInst(inst);
  }

  if (opcode == SpvOpCompositeConstruct) {
    ast::ExpressionList operands;
    for (uint32_t iarg = 0; iarg < inst.NumInOperands(); ++iarg) {
      operands.emplace_back(MakeOperand(inst, iarg).expr);
    }
    return {ast_type, create<ast::TypeConstructorExpression>(
                          Source{}, ast_type, std::move(operands))};
  }

  if (opcode == SpvOpCompositeExtract) {
    return MakeCompositeExtract(inst);
  }

  if (opcode == SpvOpVectorShuffle) {
    return MakeVectorShuffle(inst);
  }

  if (opcode == SpvOpConvertSToF || opcode == SpvOpConvertUToF ||
      opcode == SpvOpConvertFToS || opcode == SpvOpConvertFToU) {
    return MakeNumericConversion(inst);
  }

  if (opcode == SpvOpUndef) {
    // Replace undef with the null value.
    return {ast_type, parser_impl_.MakeNullValue(ast_type)};
  }

  if (opcode == SpvOpSelect) {
    return MakeSimpleSelect(inst);
  }

  if (opcode == SpvOpArrayLength) {
    return MakeArrayLength(inst);
  }

  // builtin readonly function
  // glsl.std.450 readonly function

  // Instructions:
  //    OpSatConvertSToU // Only in Kernel (OpenCL), not in WebGPU
  //    OpSatConvertUToS // Only in Kernel (OpenCL), not in WebGPU
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
  //    OpCompositeInsert

  return {};
}

TypedExpression FunctionEmitter::EmitGlslStd450ExtInst(
    const spvtools::opt::Instruction& inst) {
  const auto ext_opcode = inst.GetSingleWordInOperand(1);
  const auto name = GetGlslStd450FuncName(ext_opcode);
  if (name.empty()) {
    Fail() << "unhandled GLSL.std.450 instruction " << ext_opcode;
    return {};
  }

  auto* func = create<ast::IdentifierExpression>(
      Source{}, ast_module_.RegisterSymbol(name), name);
  ast::ExpressionList operands;
  ast::type::Type* first_operand_type = nullptr;
  // All parameters to GLSL.std.450 extended instructions are IDs.
  for (uint32_t iarg = 2; iarg < inst.NumInOperands(); ++iarg) {
    TypedExpression operand = MakeOperand(inst, iarg);
    if (first_operand_type == nullptr) {
      first_operand_type = operand.type;
    }
    operands.emplace_back(operand.expr);
  }
  auto* ast_type = parser_impl_.ConvertType(inst.type_id());
  auto* call = create<ast::CallExpression>(Source{}, func, std::move(operands));
  TypedExpression call_expr{ast_type, call};
  return parser_impl_.RectifyForcedResultType(call_expr, inst,
                                              first_operand_type);
}

ast::IdentifierExpression* FunctionEmitter::Swizzle(uint32_t i) {
  if (i >= kMaxVectorLen) {
    Fail() << "vector component index is larger than " << kMaxVectorLen - 1
           << ": " << i;
    return nullptr;
  }
  const char* names[] = {"x", "y", "z", "w"};
  return create<ast::IdentifierExpression>(
      Source{}, ast_module_.RegisterSymbol(names[i & 3]), names[i & 3]);
}

ast::IdentifierExpression* FunctionEmitter::PrefixSwizzle(uint32_t n) {
  switch (n) {
    case 1:
      return create<ast::IdentifierExpression>(
          Source{}, ast_module_.RegisterSymbol("x"), "x");
    case 2:
      return create<ast::IdentifierExpression>(
          Source{}, ast_module_.RegisterSymbol("xy"), "xy");
    case 3:
      return create<ast::IdentifierExpression>(
          Source{}, ast_module_.RegisterSymbol("xyz"), "xyz");
    default:
      break;
  }
  Fail() << "invalid swizzle prefix count: " << n;
  return nullptr;
}

TypedExpression FunctionEmitter::MakeAccessChain(
    const spvtools::opt::Instruction& inst) {
  if (inst.NumInOperands() < 1) {
    // Binary parsing will fail on this anyway.
    Fail() << "invalid access chain: has no input operands";
    return {};
  }

  const auto base_id = inst.GetSingleWordInOperand(0);
  const auto base_skip = GetSkipReason(base_id);
  if (base_skip != SkipReason::kDontSkip) {
    // This can occur for AccessChain with no indices.
    GetDefInfo(inst.result_id())->skip = base_skip;
    return {};
  }

  // A SPIR-V access chain is a single instruction with multiple indices
  // walking down into composites.  The Tint AST represents this as
  // ever-deeper nested indexing expressions. Start off with an expression
  // for the base, and then bury that inside nested indexing expressions.
  TypedExpression current_expr(MakeOperand(inst, 0));
  const auto constants = constant_mgr_->GetOperandConstants(&inst);

  auto ptr_ty_id = def_use_mgr_->GetDef(base_id)->type_id();
  uint32_t first_index = 1;
  const auto num_in_operands = inst.NumInOperands();

  // If the variable was originally gl_PerVertex, then in the AST we
  // have instead emitted a gl_Position variable.
  // If computing the pointer to the Position builtin, then emit the
  // pointer to the generated gl_Position variable.
  // If computing the pointer to the PointSize builtin, then mark the
  // result as skippable due to being the point-size pointer.
  // If computing the pointer to the ClipDistance or CullDistance builtins,
  // then error out.
  {
    const auto& builtin_position_info = parser_impl_.GetBuiltInPositionInfo();
    if (base_id == builtin_position_info.per_vertex_var_id) {
      // We only support the Position member.
      const auto* member_index_inst =
          def_use_mgr_->GetDef(inst.GetSingleWordInOperand(first_index));
      if (member_index_inst == nullptr) {
        Fail()
            << "first index of access chain does not reference an instruction: "
            << inst.PrettyPrint();
        return {};
      }
      const auto* member_index_const =
          constant_mgr_->GetConstantFromInst(member_index_inst);
      if (member_index_const == nullptr) {
        Fail() << "first index of access chain into per-vertex structure is "
                  "not a constant: "
               << inst.PrettyPrint();
        return {};
      }
      const auto* member_index_const_int = member_index_const->AsIntConstant();
      if (member_index_const_int == nullptr) {
        Fail() << "first index of access chain into per-vertex structure is "
                  "not a constant integer: "
               << inst.PrettyPrint();
        return {};
      }
      const auto member_index_value =
          member_index_const_int->GetZeroExtendedValue();
      if (member_index_value != builtin_position_info.position_member_index) {
        if (member_index_value ==
            builtin_position_info.pointsize_member_index) {
          if (auto* def_info = GetDefInfo(inst.result_id())) {
            def_info->skip = SkipReason::kPointSizeBuiltinPointer;
            return {};
          }
        } else {
          // TODO(dneto): Handle ClipDistance and CullDistance
          Fail() << "accessing per-vertex member " << member_index_value
                 << " is not supported. Only Position is supported, and "
                    "PointSize is ignored";
          return {};
        }
      }

      // Skip past the member index that gets us to Position.
      first_index = first_index + 1;
      // Replace the gl_PerVertex reference with the gl_Position reference
      ptr_ty_id = builtin_position_info.position_member_pointer_type_id;

      auto name = namer_.Name(base_id);
      current_expr.expr = create<ast::IdentifierExpression>(
          Source{}, ast_module_.RegisterSymbol(name), name);
      current_expr.type = parser_impl_.ConvertType(ptr_ty_id);
    }
  }

  const auto* ptr_type_inst = def_use_mgr_->GetDef(ptr_ty_id);
  if (!ptr_type_inst || (ptr_type_inst->opcode() != SpvOpTypePointer)) {
    Fail() << "Access chain %" << inst.result_id()
           << " base pointer is not of pointer type";
    return {};
  }
  SpvStorageClass storage_class =
      static_cast<SpvStorageClass>(ptr_type_inst->GetSingleWordInOperand(0));
  uint32_t pointee_type_id = ptr_type_inst->GetSingleWordInOperand(1);

  // Build up a nested expression for the access chain by walking down the type
  // hierarchy, maintaining |pointee_type_id| as the SPIR-V ID of the type of
  // the object pointed to after processing the previous indices.
  for (uint32_t index = first_index; index < num_in_operands; ++index) {
    const auto* index_const =
        constants[index] ? constants[index]->AsIntConstant() : nullptr;
    const int64_t index_const_val =
        index_const ? index_const->GetSignExtendedValue() : 0;
    ast::Expression* next_expr = nullptr;

    const auto* pointee_type_inst = def_use_mgr_->GetDef(pointee_type_id);
    if (!pointee_type_inst) {
      Fail() << "pointee type %" << pointee_type_id
             << " is invalid after following " << (index - first_index)
             << " indices: " << inst.PrettyPrint();
      return {};
    }
    switch (pointee_type_inst->opcode()) {
      case SpvOpTypeVector:
        if (index_const) {
          // Try generating a MemberAccessor expression
          const auto num_elems = pointee_type_inst->GetSingleWordInOperand(1);
          if (index_const_val < 0 || num_elems <= index_const_val) {
            Fail() << "Access chain %" << inst.result_id() << " index %"
                   << inst.GetSingleWordInOperand(index) << " value "
                   << index_const_val << " is out of bounds for vector of "
                   << num_elems << " elements";
            return {};
          }
          if (uint64_t(index_const_val) >= kMaxVectorLen) {
            Fail() << "internal error: swizzle index " << index_const_val
                   << " is too big. Max handled index is " << kMaxVectorLen - 1;
          }
          next_expr = create<ast::MemberAccessorExpression>(
              Source{}, current_expr.expr, Swizzle(uint32_t(index_const_val)));
        } else {
          // Non-constant index. Use array syntax
          next_expr = create<ast::ArrayAccessorExpression>(
              Source{}, current_expr.expr, MakeOperand(inst, index).expr);
        }
        // All vector components are the same type.
        pointee_type_id = pointee_type_inst->GetSingleWordInOperand(0);
        break;
      case SpvOpTypeMatrix:
        // Use array syntax.
        next_expr = create<ast::ArrayAccessorExpression>(
            Source{}, current_expr.expr, MakeOperand(inst, index).expr);
        // All matrix components are the same type.
        pointee_type_id = pointee_type_inst->GetSingleWordInOperand(0);
        break;
      case SpvOpTypeArray:
        next_expr = create<ast::ArrayAccessorExpression>(
            Source{}, current_expr.expr, MakeOperand(inst, index).expr);
        pointee_type_id = pointee_type_inst->GetSingleWordInOperand(0);
        break;
      case SpvOpTypeRuntimeArray:
        next_expr = create<ast::ArrayAccessorExpression>(
            Source{}, current_expr.expr, MakeOperand(inst, index).expr);
        pointee_type_id = pointee_type_inst->GetSingleWordInOperand(0);
        break;
      case SpvOpTypeStruct: {
        if (!index_const) {
          Fail() << "Access chain %" << inst.result_id() << " index %"
                 << inst.GetSingleWordInOperand(index)
                 << " is a non-constant index into a structure %"
                 << pointee_type_id;
          return {};
        }
        const auto num_members = pointee_type_inst->NumInOperands();
        if ((index_const_val < 0) || num_members <= uint64_t(index_const_val)) {
          Fail() << "Access chain %" << inst.result_id() << " index value "
                 << index_const_val << " is out of bounds for structure %"
                 << pointee_type_id << " having " << num_members << " members";
          return {};
        }
        auto name =
            namer_.GetMemberName(pointee_type_id, uint32_t(index_const_val));
        auto* member_access = create<ast::IdentifierExpression>(
            Source{}, ast_module_.RegisterSymbol(name), name);

        next_expr = create<ast::MemberAccessorExpression>(
            Source{}, current_expr.expr, member_access);
        pointee_type_id = pointee_type_inst->GetSingleWordInOperand(
            static_cast<uint32_t>(index_const_val));
        break;
      }
      default:
        Fail() << "Access chain with unknown or invalid pointee type %"
               << pointee_type_id << ": " << pointee_type_inst->PrettyPrint();
        return {};
    }
    const auto pointer_type_id =
        type_mgr_->FindPointerToType(pointee_type_id, storage_class);
    auto* ast_pointer_type = parser_impl_.ConvertType(pointer_type_id);
    assert(ast_pointer_type);
    assert(ast_pointer_type->Is<ast::type::Pointer>());
    current_expr = TypedExpression{ast_pointer_type, next_expr};
  }
  return current_expr;
}

TypedExpression FunctionEmitter::MakeCompositeExtract(
    const spvtools::opt::Instruction& inst) {
  // This is structurally similar to creating an access chain, but
  // the SPIR-V instruction has literal indices instead of IDs for indices.

  // A SPIR-V composite extract is a single instruction with multiple
  // literal indices walking down into composites. The Tint AST represents
  // this as ever-deeper nested indexing expressions. Start off with an
  // expression for the composite, and then bury that inside nested indexing
  // expressions.
  auto source = GetSourceForInst(inst);
  TypedExpression current_expr(MakeOperand(inst, 0));

  auto make_index = [this, source](uint32_t literal) {
    auto* type = create<ast::type::U32>();
    return create<ast::ScalarConstructorExpression>(
        source, create<ast::UintLiteral>(source, type, literal));
  };

  const auto composite = inst.GetSingleWordInOperand(0);
  auto current_type_id = def_use_mgr_->GetDef(composite)->type_id();
  // Build up a nested expression for the access chain by walking down the type
  // hierarchy, maintaining |current_type_id| as the SPIR-V ID of the type of
  // the object pointed to after processing the previous indices.
  const auto num_in_operands = inst.NumInOperands();
  for (uint32_t index = 1; index < num_in_operands; ++index) {
    const uint32_t index_val = inst.GetSingleWordInOperand(index);

    const auto* current_type_inst = def_use_mgr_->GetDef(current_type_id);
    if (!current_type_inst) {
      Fail() << "composite type %" << current_type_id
             << " is invalid after following " << (index - 1)
             << " indices: " << inst.PrettyPrint();
      return {};
    }
    ast::Expression* next_expr = nullptr;
    switch (current_type_inst->opcode()) {
      case SpvOpTypeVector: {
        // Try generating a MemberAccessor expression. That result in something
        // like  "foo.z", which is more idiomatic than "foo[2]".
        const auto num_elems = current_type_inst->GetSingleWordInOperand(1);
        if (num_elems <= index_val) {
          Fail() << "CompositeExtract %" << inst.result_id() << " index value "
                 << index_val << " is out of bounds for vector of " << num_elems
                 << " elements";
          return {};
        }
        if (index_val >= kMaxVectorLen) {
          Fail() << "internal error: swizzle index " << index_val
                 << " is too big. Max handled index is " << kMaxVectorLen - 1;
        }
        next_expr = create<ast::MemberAccessorExpression>(
            Source{}, current_expr.expr, Swizzle(index_val));
        // All vector components are the same type.
        current_type_id = current_type_inst->GetSingleWordInOperand(0);
        break;
      }
      case SpvOpTypeMatrix: {
        // Check bounds
        const auto num_elems = current_type_inst->GetSingleWordInOperand(1);
        if (num_elems <= index_val) {
          Fail() << "CompositeExtract %" << inst.result_id() << " index value "
                 << index_val << " is out of bounds for matrix of " << num_elems
                 << " elements";
          return {};
        }
        if (index_val >= kMaxVectorLen) {
          Fail() << "internal error: swizzle index " << index_val
                 << " is too big. Max handled index is " << kMaxVectorLen - 1;
        }
        // Use array syntax.
        next_expr = create<ast::ArrayAccessorExpression>(
            Source{}, current_expr.expr, make_index(index_val));
        // All matrix components are the same type.
        current_type_id = current_type_inst->GetSingleWordInOperand(0);
        break;
      }
      case SpvOpTypeArray:
        // The array size could be a spec constant, and so it's not always
        // statically checkable.  Instead, rely on a runtime index clamp
        // or runtime check to keep this safe.
        next_expr = create<ast::ArrayAccessorExpression>(
            Source{}, current_expr.expr, make_index(index_val));
        current_type_id = current_type_inst->GetSingleWordInOperand(0);
        break;
      case SpvOpTypeRuntimeArray:
        Fail() << "can't do OpCompositeExtract on a runtime array";
        return {};
      case SpvOpTypeStruct: {
        const auto num_members = current_type_inst->NumInOperands();
        if (num_members <= index_val) {
          Fail() << "CompositeExtract %" << inst.result_id() << " index value "
                 << index_val << " is out of bounds for structure %"
                 << current_type_id << " having " << num_members << " members";
          return {};
        }
        auto name = namer_.GetMemberName(current_type_id, uint32_t(index_val));
        auto* member_access = create<ast::IdentifierExpression>(
            Source{}, ast_module_.RegisterSymbol(name), name);

        next_expr = create<ast::MemberAccessorExpression>(
            Source{}, current_expr.expr, member_access);
        current_type_id = current_type_inst->GetSingleWordInOperand(index_val);
        break;
      }
      default:
        Fail() << "CompositeExtract with bad type %" << current_type_id << ": "
               << current_type_inst->PrettyPrint();
        return {};
    }
    current_expr =
        TypedExpression{parser_impl_.ConvertType(current_type_id), next_expr};
  }
  return current_expr;
}

ast::Expression* FunctionEmitter::MakeTrue(const Source& source) const {
  return create<ast::ScalarConstructorExpression>(
      source, create<ast::BoolLiteral>(source, parser_impl_.Bool(), true));
}

ast::Expression* FunctionEmitter::MakeFalse(const Source& source) const {
  ast::type::Bool bool_type;
  return create<ast::ScalarConstructorExpression>(
      source, create<ast::BoolLiteral>(source, parser_impl_.Bool(), false));
}

TypedExpression FunctionEmitter::MakeVectorShuffle(
    const spvtools::opt::Instruction& inst) {
  const auto vec0_id = inst.GetSingleWordInOperand(0);
  const auto vec1_id = inst.GetSingleWordInOperand(1);
  const spvtools::opt::Instruction& vec0 = *(def_use_mgr_->GetDef(vec0_id));
  const spvtools::opt::Instruction& vec1 = *(def_use_mgr_->GetDef(vec1_id));
  const auto vec0_len =
      type_mgr_->GetType(vec0.type_id())->AsVector()->element_count();
  const auto vec1_len =
      type_mgr_->GetType(vec1.type_id())->AsVector()->element_count();

  // Idiomatic vector accessors.

  // Generate an ast::TypeConstructor expression.
  // Assume the literal indices are valid, and there is a valid number of them.
  auto source = GetSourceForInst(inst);
  ast::type::Vector* result_type =
      parser_impl_.ConvertType(inst.type_id())->As<ast::type::Vector>();
  ast::ExpressionList values;
  for (uint32_t i = 2; i < inst.NumInOperands(); ++i) {
    const auto index = inst.GetSingleWordInOperand(i);
    if (index < vec0_len) {
      values.emplace_back(create<ast::MemberAccessorExpression>(
          source, MakeExpression(vec0_id).expr, Swizzle(index)));
    } else if (index < vec0_len + vec1_len) {
      const auto sub_index = index - vec0_len;
      assert(sub_index < kMaxVectorLen);
      values.emplace_back(create<ast::MemberAccessorExpression>(
          source, MakeExpression(vec1_id).expr, Swizzle(sub_index)));
    } else if (index == 0xFFFFFFFF) {
      // By rule, this maps to OpUndef.  Instead, make it zero.
      values.emplace_back(parser_impl_.MakeNullValue(result_type->type()));
    } else {
      Fail() << "invalid vectorshuffle ID %" << inst.result_id()
             << ": index too large: " << index;
      return {};
    }
  }
  return {result_type,
          create<ast::TypeConstructorExpression>(source, result_type, values)};
}

bool FunctionEmitter::RegisterIgnoredBuiltInVariables() {
  size_t index = def_info_.size();
  for (auto& ignored_var : parser_impl_.ignored_builtins()) {
    const auto id = ignored_var.first;
    const auto builtin = ignored_var.second;
    def_info_[id] =
        std::make_unique<DefInfo>(*(def_use_mgr_->GetDef(id)), 0, index);
    ++index;
    auto& def = def_info_[id];
    switch (builtin) {
      case SpvBuiltInPointSize:
        def->skip = SkipReason::kPointSizeBuiltinPointer;
        break;
      default:
        return Fail() << "unrecognized ignored builtin: " << int(builtin);
    }
  }
  return true;
}

bool FunctionEmitter::RegisterLocallyDefinedValues() {
  // Create a DefInfo for each value definition in this function.
  size_t index = def_info_.size();
  for (auto block_id : block_order_) {
    const auto* block_info = GetBlockInfo(block_id);
    const auto block_pos = block_info->pos;
    for (const auto& inst : *(block_info->basic_block)) {
      const auto result_id = inst.result_id();
      if ((result_id == 0) || inst.opcode() == SpvOpLabel) {
        continue;
      }
      def_info_[result_id] = std::make_unique<DefInfo>(inst, block_pos, index);
      ++index;
      auto& info = def_info_[result_id];

      // Determine storage class for pointer values. Do this in order because
      // we might rely on the storage class for a previously-visited definition.
      // Logical pointers can't be transmitted through OpPhi, so remaining
      // pointer definitions are SSA values, and their definitions must be
      // visited before their uses.
      const auto* type = type_mgr_->GetType(inst.type_id());
      if (type) {
        if (type->AsPointer()) {
          if (const auto* ast_type = parser_impl_.ConvertType(inst.type_id())) {
            if (auto* ptr = ast_type->As<ast::type::Pointer>()) {
              info->storage_class = ptr->storage_class();
            }
          }
          switch (inst.opcode()) {
            case SpvOpUndef:
            case SpvOpVariable:
              // Keep the default decision based on the result type.
              break;
            case SpvOpAccessChain:
            case SpvOpCopyObject:
              // Inherit from the first operand. We need this so we can pick up
              // a remapped storage buffer.
              info->storage_class = GetStorageClassForPointerValue(
                  inst.GetSingleWordInOperand(0));
              break;
            default:
              return Fail()
                     << "pointer defined in function from unknown opcode: "
                     << inst.PrettyPrint();
          }
          if (info->storage_class == ast::StorageClass::kUniformConstant) {
            info->skip = SkipReason::kOpaqueObject;
          }
        }
        if (type->AsSampler() || type->AsImage() || type->AsSampledImage()) {
          // Defer code generation until the instruction that actually acts on
          // the image.
          info->skip = SkipReason::kOpaqueObject;
        }
      }
    }
  }
  return true;
}

ast::StorageClass FunctionEmitter::GetStorageClassForPointerValue(uint32_t id) {
  auto where = def_info_.find(id);
  if (where != def_info_.end()) {
    return where->second.get()->storage_class;
  }
  const auto type_id = def_use_mgr_->GetDef(id)->type_id();
  if (type_id) {
    auto* ast_type = parser_impl_.ConvertType(type_id);
    if (ast_type && ast_type->Is<ast::type::Pointer>()) {
      return ast_type->As<ast::type::Pointer>()->storage_class();
    }
  }
  return ast::StorageClass::kNone;
}

ast::type::Type* FunctionEmitter::RemapStorageClass(ast::type::Type* type,
                                                    uint32_t result_id) {
  if (const auto* ast_ptr_type = type->As<ast::type::Pointer>()) {
    // Remap an old-style storage buffer pointer to a new-style storage
    // buffer pointer.
    const auto sc = GetStorageClassForPointerValue(result_id);
    if (ast_ptr_type->storage_class() != sc) {
      return parser_impl_.get_module().create<ast::type::Pointer>(
          ast_ptr_type->type(), sc);
    }
  }
  return type;
}

void FunctionEmitter::FindValuesNeedingNamedOrHoistedDefinition() {
  // Mark vector operands of OpVectorShuffle as needing a named definition,
  // but only if they are defined in this function as well.
  for (auto& id_def_info_pair : def_info_) {
    const auto& inst = id_def_info_pair.second->inst;
    if (inst.opcode() == SpvOpVectorShuffle) {
      // We might access the vector operands multiple times. Make sure they
      // are evaluated only once.
      for (auto vector_arg : std::array<uint32_t, 2>{0, 1}) {
        auto id = inst.GetSingleWordInOperand(vector_arg);
        auto* operand_def = GetDefInfo(id);
        if (operand_def) {
          operand_def->requires_named_const_def = true;
        }
      }
    }
  }

  // Scan uses of locally defined IDs, in function block order.
  for (auto block_id : block_order_) {
    const auto* block_info = GetBlockInfo(block_id);
    const auto block_pos = block_info->pos;
    for (const auto& inst : *(block_info->basic_block)) {
      // Update bookkeeping for locally-defined IDs used by this instruction.
      inst.ForEachInId([this, block_pos, block_info](const uint32_t* id_ptr) {
        auto* def_info = GetDefInfo(*id_ptr);
        if (def_info) {
          // Update usage count.
          def_info->num_uses++;
          // Update usage span.
          def_info->last_use_pos = std::max(def_info->last_use_pos, block_pos);

          // Determine whether this ID is defined in a different construct
          // from this use.
          const auto defining_block = block_order_[def_info->block_pos];
          const auto* def_in_construct =
              GetBlockInfo(defining_block)->construct;
          if (def_in_construct != block_info->construct) {
            def_info->used_in_another_construct = true;
          }
        }
      });

      if (inst.opcode() == SpvOpPhi) {
        // Declare a name for the variable used to carry values to a phi.
        const auto phi_id = inst.result_id();
        auto* phi_def_info = GetDefInfo(phi_id);
        phi_def_info->phi_var =
            namer_.MakeDerivedName(namer_.Name(phi_id) + "_phi");
        // Track all the places where we need to mention the variable,
        // so we can place its declaration.  First, record the location of
        // the read from the variable.
        uint32_t first_pos = block_pos;
        uint32_t last_pos = block_pos;
        // Record the assignments that will propagate values from predecessor
        // blocks.
        for (uint32_t i = 0; i + 1 < inst.NumInOperands(); i += 2) {
          const uint32_t value_id = inst.GetSingleWordInOperand(i);
          const uint32_t pred_block_id = inst.GetSingleWordInOperand(i + 1);
          auto* pred_block_info = GetBlockInfo(pred_block_id);
          // The predecessor might not be in the block order at all, so we
          // need this guard.
          if (pred_block_info) {
            // Record the assignment that needs to occur at the end
            // of the predecessor block.
            pred_block_info->phi_assignments.push_back({phi_id, value_id});
            first_pos = std::min(first_pos, pred_block_info->pos);
            last_pos = std::min(last_pos, pred_block_info->pos);
          }
        }

        // Schedule the declaration of the state variable.
        const auto* enclosing_construct =
            GetEnclosingScope(first_pos, last_pos);
        GetBlockInfo(enclosing_construct->begin_id)
            ->phis_needing_state_vars.push_back(phi_id);
      }
    }
  }

  // For an ID defined in this function, determine if its evaluation and
  // potential declaration needs special handling:
  // - Compensate for the fact that dominance does not map directly to scope.
  //   A definition could dominate its use, but a named definition in WGSL
  //   at the location of the definition could go out of scope by the time
  //   you reach the use.  In that case, we hoist the definition to a basic
  //   block at the smallest scope enclosing both the definition and all
  //   its uses.
  // - If value is used in a different construct than its definition, then it
  //   needs a named constant definition.  Otherwise we might sink an
  //   expensive computation into control flow, and hence change performance.
  for (auto& id_def_info_pair : def_info_) {
    const auto def_id = id_def_info_pair.first;
    auto* def_info = id_def_info_pair.second.get();
    if (def_info->num_uses == 0) {
      // There is no need to adjust the location of the declaration.
      continue;
    }
    // The first use must be the at the SSA definition, because block order
    // respects dominance.
    const auto first_pos = def_info->block_pos;
    const auto last_use_pos = def_info->last_use_pos;

    const auto* def_in_construct =
        GetBlockInfo(block_order_[first_pos])->construct;
    // A definition in the first block of an kIfSelection or kSwitchSelection
    // occurs before the branch, and so that definition should count as
    // having been defined at the scope of the parent construct.
    if (first_pos == def_in_construct->begin_pos) {
      if ((def_in_construct->kind == Construct::kIfSelection) ||
          (def_in_construct->kind == Construct::kSwitchSelection)) {
        def_in_construct = def_in_construct->parent;
      }
    }

    bool should_hoist = false;
    if (!def_in_construct->ContainsPos(last_use_pos)) {
      // To satisfy scoping, we have to hoist the definition out to an enclosing
      // construct.
      should_hoist = true;
    } else {
      // Avoid moving combinatorial values across constructs.  This is a
      // simple heuristic to avoid changing the cost of an operation
      // by moving it into or out of a loop, for example.
      if ((def_info->storage_class == ast::StorageClass::kNone) &&
          def_info->used_in_another_construct) {
        should_hoist = true;
      }
    }

    if (should_hoist) {
      const auto* enclosing_construct =
          GetEnclosingScope(first_pos, last_use_pos);
      if (enclosing_construct == def_in_construct) {
        // We can use a plain 'const' definition.
        def_info->requires_named_const_def = true;
      } else {
        // We need to make a hoisted variable definition.
        // TODO(dneto): Handle non-storable types, particularly pointers.
        def_info->requires_hoisted_def = true;
        auto* hoist_to_block = GetBlockInfo(enclosing_construct->begin_id);
        hoist_to_block->hoisted_ids.push_back(def_id);
      }
    }
  }
}

const Construct* FunctionEmitter::GetEnclosingScope(uint32_t first_pos,
                                                    uint32_t last_pos) const {
  const auto* enclosing_construct =
      GetBlockInfo(block_order_[first_pos])->construct;
  assert(enclosing_construct != nullptr);
  // Constructs are strictly nesting, so follow parent pointers
  while (enclosing_construct &&
         !enclosing_construct->ScopeContainsPos(last_pos)) {
    // The scope of a continue construct is enclosed in its associated loop
    // construct, but they are siblings in our construct tree.
    const auto* sibling_loop = SiblingLoopConstruct(enclosing_construct);
    // Go to the sibling loop if it exists, otherwise walk up to the parent.
    enclosing_construct =
        sibling_loop ? sibling_loop : enclosing_construct->parent;
  }
  // At worst, we go all the way out to the function construct.
  assert(enclosing_construct != nullptr);
  return enclosing_construct;
}

TypedExpression FunctionEmitter::MakeNumericConversion(
    const spvtools::opt::Instruction& inst) {
  const auto opcode = inst.opcode();
  auto* requested_type = parser_impl_.ConvertType(inst.type_id());
  auto arg_expr = MakeOperand(inst, 0);
  if (!arg_expr.expr || !arg_expr.type) {
    return {};
  }

  ast::type::Type* expr_type = nullptr;
  if ((opcode == SpvOpConvertSToF) || (opcode == SpvOpConvertUToF)) {
    if (arg_expr.type->is_integer_scalar_or_vector()) {
      expr_type = requested_type;
    } else {
      Fail() << "operand for conversion to floating point must be integral "
                "scalar or vector, but got: "
             << arg_expr.type->type_name();
    }
  } else if (inst.opcode() == SpvOpConvertFToU) {
    if (arg_expr.type->is_float_scalar_or_vector()) {
      expr_type = parser_impl_.GetUnsignedIntMatchingShape(arg_expr.type);
    } else {
      Fail() << "operand for conversion to unsigned integer must be floating "
                "point scalar or vector, but got: "
             << arg_expr.type->type_name();
    }
  } else if (inst.opcode() == SpvOpConvertFToS) {
    if (arg_expr.type->is_float_scalar_or_vector()) {
      expr_type = parser_impl_.GetSignedIntMatchingShape(arg_expr.type);
    } else {
      Fail() << "operand for conversion to signed integer must be floating "
                "point scalar or vector, but got: "
             << arg_expr.type->type_name();
    }
  }
  if (expr_type == nullptr) {
    // The diagnostic has already been emitted.
    return {};
  }

  ast::ExpressionList params;
  params.push_back(arg_expr.expr);
  TypedExpression result{
      expr_type, create<ast::TypeConstructorExpression>(Source{}, expr_type,
                                                        std::move(params))};

  if (requested_type == expr_type) {
    return result;
  }
  return {requested_type, create<ast::BitcastExpression>(
                              Source{}, requested_type, result.expr)};
}

bool FunctionEmitter::EmitFunctionCall(const spvtools::opt::Instruction& inst) {
  // We ignore function attributes such as Inline, DontInline, Pure, Const.
  auto name = namer_.Name(inst.GetSingleWordInOperand(0));
  auto* function = create<ast::IdentifierExpression>(
      Source{}, ast_module_.RegisterSymbol(name), name);

  ast::ExpressionList params;
  for (uint32_t iarg = 1; iarg < inst.NumInOperands(); ++iarg) {
    params.emplace_back(MakeOperand(inst, iarg).expr);
  }
  auto* call_expr =
      create<ast::CallExpression>(Source{}, function, std::move(params));
  auto* result_type = parser_impl_.ConvertType(inst.type_id());
  if (!result_type) {
    return Fail() << "internal error: no mapped type result of call: "
                  << inst.PrettyPrint();
  }

  if (result_type->Is<ast::type::Void>()) {
    return nullptr !=
           AddStatement(create<ast::CallStatement>(Source{}, call_expr));
  }

  return EmitConstDefOrWriteToHoistedVar(inst, {result_type, call_expr});
}

TypedExpression FunctionEmitter::MakeIntrinsicCall(
    const spvtools::opt::Instruction& inst) {
  const auto intrinsic = GetIntrinsic(inst.opcode());
  std::ostringstream ss;
  ss << intrinsic;
  auto name = ss.str();
  auto* ident = create<ast::IdentifierExpression>(
      Source{}, ast_module_.RegisterSymbol(name), name);
  ident->set_intrinsic(intrinsic);

  ast::ExpressionList params;
  ast::type::Type* first_operand_type = nullptr;
  for (uint32_t iarg = 0; iarg < inst.NumInOperands(); ++iarg) {
    TypedExpression operand = MakeOperand(inst, iarg);
    if (first_operand_type == nullptr) {
      first_operand_type = operand.type;
    }
    params.emplace_back(operand.expr);
  }
  auto* call_expr =
      create<ast::CallExpression>(Source{}, ident, std::move(params));
  auto* result_type = parser_impl_.ConvertType(inst.type_id());
  if (!result_type) {
    Fail() << "internal error: no mapped type result of call: "
           << inst.PrettyPrint();
    return {};
  }
  TypedExpression call{result_type, call_expr};
  return parser_impl_.RectifyForcedResultType(call, inst, first_operand_type);
}

TypedExpression FunctionEmitter::MakeSimpleSelect(
    const spvtools::opt::Instruction& inst) {
  auto condition = MakeOperand(inst, 0);
  auto operand1 = MakeOperand(inst, 1);
  auto operand2 = MakeOperand(inst, 2);

  // SPIR-V validation requires:
  // - the condition to be bool or bool vector, so we don't check it here.
  // - operand1, operand2, and result type to match.
  // - you can't select over pointers or pointer vectors, unless you also have
  //   a VariablePointers* capability, which is not allowed in by WebGPU.
  auto* op_ty = operand1.type;
  if (op_ty->Is<ast::type::Vector>() || op_ty->is_float_scalar() ||
      op_ty->is_integer_scalar() || op_ty->Is<ast::type::Bool>()) {
    ast::ExpressionList params;
    params.push_back(operand1.expr);
    params.push_back(operand2.expr);
    // The condition goes last.
    params.push_back(condition.expr);
    return {operand1.type,
            create<ast::CallExpression>(
                Source{},
                create<ast::IdentifierExpression>(
                    Source{}, ast_module_.RegisterSymbol("select"), "select"),
                std::move(params))};
  }
  return {};
}

Source FunctionEmitter::GetSourceForInst(
    const spvtools::opt::Instruction& inst) const {
  return parser_impl_.GetSourceForInst(&inst);
}

bool FunctionEmitter::EmitImageAccess(const spvtools::opt::Instruction& inst) {
  uint32_t arg_index = 0;  // The SPIR-V input argument index
  ast::ExpressionList params;

  const auto image_or_sampled_image_operand_id =
      inst.GetSingleWordInOperand(arg_index);
  // Form the texture operand.
  const auto* image = parser_impl_.GetMemoryObjectDeclarationForHandle(
      image_or_sampled_image_operand_id, true);
  if (!image) {
    return Fail() << "internal error: couldn't find image for "
                  << inst.PrettyPrint();
  }
  auto name = namer_.Name(image->result_id());
  params.push_back(create<ast::IdentifierExpression>(
      Source{}, ast_module_.RegisterSymbol(name), name));

  const auto opcode = inst.opcode();
  if (IsSampledImageAccess(opcode)) {
    // Form the sampler operand.
    const auto* sampler = parser_impl_.GetMemoryObjectDeclarationForHandle(
        image_or_sampled_image_operand_id, false);
    if (!sampler) {
      return Fail() << "internal error: couldn't find sampler for "
                    << inst.PrettyPrint();
    }
    auto param_name = namer_.Name(sampler->result_id());
    params.push_back(create<ast::IdentifierExpression>(
        Source{}, ast_module_.RegisterSymbol(param_name), param_name));
  }

  ast::type::Pointer* texture_ptr_type =
      parser_impl_.GetTypeForHandleVar(*image);
  if (!texture_ptr_type) {
    return Fail();
  }
  ast::type::Texture* texture_type =
      texture_ptr_type->type()->As<ast::type::Texture>();
  if (!texture_type) {
    return Fail();
  }

  // We're done with the first SPIR-V operand.  Move on to the next.
  arg_index++;

  // Push the coordinates operands.
  auto coords = MakeCoordinateOperandsForImageAccess(inst);
  if (coords.empty()) {
    return false;
  }
  params.insert(params.end(), coords.begin(), coords.end());
  // Skip the coordinates operand.
  arg_index++;

  const auto num_args = inst.NumInOperands();

  std::string builtin_name;
  bool use_load_suffix = true;
  switch (opcode) {
    case SpvOpImageSampleImplicitLod:
    case SpvOpImageSampleExplicitLod:
      builtin_name = "textureSample";
      break;
    case SpvOpImageSampleDrefImplicitLod:
    case SpvOpImageSampleDrefExplicitLod:
      builtin_name = "textureSampleCompare";
      if (arg_index < num_args) {
        params.push_back(MakeOperand(inst, arg_index).expr);
        arg_index++;
      } else {
        return Fail()
               << "image depth-compare instruction is missing a Dref operand: "
               << inst.PrettyPrint();
      }
      break;
    case SpvOpImageGather:
    case SpvOpImageDrefGather:
      return Fail() << " image gather is not yet supported";
    case SpvOpImageQueryLod:
      return Fail() << " image query Lod is not yet supported";
    case SpvOpImageFetch:
      // Read a single texel from a sampled image.
      builtin_name = "textureLoad";
      use_load_suffix = false;
      break;
    case SpvOpImageRead:
      // Read a single texel from a storage image.
      builtin_name = "textureLoad";
      use_load_suffix = false;
      break;
    case SpvOpImageWrite:
      builtin_name = "textureStore";
      use_load_suffix = false;
      if (arg_index < num_args) {
        auto texel = MakeOperand(inst, arg_index);
        auto* converted_texel =
            ConvertTexelForStorage(inst, texel, texture_type);
        if (!converted_texel) {
          return false;
        }

        params.push_back(converted_texel);
        arg_index++;
      } else {
        return Fail() << "image write is missing a Texel operand: "
                      << inst.PrettyPrint();
      }
      break;
    default:
      return Fail() << "internal error: sampled image access";
  }

  // Loop over the image operands, looking for extra operands to the builtin.
  // Except we uroll the loop.
  uint32_t image_operands_mask = 0;
  if (arg_index < num_args) {
    image_operands_mask = inst.GetSingleWordInOperand(arg_index);
    arg_index++;
  }
  if (arg_index < num_args &&
      (image_operands_mask & SpvImageOperandsBiasMask)) {
    builtin_name += "Bias";
    params.push_back(MakeOperand(inst, arg_index).expr);
    image_operands_mask ^= SpvImageOperandsBiasMask;
    arg_index++;
  }
  if (arg_index < num_args && (image_operands_mask & SpvImageOperandsLodMask)) {
    if (use_load_suffix) {
      builtin_name += "Level";
    }
    TypedExpression lod = MakeOperand(inst, arg_index);
    // When sampling from a depth texture, the Lod operand must be an I32.
    if (texture_type->Is<ast::type::DepthTexture>()) {
      // Convert it to a signed integer type.
      lod = ToI32(lod);
    }
    params.push_back(lod.expr);
    image_operands_mask ^= SpvImageOperandsLodMask;
    arg_index++;
  }
  if (arg_index + 1 < num_args &&
      (image_operands_mask & SpvImageOperandsGradMask)) {
    builtin_name += "Grad";
    params.push_back(MakeOperand(inst, arg_index).expr);
    params.push_back(MakeOperand(inst, arg_index + 1).expr);
    image_operands_mask ^= SpvImageOperandsGradMask;
    arg_index += 2;
  }
  if (arg_index < num_args &&
      (image_operands_mask & SpvImageOperandsConstOffsetMask)) {
    if (!IsImageSampling(opcode)) {
      return Fail() << "ConstOffset is only permitted for sampling operations: "
                    << inst.PrettyPrint();
    }
    switch (texture_type->dim()) {
      case ast::type::TextureDimension::k2d:
      case ast::type::TextureDimension::k2dArray:
      case ast::type::TextureDimension::k3d:
        break;
      default:
        return Fail() << "ConstOffset is only permitted for 2D, 2D Arrayed, "
                         "and 3D textures: "
                      << inst.PrettyPrint();
    }

    params.push_back(ToSignedIfUnsigned(MakeOperand(inst, arg_index)).expr);
    image_operands_mask ^= SpvImageOperandsConstOffsetMask;
    arg_index++;
  }
  if (arg_index < num_args &&
      (image_operands_mask & SpvImageOperandsSampleMask)) {
    // TODO(dneto): only permitted with ImageFetch
    params.push_back(ToI32(MakeOperand(inst, arg_index)).expr);
    image_operands_mask ^= SpvImageOperandsSampleMask;
    arg_index++;
  }
  if (image_operands_mask) {
    return Fail() << "unsupported image operands (" << image_operands_mask
                  << "): " << inst.PrettyPrint();
  }

  auto* ident = create<ast::IdentifierExpression>(
      Source{}, ast_module_.RegisterSymbol(builtin_name), builtin_name);
  auto* call_expr =
      create<ast::CallExpression>(Source{}, ident, std::move(params));

  if (inst.type_id() != 0) {
    // It returns a value.
    ast::Expression* value = call_expr;

    // The result type, derived from the SPIR-V instruction.
    auto* result_type = parser_impl_.ConvertType(inst.type_id());
    auto* result_component_type = result_type;
    if (auto* result_vector_type = result_type->As<ast::type::Vector>()) {
      result_component_type = result_vector_type->type();
    }

    // Convert the arity of the result when operating on depth textures.
    // SPIR-V operations on depth textures always result in 4-element vectors.
    // But WGSL operations on depth textures always result in a f32 scalar.
    // Construct a 4-element vector with the result from the builtin in the
    // first component.
    if (texture_type->Is<ast::type::DepthTexture>()) {
      value = create<ast::TypeConstructorExpression>(
          Source{},
          result_type,  // a vec4
          ast::ExpressionList{
              value, parser_impl_.MakeNullValue(result_component_type),
              parser_impl_.MakeNullValue(result_component_type),
              parser_impl_.MakeNullValue(result_component_type)});
    }

    // If necessary, convert the result to the signedness of the instruction
    // result type. Compare the SPIR-V image's sampled component type with the
    // component of the result type of the SPIR-V instruction.
    auto* spirv_image_type =
        parser_impl_.GetSpirvTypeForHandleMemoryObjectDeclaration(*image);
    if (!spirv_image_type || (spirv_image_type->opcode() != SpvOpTypeImage)) {
      return Fail() << "invalid image type for image memory object declaration "
                    << image->PrettyPrint();
    }
    auto* expected_component_type =
        parser_impl_.ConvertType(spirv_image_type->GetSingleWordInOperand(0));
    if (expected_component_type != result_component_type) {
      // This occurs if one is signed integer and the other is unsigned integer,
      // or vice versa. Perform a bitcast.
      value = create<ast::BitcastExpression>(Source{}, result_type, call_expr);
    }
    if (!expected_component_type->Is<ast::type::F32>() &&
        IsSampledImageAccess(opcode)) {
      // WGSL permits sampled image access only on float textures.
      // Reject this case in the SPIR-V reader, at least until SPIR-V validation
      // catches up with this rule and can reject it earlier in the workflow.
      return Fail() << "sampled image must have float component type";
    }

    EmitConstDefOrWriteToHoistedVar(inst, {result_type, value});
  } else {
    // It's an image write. No value is returned, so make a statement out
    // of the call.
    AddStatement(create<ast::CallStatement>(Source{}, call_expr));
  }
  return success();
}

ast::ExpressionList FunctionEmitter::MakeCoordinateOperandsForImageAccess(
    const spvtools::opt::Instruction& inst) {
  if (!parser_impl_.success()) {
    Fail();
    return {};
  }
  if (inst.NumInOperands() == 0) {
    Fail() << "internal error: not an image access instruction: "
           << inst.PrettyPrint();
    return {};
  }
  const auto sampled_image_id = inst.GetSingleWordInOperand(0);
  const auto* image =
      parser_impl_.GetMemoryObjectDeclarationForHandle(sampled_image_id, true);
  if (!image) {
    Fail() << "internal error: couldn't find image for " << inst.PrettyPrint();
    return {};
  }
  if (image->NumInOperands() < 1) {
    Fail() << "image access is missing a coordinate parameter: "
           << inst.PrettyPrint();
    return {};
  }

  // In SPIR-V for Shader, coordinates are:
  //  - floating point for sampling, dref sampling, gather, dref gather
  //  - integral for fetch, read, write
  // In WGSL:
  //  - floating point for sampling, dref sampling, gather, dref gather
  //  - signed integral for textureLoad, textureStore
  //
  // The only conversions we have to do for WGSL are:
  //  - When the coordinates are unsigned integral, convert them to signed.
  //  - Array index is always i32

  // The coordinates parameter is always in position 1.
  TypedExpression raw_coords(MakeOperand(inst, 1));
  if (!raw_coords.type) {
    return {};
  }
  ast::type::Pointer* type = parser_impl_.GetTypeForHandleVar(*image);
  if (!parser_impl_.success()) {
    Fail();
    return {};
  }
  if (!type || !type->type()->Is<ast::type::Texture>()) {
    Fail() << "invalid texture type for " << image->PrettyPrint();
    return {};
  }
  ast::type::TextureDimension dim =
      type->type()->As<ast::type::Texture>()->dim();
  // Number of regular coordinates.
  uint32_t num_axes = 0;
  bool is_arrayed = false;
  switch (dim) {
    case ast::type::TextureDimension::k1d:
      num_axes = 1;
      break;
    case ast::type::TextureDimension::k1dArray:
      num_axes = 1;
      is_arrayed = true;
      break;
    case ast::type::TextureDimension::k2d:
      num_axes = 2;
      break;
    case ast::type::TextureDimension::k2dArray:
      num_axes = 2;
      is_arrayed = true;
      break;
    case ast::type::TextureDimension::k3d:
      num_axes = 3;
      break;
    case ast::type::TextureDimension::kCube:
      // For cubes, 3 coordinates form a direction vector.
      num_axes = 3;
      break;
    case ast::type::TextureDimension::kCubeArray:
      // For cubes, 3 coordinates form a direction vector.
      num_axes = 3;
      is_arrayed = true;
      break;
    default:
      Fail() << "unsupported image dimensionality for " << type->type_name()
             << " prompted by " << inst.PrettyPrint();
      return {};
  }
  assert(num_axes <= 3);
  const auto num_coords_required = num_axes + (is_arrayed ? 1 : 0);
  uint32_t num_coords_supplied = 0;
  auto* component_type = raw_coords.type;
  if (component_type->is_float_scalar() ||
      component_type->is_integer_scalar()) {
    num_coords_supplied = 1;
  } else if (auto* vec_type = raw_coords.type->As<ast::type::Vector>()) {
    component_type = vec_type->type();
    num_coords_supplied = vec_type->size();
  }
  if (num_coords_supplied == 0) {
    Fail() << "bad or unsupported coordinate type for image access: "
           << inst.PrettyPrint();
    return {};
  }
  if (num_coords_required > num_coords_supplied) {
    Fail() << "image access required " << num_coords_required
           << " coordinate components, but only " << num_coords_supplied
           << " provided, in: " << inst.PrettyPrint();
    return {};
  }

  ast::ExpressionList result;

  // Generates the expression for the WGSL coordinates, when it is a prefix
  // swizzle with num_axes.  If the result would be unsigned, also converts
  // it to a signed value of the same shape (scalar or vector).
  // Use a lambda to make it easy to only generate the expressions when we
  // will actually use them.
  auto prefix_swizzle_expr = [this, num_axes, component_type,
                              raw_coords]() -> ast::Expression* {
    auto* swizzle_type =
        (num_axes == 1) ? component_type
                        : create<ast::type::Vector>(component_type, num_axes);
    auto* swizzle = create<ast::MemberAccessorExpression>(
        Source{}, raw_coords.expr, PrefixSwizzle(num_axes));
    return ToSignedIfUnsigned({swizzle_type, swizzle}).expr;
  };

  if (is_arrayed) {
    // The source must be a vector. It has at least one coordinate component
    // and it must have an array component.  Use a vector swizzle to get the
    // first `num_axes` components.
    result.push_back(prefix_swizzle_expr());

    // Now get the array index.
    ast::Expression* array_index = create<ast::MemberAccessorExpression>(
        Source{}, raw_coords.expr, Swizzle(num_axes));
    // Convert it to a signed integer type, if needed
    result.push_back(ToI32({component_type, array_index}).expr);
  } else {
    if (num_coords_supplied == num_coords_required) {
      // Pass the value through, with possible unsigned->signed conversion.
      result.push_back(ToSignedIfUnsigned(raw_coords).expr);
    } else {
      // There are more coordinates supplied than needed. So the source type
      // is a vector. Use a vector swizzle to get the first `num_axes`
      // components.
      result.push_back(prefix_swizzle_expr());
    }
  }
  return result;
}

ast::Expression* FunctionEmitter::ConvertTexelForStorage(
    const spvtools::opt::Instruction& inst,
    TypedExpression texel,
    ast::type::Texture* texture_type) {
  auto* storage_texture_type = texture_type->As<ast::type::StorageTexture>();
  auto* src_type = texel.type;
  if (!storage_texture_type) {
    Fail() << "writing to other than storage texture: " << inst.PrettyPrint();
    return nullptr;
  }
  const auto format = storage_texture_type->image_format();
  auto* dest_type = parser_impl_.GetTexelTypeForFormat(format);
  if (!dest_type) {
    Fail();
    return nullptr;
  }
  if (src_type == dest_type) {
    return texel.expr;
  }

  const uint32_t dest_count =
      dest_type->is_scalar() ? 1 : dest_type->As<ast::type::Vector>()->size();
  if (dest_count == 3) {
    Fail() << "3-channel storage textures are not supported: "
           << inst.PrettyPrint();
    return nullptr;
  }
  const uint32_t src_count =
      src_type->is_scalar() ? 1 : src_type->As<ast::type::Vector>()->size();
  if (src_count < dest_count) {
    Fail() << "texel has too few components for storage texture: " << src_count
           << " provided but " << dest_count
           << " required, in: " << inst.PrettyPrint();
    return nullptr;
  }
  // If the texel has more components than necessary, then we will ignore the
  // higher-numbered components.
  auto* texel_prefix =
      (src_count == dest_count)
          ? texel.expr
          : create<ast::MemberAccessorExpression>(Source{}, texel.expr,
                                                  PrefixSwizzle(dest_count));

  if (!(dest_type->is_float_scalar_or_vector() ||
        dest_type->is_unsigned_scalar_or_vector() ||
        dest_type->is_signed_scalar_or_vector())) {
    Fail() << "invalid destination type for storage texture write: "
           << dest_type->type_name();
    return nullptr;
  }
  if (!(src_type->is_float_scalar_or_vector() ||
        src_type->is_unsigned_scalar_or_vector() ||
        src_type->is_signed_scalar_or_vector())) {
    Fail() << "invalid texel type for storage texture write: "
           << inst.PrettyPrint();
    return nullptr;
  }
  if (dest_type->is_float_scalar_or_vector() &&
      !src_type->is_float_scalar_or_vector()) {
    Fail() << "can only write float or float vector to a storage image with "
              "floating texel format: "
           << inst.PrettyPrint();
    return nullptr;
  }
  if (!dest_type->is_float_scalar_or_vector() &&
      src_type->is_float_scalar_or_vector()) {
    Fail()
        << "float or float vector can only be written to a storage image with "
           "floating texel format: "
        << inst.PrettyPrint();
    return nullptr;
  }

  if (dest_type->is_float_scalar_or_vector()) {
    return texel_prefix;
  }
  // The only remaining cases are signed/unsigned source, and signed/unsigned
  // destination.
  if (dest_type->is_unsigned_scalar_or_vector() ==
      src_type->is_unsigned_scalar_or_vector()) {
    return texel_prefix;
  }
  // We must do a bitcast conversion.
  return create<ast::BitcastExpression>(Source{}, dest_type, texel_prefix);
}

TypedExpression FunctionEmitter::ToI32(TypedExpression value) {
  if (!value.type || value.type == i32_) {
    return value;
  }
  return {i32_, create<ast::TypeConstructorExpression>(
                    Source{}, i32_, ast::ExpressionList{value.expr})};
}

TypedExpression FunctionEmitter::ToSignedIfUnsigned(TypedExpression value) {
  if (!value.type || !value.type->is_unsigned_scalar_or_vector()) {
    return value;
  }
  if (auto* vec_type = value.type->As<ast::type::Vector>()) {
    auto* new_type = create<ast::type::Vector>(i32_, vec_type->size());
    return {new_type, create<ast::TypeConstructorExpression>(
                          Source{}, new_type, ast::ExpressionList{value.expr})};
  }
  return ToI32(value);
}

TypedExpression FunctionEmitter::MakeArrayLength(
    const spvtools::opt::Instruction& inst) {
  if (inst.NumInOperands() != 2) {
    // Binary parsing will fail on this anyway.
    Fail() << "invalid array length: requires 2 operands: "
           << inst.PrettyPrint();
    return {};
  }
  const auto struct_ptr_id = inst.GetSingleWordInOperand(0);
  const auto field_index = inst.GetSingleWordInOperand(1);
  const auto struct_ptr_type_id =
      def_use_mgr_->GetDef(struct_ptr_id)->type_id();
  // Trace through the pointer type to get to the struct type.
  const auto struct_type_id =
      def_use_mgr_->GetDef(struct_ptr_type_id)->GetSingleWordInOperand(1);
  const auto field_name = namer_.GetMemberName(struct_type_id, field_index);
  if (field_name.empty()) {
    Fail() << "struct index out of bounds for array length: "
           << inst.PrettyPrint();
  }

  auto* member_ident = create<ast::IdentifierExpression>(
      Source{}, ast_module_.RegisterSymbol(field_name), field_name);
  auto* member_access = create<ast::MemberAccessorExpression>(
      Source{}, MakeExpression(struct_ptr_id).expr, member_ident);

  // Generate the intrinsic function call.
  std::string call_ident_str = "arrayLength";
  auto* call_ident = create<ast::IdentifierExpression>(
      Source{}, ast_module_.RegisterSymbol(call_ident_str), call_ident_str);
  call_ident->set_intrinsic(ast::Intrinsic::kArrayLength);

  ast::ExpressionList params{member_access};
  auto* call_expr =
      create<ast::CallExpression>(Source{}, call_ident, std::move(params));

  return {parser_impl_.ConvertType(inst.type_id()), call_expr};
}

FunctionEmitter::FunctionDeclaration::FunctionDeclaration() = default;
FunctionEmitter::FunctionDeclaration::~FunctionDeclaration() = default;

}  // namespace spirv
}  // namespace reader
}  // namespace tint

TINT_INSTANTIATE_CLASS_ID(tint::reader::spirv::StatementBuilder);
TINT_INSTANTIATE_CLASS_ID(tint::reader::spirv::SwitchStatementBuilder);
TINT_INSTANTIATE_CLASS_ID(tint::reader::spirv::IfStatementBuilder);
TINT_INSTANTIATE_CLASS_ID(tint::reader::spirv::LoopStatementBuilder);
