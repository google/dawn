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
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/program_builder.h"
#include "src/reader/spirv/construct.h"
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
  /// Is this block a continue target which is its own loop header block?
  /// In this case the continue construct is the entire loop.  The associated
  /// "loop construct" is empty, and not represented.
  bool is_continue_entire_loop = false;

  /// The immediately enclosing structured construct. If this block is not
  /// in the block order at all, then this is still nullptr.
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

  /// When this block is an if-selection header, this is the edge kind
  /// for the true branch.
  EdgeKind true_kind = EdgeKind::kForward;
  /// When this block is an if-selection header, this is the edge kind
  /// for the false branch.
  EdgeKind false_kind = EdgeKind::kForward;
  /// If not 0, then this block is an if-selection header, and `true_head` is
  /// the target id of the true branch on the OpBranchConditional, and that
  /// target is inside the if-selection.
  uint32_t true_head = 0;
  /// If not 0, then this block is an if-selection header, and `false_head`
  /// is the target id of the false branch on the OpBranchConditional, and
  /// that target is inside the if-selection.
  uint32_t false_head = 0;
  /// If not 0, then this block is an if-selection header, and when following
  /// the flow via the true and false branches, control first reconverges at
  /// the block with ID `premerge_head`, and `premerge_head` is still inside
  /// the if-selection.
  uint32_t premerge_head = 0;
  /// If non-empty, then this block is an if-selection header, and control flow
  /// in the body must be guarded by a boolean flow variable with this name.
  /// This occurs when a block in this selection has both an if-break edge, and
  /// also a different normal forward edge but without a merge instruction.
  std::string flow_guard_name = "";

  /// The result IDs that this block is responsible for declaring as a
  /// hoisted variable.
  /// @see DefInfo#requires_hoisted_def
  std::vector<uint32_t> hoisted_ids;

  /// A PhiAssignment represents the assignment of a value to the state
  /// variable associated with an OpPhi in a successor block.
  struct PhiAssignment {
    /// The ID of an OpPhi receiving a value from this basic block.
    uint32_t phi_id;
    /// The the value carried to the given OpPhi.
    uint32_t value;
  };
  /// If this basic block branches to a visited basic block containing phis,
  /// then this is the list of writes to the variables associated those phis.
  std::vector<PhiAssignment> phi_assignments;
  /// The IDs of OpPhi instructions which require their associated state
  /// variable to be declared in this basic block.
  std::vector<uint32_t> phis_needing_state_vars;
};

inline std::ostream& operator<<(std::ostream& o, const BlockInfo& bi) {
  o << "BlockInfo{"
    << " id: " << bi.id << " pos: " << bi.pos
    << " merge_for_header: " << bi.merge_for_header
    << " continue_for_header: " << bi.continue_for_header
    << " header_for_merge: " << bi.header_for_merge
    << " is_continue_entire_loop: " << int(bi.is_continue_entire_loop) << "}";
  return o;
}

/// Reasons for avoiding generating an intermediate value.
enum class SkipReason {
  /// `kDontSkip`: The value should be generated. Used for most values.
  kDontSkip,

  /// For remaining cases, the value is not generated.

  /// `kOpaqueObject`: used for any intermediate value which is an sampler,
  /// image,
  /// or sampled image, or any pointer to such object. Code is generated
  /// for those objects only when emitting the image instructions that access
  /// the image (read, write, sample, gather, fetch, or query). For example,
  /// when encountering an OpImageSampleExplicitLod, a call to the
  /// textureSampleLevel builtin function will be emitted, and the call will
  /// directly reference the underlying texture and sampler (variable or
  /// function parameter).
  kOpaqueObject,

  /// `kPointSizeBuiltinPointer`: the value is a pointer to the Position builtin
  /// variable.  Don't generate its address.  Avoid generating stores to this
  /// pointer.
  kPointSizeBuiltinPointer,
  /// `kPointSizeBuiltinValue`: the value is the value loaded from the
  /// PointSize builtin. Use 1.0f instead, because that's the only value
  /// supported by WebGPU.
  kPointSizeBuiltinValue,

  /// `kSampleIdBuiltinPointer`: the value is a pointer to the SampleId builtin
  /// variable.  Don't generate its address.
  kSampleIdBuiltinPointer,

  /// `kVertexIndexBuiltinPointer`: the value is a pointer to the VertexIndex
  /// builtin variable.  Don't generate its address.
  kVertexIndexBuiltinPointer,

  /// `kInstanceIndexBuiltinPointer`: the value is a pointer to the
  /// InstanceIndex
  /// builtin variable.  Don't generate its address.
  kInstanceIndexBuiltinPointer,

  /// `kSampleMaskInBuiltinPointer`: the value is a pointer to the SampleMaskIn
  /// builtin input variable.  Don't generate its address.
  kSampleMaskInBuiltinPointer,

  /// `kSampleMaskOutBuiltinPointer`: the value is a pointer to the SampleMask
  /// builtin output variable.
  kSampleMaskOutBuiltinPointer,
};

/// Bookkeeping info for a SPIR-V ID defined in the function, or some
/// module-scope variables. This will be valid for result IDs that are:
/// - defined in the function and:
///    - instructions that are not OpLabel, and not OpFunctionParameter
///    - are defined in a basic block visited in the block-order for the
///    function.
/// - certain module-scope builtin variables.
struct DefInfo {
  /// Constructor.
  /// @param def_inst the SPIR-V instruction defining the ID
  /// @param block_pos the position of the basic block where the ID is defined.
  /// @param index an ordering index for this local definition
  DefInfo(const spvtools::opt::Instruction& def_inst,
          uint32_t block_pos,
          size_t index);
  /// Destructor.
  ~DefInfo();

  /// The SPIR-V instruction that defines the ID.
  const spvtools::opt::Instruction& inst;
  /// The position of the first block in which this ID is visible, in function
  /// block order.  For IDs defined outside of the function, it is 0.
  /// For IDs defined in the function, it is the position of the block
  /// containing the definition of the ID.
  /// See method `FunctionEmitter::ComputeBlockOrderAndPositions`
  const uint32_t block_pos = 0;

  /// An index for uniquely and deterministically ordering all DefInfo records
  /// in a function.
  const size_t index = 0;

  /// The number of uses of this ID.
  uint32_t num_uses = 0;

  /// The block position of the last use of this ID, or 0 if it is not used
  /// at all.  The "last" ordering is determined by the function block order.
  uint32_t last_use_pos = 0;

  /// Is this value used in a construct other than the one in which it was
  /// defined?
  bool used_in_another_construct = false;

  /// True if this ID requires a WGSL 'const' definition, due to context. It
  /// might get one anyway (so this is *not* an if-and-only-if condition).
  bool requires_named_const_def = false;

  /// True if this ID must map to a WGSL variable declaration before the
  /// corresponding position of the ID definition in SPIR-V.  This compensates
  /// for the difference between dominance and scoping. An SSA definition can
  /// dominate all its uses, but the construct where it is defined does not
  /// enclose all the uses, and so if it were declared as a WGSL constant
  /// definition at the point of its SPIR-V definition, then the WGSL name
  /// would go out of scope too early. Fix that by creating a variable at the
  /// top of the smallest construct that encloses both the definition and all
  /// its uses. Then the original SPIR-V definition maps to a WGSL assignment
  /// to that variable, and each SPIR-V use becomes a WGSL read from the
  /// variable.
  /// TODO(dneto): This works for constants of storable type, but not, for
  /// example, pointers.
  bool requires_hoisted_def = false;

  /// If the definition is an OpPhi, then `phi_var` is the name of the
  /// variable that stores the value carried from parent basic blocks into
  /// the basic block containing the OpPhi. Otherwise this is the empty string.
  std::string phi_var;

  /// The storage class to use for this value, if it is of pointer type.
  /// This is required to carry a storage class override from a storage
  /// buffer expressed in the old style (with Uniform storage class)
  /// that needs to be remapped to StorageBuffer storage class.
  /// This is kNone for non-pointers.
  ast::StorageClass storage_class = ast::StorageClass::kNone;

  /// The reason, if any, that this value should be ignored.
  /// Normally no values are ignored.  This field can be updated while
  /// generating code because sometimes we only discover necessary facts
  /// in the middle of generating code.
  SkipReason skip = SkipReason::kDontSkip;
};

inline std::ostream& operator<<(std::ostream& o, const DefInfo& di) {
  o << "DefInfo{"
    << " inst.result_id: " << di.inst.result_id()
    << " block_pos: " << di.block_pos << " num_uses: " << di.num_uses
    << " last_use_pos: " << di.last_use_pos << " requires_named_const_def: "
    << (di.requires_named_const_def ? "true" : "false")
    << " requires_hoisted_def: " << (di.requires_hoisted_def ? "true" : "false")
    << " phi_var: '" << di.phi_var << "'";
  if (di.storage_class != ast::StorageClass::kNone) {
    o << " sc:" << int(di.storage_class);
  }
  switch (di.skip) {
    case SkipReason::kDontSkip:
      break;
    case SkipReason::kOpaqueObject:
      o << " skip:opaque";
      break;
    case SkipReason::kPointSizeBuiltinPointer:
      o << " skip:pointsize_pointer";
      break;
    case SkipReason::kPointSizeBuiltinValue:
      o << " skip:pointsize_value";
      break;
    case SkipReason::kSampleIdBuiltinPointer:
      o << " skip:sampleid_pointer";
      break;
    case SkipReason::kVertexIndexBuiltinPointer:
      o << " skip:vertexindex_pointer";
      break;
    case SkipReason::kInstanceIndexBuiltinPointer:
      o << " skip:instanceindex_pointer";
      break;
    case SkipReason::kSampleMaskInBuiltinPointer:
      o << " skip:samplemaskin_pointer";
      break;
    case SkipReason::kSampleMaskOutBuiltinPointer:
      o << " skip:samplemaskout_pointer";
      break;
  }
  o << "}";
  return o;
}

/// A placeholder Statement that exists for the duration of building a
/// StatementBlock. Once the StatementBlock is built, Build() will be called to
/// construct the final AST node, which will be used in the place of this
/// StatementBuilder.
/// StatementBuilders are used to simplify construction of AST nodes that will
/// become immutable. The builders may hold mutable state while the
/// StatementBlock is being constructed, which becomes an immutable node on
/// StatementBlock::Finalize().
class StatementBuilder : public Castable<StatementBuilder, ast::Statement> {
 public:
  /// Constructor
  StatementBuilder() : Base(ProgramID(), Source{}) {}

  /// @param builder the program builder
  /// @returns the build AST node
  virtual ast::Statement* Build(ProgramBuilder* builder) const = 0;

 private:
  Node* Clone(CloneContext*) const override;
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;
};

/// A FunctionEmitter emits a SPIR-V function onto a Tint AST module.
class FunctionEmitter {
 public:
  /// Creates a FunctionEmitter, and prepares to write to the AST module
  /// in `pi`
  /// @param pi a ParserImpl which has already executed BuildInternalModule
  /// @param function the function to emit
  FunctionEmitter(ParserImpl* pi, const spvtools::opt::Function& function);
  /// Creates a FunctionEmitter, and prepares to write to the AST module
  /// in `pi`
  /// @param pi a ParserImpl which has already executed BuildInternalModule
  /// @param function the function to emit
  /// @param ep_info entry point information for this function, or nullptr
  FunctionEmitter(ParserImpl* pi,
                  const spvtools::opt::Function& function,
                  const EntryPointInfo* ep_info);
  /// Move constructor. Only valid when the other object was newly created.
  /// @param other the emitter to clone
  FunctionEmitter(FunctionEmitter&& other);
  /// Destructor
  ~FunctionEmitter();

  /// Emits the function to AST module.
  /// @return whether emission succeeded
  bool Emit();

  /// @returns true if emission has not yet failed.
  bool success() const { return fail_stream_.status(); }
  /// @returns true if emission has failed.
  bool failed() const { return !success(); }

  /// Finalizes any StatementBuilders returns the body of the function.
  /// Must only be called once, and to be used only for testing.
  /// @returns the body of the function.
  const ast::StatementList ast_body();

  /// Records failure.
  /// @returns a FailStream on which to emit diagnostics.
  FailStream& Fail() { return fail_stream_.Fail(); }

  /// @returns the parser implementation
  ParserImpl* parser() { return &parser_impl_; }

  /// Emits the entry point as a wrapper around its implementation function.
  /// @returns false if emission failed.
  bool EmitEntryPointAsWrapper();

  /// Create an ast::BlockStatement representing the body of the function.
  /// This creates the statement stack, which is non-empty for the lifetime
  /// of the function.
  /// @returns the body of the function, or null on error
  ast::BlockStatement* MakeFunctionBody();

  /// Emits the function body, populating the bottom entry of the statements
  /// stack.
  /// @returns false if emission failed.
  bool EmitBody();

  /// Records a mapping from block ID to a BlockInfo struct.
  /// Populates `block_info_`
  void RegisterBasicBlocks();

  /// Verifies that terminators only branch to labels in the current function.
  /// Assumes basic blocks have been registered.
  /// @returns true if terminators are valid
  bool TerminatorsAreValid();

  /// Populates merge-header cross-links and BlockInfo#is_continue_entire_loop.
  /// Also verifies that merge instructions go to blocks in the same function.
  /// Assumes basic blocks have been registered, and terminators are valid.
  /// @returns false if registration fails
  bool RegisterMerges();

  /// Determines the output order for the basic blocks in the function.
  /// Populates `block_order_` and BlockInfo#pos.
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
  /// Populates BlockInfo#construct and the `constructs_` list.
  /// Assumes terminators are valid and merges have been registered, block
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
  /// Also checks validity of each edge (populates BlockInfo#succ_edge).
  /// Implicitly checks dominance rules for headers and continue constructs.
  /// Assumes each block has been labeled with its control flow construct.
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

  /// Creates a DefInfo record for each module-scope builtin variable
  /// that should be handled specially.  Either it's ignored, or its store
  /// type is converted on load.
  /// Populates the `def_info_` mapping for such IDs.
  /// @returns false on failure
  bool RegisterSpecialBuiltInVariables();

  /// Creates a DefInfo record for each locally defined SPIR-V ID.
  /// Populates the `def_info_` mapping with basic results for such IDs.
  /// @returns false on failure
  bool RegisterLocallyDefinedValues();

  /// Returns the Tint storage class for the given SPIR-V ID that is a
  /// pointer value.
  /// @param id a SPIR-V ID for a pointer value
  /// @returns the storage class
  ast::StorageClass GetStorageClassForPointerValue(uint32_t id);

  /// Remaps the storage class for the type of a locally-defined value,
  /// if necessary. If it's not a pointer type, or if its storage class
  /// already matches, then the result is a copy of the `type` argument.
  /// @param type the AST type
  /// @param result_id the SPIR-V ID for the locally defined value
  /// @returns an possibly updated type
  const Type* RemapStorageClass(const Type* type, uint32_t result_id);

  /// Marks locally defined values when they should get a 'const'
  /// definition in WGSL, or a 'var' definition at an outer scope.
  /// This occurs in several cases:
  ///  - When a SPIR-V instruction might use the dynamically computed value
  ///    only once, but the WGSL code might reference it multiple times.
  ///    For example, this occurs for the vector operands of OpVectorShuffle.
  ///    In this case the definition's DefInfo#requires_named_const_def property
  ///    is set to true.
  ///  - When a definition and at least one of its uses are not in the
  ///    same structured construct.
  ///    In this case the definition's DefInfo#requires_named_const_def property
  ///    is set to true.
  ///  - When a definition is in a construct that does not enclose all the
  ///    uses.  In this case the definition's DefInfo#requires_hoisted_def
  ///    property is set to true.
  /// Updates the `def_info_` mapping.
  void FindValuesNeedingNamedOrHoistedDefinition();

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

  /// Emits a SwitchStatement, including its condition expression, and sets
  /// up the statement stack to accumulate subsequent basic blocks into
  /// the default clause and case clauses.
  /// @param block_info the switch-selection header block
  /// @returns false if emission failed.
  bool EmitSwitchStart(const BlockInfo& block_info);

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
  /// The `already_emitted` parameter indicates whether the code has already
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
  /// WGSL statement is required, the statement will be nullptr. This method
  /// tries to avoid emitting a 'break' statement when that would be redundant
  /// in WGSL due to implicit breaking out of a switch.
  /// @param src_info the source block
  /// @param dest_info the destination block
  /// @returns the new statement, or a null statement
  ast::Statement* MakeBranch(const BlockInfo& src_info,
                             const BlockInfo& dest_info) const {
    return MakeBranchDetailed(src_info, dest_info, false, nullptr);
  }

  /// Returns a new statement to represent the given branch representing a
  /// "normal" terminator, as in the sense of EmitNormalTerminator.  If no
  /// WGSL statement is required, the statement will be nullptr.
  /// @param src_info the source block
  /// @param dest_info the destination block
  /// @returns the new statement, or a null statement
  ast::Statement* MakeForcedBranch(const BlockInfo& src_info,
                                   const BlockInfo& dest_info) const {
    return MakeBranchDetailed(src_info, dest_info, true, nullptr);
  }

  /// Returns a new statement to represent the given branch representing a
  /// "normal" terminator, as in the sense of EmitNormalTerminator.  If no
  /// WGSL statement is required, the statement will be nullptr. When `forced`
  /// is false, this method tries to avoid emitting a 'break' statement when
  /// that would be redundant in WGSL due to implicit breaking out of a switch.
  /// When `forced` is true, the method won't try to avoid emitting that break.
  /// If the control flow edge is an if-break for an if-selection with a
  /// control flow guard, then return that guard name via `flow_guard_name_ptr`
  /// when that parameter is not null.
  /// @param src_info the source block
  /// @param dest_info the destination block
  /// @param forced if true, always emit the branch (if it exists in WGSL)
  /// @param flow_guard_name_ptr return parameter for control flow guard name
  /// @returns the new statement, or a null statement
  ast::Statement* MakeBranchDetailed(const BlockInfo& src_info,
                                     const BlockInfo& dest_info,
                                     bool forced,
                                     std::string* flow_guard_name_ptr) const;

  /// Returns a new if statement with the given statements as the then-clause
  /// and the else-clause.  Either or both clauses might be nullptr. If both
  /// are nullptr, then don't make a new statement and instead return nullptr.
  /// @param condition the branching condition
  /// @param then_stmt the statement for the then clause of the if, or nullptr
  /// @param else_stmt the statement for the else clause of the if, or nullptr
  /// @returns the new statement, or nullptr
  ast::Statement* MakeSimpleIf(ast::Expression* condition,
                               ast::Statement* then_stmt,
                               ast::Statement* else_stmt) const;

  /// Emits the statements for an normal-terminator OpBranchConditional
  /// where one branch is a case fall through (the true branch if and only
  /// if `fall_through_is_true_branch` is true), and the other branch is
  /// goes to a different destination, named by `other_dest`.
  /// @param src_info the basic block from which we're branching
  /// @param cond the branching condition
  /// @param other_edge_kind the edge kind from the source block to the other
  /// destination
  /// @param other_dest the other branching destination
  /// @param fall_through_is_true_branch true when the fall-through is the true
  /// branch
  /// @returns the false if emission fails
  bool EmitConditionalCaseFallThrough(const BlockInfo& src_info,
                                      ast::Expression* cond,
                                      EdgeKind other_edge_kind,
                                      const BlockInfo& other_dest,
                                      bool fall_through_is_true_branch);

  /// Emits a normal instruction: not a terminator, label, or variable
  /// declaration.
  /// @param inst the instruction
  /// @returns false if emission failed.
  bool EmitStatement(const spvtools::opt::Instruction& inst);

  /// Emits a const definition for the typed value in `ast_expr`, and
  /// records it as the translation for the result ID from `inst`.
  /// @param inst the SPIR-V instruction defining the value
  /// @param ast_expr the already-computed AST expression for the value
  /// @returns false if emission failed.
  bool EmitConstDefinition(const spvtools::opt::Instruction& inst,
                           TypedExpression ast_expr);

  /// Emits a write of the typed value in `ast_expr` to a hoisted variable
  /// for the given SPIR-V ID, if that ID has a hoisted declaration. Otherwise,
  /// emits a const definition instead.
  /// @param inst the SPIR-V instruction defining the value
  /// @param ast_expr the already-computed AST expression for the value
  /// @returns false if emission failed.
  bool EmitConstDefOrWriteToHoistedVar(const spvtools::opt::Instruction& inst,
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

  /// Creates an expression and supporting statements for the a GLSL.std.450
  /// extended instruction.
  /// @param inst a SPIR-V OpExtInst instruction from GLSL.std.450
  /// @returns an AST expression for the instruction, or nullptr.
  TypedExpression EmitGlslStd450ExtInst(const spvtools::opt::Instruction& inst);

  /// Creates an expression for OpCompositeExtract
  /// @param inst an OpCompositeExtract instruction.
  /// @returns an AST expression for the instruction, or nullptr.
  TypedExpression MakeCompositeExtract(const spvtools::opt::Instruction& inst);

  /// Creates an expression for indexing into a composite value.  The literal
  /// indices that step into the value start at instruction input operand
  /// `start_index` and run to the end of the instruction.
  /// @param inst the original instruction
  /// @param composite the typed expression for the composite
  /// @param composite_type_id the SPIR-V type ID for the composite
  /// @param index_start the index of the first operand in `inst` that is an
  /// index into the composite type
  /// @returns an AST expression for the decomposed composite, or {} on error
  TypedExpression MakeCompositeValueDecomposition(
      const spvtools::opt::Instruction& inst,
      TypedExpression composite,
      uint32_t composite_type_id,
      int index_start);

  /// Creates an expression for OpVectorShuffle
  /// @param inst an OpVectorShuffle instruction.
  /// @returns an AST expression for the instruction, or nullptr.
  TypedExpression MakeVectorShuffle(const spvtools::opt::Instruction& inst);

  /// Creates an expression for a numeric conversion.
  /// @param inst a numeric conversion instruction
  /// @returns an AST expression for the instruction, or nullptr.
  TypedExpression MakeNumericConversion(const spvtools::opt::Instruction& inst);

  /// Gets the block info for a block ID, if any exists
  /// @param id the SPIR-V ID of the OpLabel instruction starting the block
  /// @returns the block info for the given ID, if it exists, or nullptr
  BlockInfo* GetBlockInfo(uint32_t id) const {
    auto where = block_info_.find(id);
    if (where == block_info_.end()) {
      return nullptr;
    }
    return where->second.get();
  }

  /// Gets the local definition info for a result ID.
  /// @param id the SPIR-V ID of local definition.
  /// @returns the definition info for the given ID, if it exists, or nullptr
  DefInfo* GetDefInfo(uint32_t id) const {
    auto where = def_info_.find(id);
    if (where == def_info_.end()) {
      return nullptr;
    }
    return where->second.get();
  }
  /// Returns the skip reason for a result ID.
  /// @param id SPIR-V result ID
  /// @returns the skip reason for the given ID, or SkipReason::kDontSkip
  SkipReason GetSkipReason(uint32_t id) const {
    if (auto* def_info = GetDefInfo(id)) {
      return def_info->skip;
    }
    return SkipReason::kDontSkip;
  }

  /// Returns the WGSL variable name for an input builtin variable whose
  /// translation is managed via the SkipReason mechanism.
  /// @param skip_reason the skip reason for the special variable
  /// @returns the variable name for a special builtin variable
  /// that is handled via the "skip" mechanism.
  std::string NameForSpecialInputBuiltin(SkipReason skip_reason);

  /// Returns the most deeply nested structured construct which encloses the
  /// WGSL scopes of names declared in both block positions. Each position must
  /// be a valid index into the function block order array.
  /// @param first_pos the first block position
  /// @param last_pos the last block position
  /// @returns the smallest construct containing both positions
  const Construct* GetEnclosingScope(uint32_t first_pos,
                                     uint32_t last_pos) const;

  /// Finds loop construct associated with a continue construct, if it exists.
  /// Returns nullptr if:
  ///  - the given construct is not a continue construct
  ///  - the continue construct does not have an associated loop construct
  ///    (the continue target is also the loop header block)
  /// @param c the continue construct
  /// @returns the associated loop construct, or nullptr
  const Construct* SiblingLoopConstruct(const Construct* c) const;

  /// Returns an identifier expression for the swizzle name of the given
  /// index into a vector.  Emits an error and returns nullptr if the
  /// index is out of range, i.e. 4 or higher.
  /// @param i index of the subcomponent
  /// @returns the identifier expression for the `i`'th component
  ast::IdentifierExpression* Swizzle(uint32_t i);

  /// Returns an identifier expression for the swizzle name of the first
  /// `n` elements of a vector.  Emits an error and returns nullptr if `n`
  /// is out of range, i.e. 4 or higher.
  /// @param n the number of components in the swizzle
  /// @returns the swizzle identifier for the first n elements of a vector
  ast::IdentifierExpression* PrefixSwizzle(uint32_t n);

  /// Converts SPIR-V image coordinates from an image access instruction
  /// (e.g. OpImageSampledImplicitLod) into an expression list consisting of
  /// the texture coordinates, and an integral array index if the texture is
  /// arrayed. The texture coordinate is a scalar for 1D textures, a vector of
  /// 2 elements for a 2D texture, and a vector of 3 elements for a 3D or
  /// Cube texture. Excess components are ignored, e.g. if the SPIR-V
  /// coordinate is a 4-element vector but the image is a 2D non-arrayed
  /// texture then the 3rd and 4th components are ignored.
  /// On failure, issues an error and returns an empty expression list.
  /// @param image_access the image access instruction
  /// @returns an ExpressionList of the coordinate and array index (if any)
  ast::ExpressionList MakeCoordinateOperandsForImageAccess(
      const spvtools::opt::Instruction& image_access);

  /// Returns the given value as an I32.  If it's already an I32 then this
  /// return the given value.  Otherwise, wrap the value in a TypeConstructor
  /// expression.
  /// @param value the value to pass through or convert
  /// @returns the value as an I32 value.
  TypedExpression ToI32(TypedExpression value);

  /// Returns the given value as a signed integer type of the same shape
  /// if the value is unsigned scalar or vector, by wrapping the value
  /// with a TypeConstructor expression.  Returns the value itself if the
  /// value otherwise.
  /// @param value the value to pass through or convert
  /// @returns the value itself, or converted to signed integral
  TypedExpression ToSignedIfUnsigned(TypedExpression value);

  /// Returns true if the given SPIR-V id represents a constant float 0.
  bool IsFloatZero(uint32_t value_id);
  /// Returns true if the given SPIR-V id represents a constant float 1.
  bool IsFloatOne(uint32_t value_id);

 private:
  /// FunctionDeclaration contains the parsed information for a function header.
  struct FunctionDeclaration {
    /// Constructor
    FunctionDeclaration();
    /// Destructor
    ~FunctionDeclaration();

    /// Parsed header source
    Source source;
    /// Function name
    std::string name;
    /// Function parameters
    ast::VariableList params;
    /// Function return type
    const Type* return_type;
    /// Function decorations
    ast::DecorationList decorations;
  };

  /// Parse the function declaration, which comprises the name, parameters, and
  /// return type, populating `decl`.
  /// @param decl the FunctionDeclaration to populate
  /// @returns true if emission has not yet failed.
  bool ParseFunctionDeclaration(FunctionDeclaration* decl);

  /// @returns the store type for the OpVariable instruction, or
  /// null on failure.
  const Type* GetVariableStoreType(
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

  /// Emits a function call.  On failure, emits a diagnostic and returns false.
  /// @param inst the SPIR-V function call instruction
  /// @returns false if emission failed
  bool EmitFunctionCall(const spvtools::opt::Instruction& inst);

  /// Emits a control barrier intrinsic.  On failure, emits a diagnostic and
  /// returns false.
  /// @param inst the SPIR-V control barrier instruction
  /// @returns false if emission failed
  bool EmitControlBarrier(const spvtools::opt::Instruction& inst);

  /// Returns an expression for a SPIR-V instruction that maps to a WGSL
  /// intrinsic function call.
  /// @param inst the SPIR-V instruction
  /// @returns an expression
  TypedExpression MakeIntrinsicCall(const spvtools::opt::Instruction& inst);

  /// Returns an expression for a SPIR-V OpArrayLength instruction.
  /// @param inst the SPIR-V instruction
  /// @returns an expression
  TypedExpression MakeArrayLength(const spvtools::opt::Instruction& inst);

  /// Generates an expression for a SPIR-V OpOuterProduct instruction.
  /// @param inst the SPIR-V instruction
  /// @returns an expression
  TypedExpression MakeOuterProduct(const spvtools::opt::Instruction& inst);

  /// Generates statements for a SPIR-V OpVectorInsertDynamic instruction.
  /// Registers a const declaration for the result.
  /// @param inst the SPIR-V instruction
  /// @returns an expression
  bool MakeVectorInsertDynamic(const spvtools::opt::Instruction& inst);

  /// Generates statements for a SPIR-V OpComposite instruction.
  /// Registers a const declaration for the result.
  /// @param inst the SPIR-V instruction
  /// @returns an expression
  bool MakeCompositeInsert(const spvtools::opt::Instruction& inst);

  /// Get the SPIR-V instruction for the image memory object declaration for
  /// the image operand to the given instruction.
  /// @param inst the SPIR-V instruction
  /// @returns a SPIR-V OpVariable or OpFunctionParameter instruction, or null
  /// on error
  const spvtools::opt::Instruction* GetImage(
      const spvtools::opt::Instruction& inst);

  /// Get the AST texture the SPIR-V image memory object declaration.
  /// @param inst the SPIR-V memory object declaration for the image.
  /// @returns a texture type, or null on error
  const Texture* GetImageType(const spvtools::opt::Instruction& inst);

  /// Get the expression for the image operand from the first operand to the
  /// given instruction.
  /// @param inst the SPIR-V instruction
  /// @returns an identifier expression, or null on error
  ast::Expression* GetImageExpression(const spvtools::opt::Instruction& inst);

  /// Get the expression for the sampler operand from the first operand to the
  /// given instruction.
  /// @param inst the SPIR-V instruction
  /// @returns an identifier expression, or null on error
  ast::Expression* GetSamplerExpression(const spvtools::opt::Instruction& inst);

  /// Emits a texture builtin function call for a SPIR-V instruction that
  /// accesses an image or sampled image.
  /// @param inst the SPIR-V instruction
  /// @returns an expression
  bool EmitImageAccess(const spvtools::opt::Instruction& inst);

  /// Emits statements to implement a SPIR-V image query.
  /// @param inst the SPIR-V instruction
  /// @returns an expression
  bool EmitImageQuery(const spvtools::opt::Instruction& inst);

  /// Converts the given texel to match the type required for the storage
  /// texture with the given type. In WGSL the texel value is always provided
  /// as a 4-element vector, but the component type is determined by the
  /// texel channel type. See "Texel Formats for Storage Textures" in the WGSL
  /// spec. Returns an expression, or emits an error and returns nullptr.
  /// @param inst the image access instruction (used for diagnostics)
  /// @param texel the texel
  /// @param texture_type the type of the storage texture
  /// @returns the texel, after necessary conversion.
  ast::Expression* ConvertTexelForStorage(
      const spvtools::opt::Instruction& inst,
      TypedExpression texel,
      const Texture* texture_type);

  /// Returns an expression for an OpSelect, if its operands are scalars
  /// or vectors. These translate directly to WGSL select.  Otherwise, return
  /// an expression with a null owned expression
  /// @param inst the SPIR-V OpSelect instruction
  /// @returns a typed expression, or one with a null owned expression
  TypedExpression MakeSimpleSelect(const spvtools::opt::Instruction& inst);

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
  ast::Statement* AddStatement(ast::Statement* statement);

  /// AddStatementBuilder() constructs and adds the StatementBuilder of type
  /// `T` to the top of the statement stack.
  /// @param args the arguments forwarded to the T constructor
  /// @return the built StatementBuilder
  template <typename T, typename... ARGS>
  T* AddStatementBuilder(ARGS&&... args) {
    TINT_ASSERT(!statements_stack_.empty());
    return statements_stack_.back().AddStatementBuilder<T>(
        std::forward<ARGS>(args)...);
  }

  /// Returns the source record for the given instruction.
  /// @param inst the SPIR-V instruction
  /// @return the Source record, or a default one
  Source GetSourceForInst(const spvtools::opt::Instruction& inst) const;

  /// @returns the last statetment in the top of the statement stack.
  ast::Statement* LastStatement();

  using CompletionAction = std::function<void(const ast::StatementList&)>;

  // A StatementBlock represents a braced-list of statements while it is being
  // constructed.
  class StatementBlock {
   public:
    StatementBlock(const Construct* construct,
                   uint32_t end_id,
                   CompletionAction completion_action);
    StatementBlock(StatementBlock&&);
    ~StatementBlock();

    StatementBlock(const StatementBlock&) = delete;
    StatementBlock& operator=(const StatementBlock&) = delete;

    /// Replaces any StatementBuilders with the built result, and calls the
    /// completion callback (if set). Must only be called once, after all
    /// statements have been added with Add().
    /// @param builder the program builder
    void Finalize(ProgramBuilder* builder);

    /// Add() adds `statement` to the block.
    /// Add() must not be called after calling Finalize().
    void Add(ast::Statement* statement);

    /// AddStatementBuilder() constructs and adds the StatementBuilder of type
    /// `T` to the block.
    /// Add() must not be called after calling Finalize().
    /// @param args the arguments forwarded to the T constructor
    /// @return the built StatementBuilder
    template <typename T, typename... ARGS>
    T* AddStatementBuilder(ARGS&&... args) {
      auto builder = std::make_unique<T>(std::forward<ARGS>(args)...);
      auto* ptr = builder.get();
      Add(ptr);
      builders_.emplace_back(std::move(builder));
      return ptr;
    }

    /// @param construct the construct which this construct constributes to
    void SetConstruct(const Construct* construct) { construct_ = construct; }

    /// @return the construct to which this construct constributes
    const Construct* GetConstruct() const { return construct_; }

    /// @return the ID of the block at which the completion action should be
    /// triggered and this statement block discarded. This is often the `end_id`
    /// of `construct` itself.
    uint32_t GetEndId() const { return end_id_; }

    /// @return the list of statements being built, if this construct is not a
    /// switch.
    const ast::StatementList& GetStatements() const { return statements_; }

   private:
    /// The construct to which this construct constributes.
    const Construct* construct_;
    /// The ID of the block at which the completion action should be triggered
    /// and this statement block discarded. This is often the `end_id` of
    /// `construct` itself.
    uint32_t const end_id_;
    /// The completion action finishes processing this statement block.
    FunctionEmitter::CompletionAction const completion_action_;
    /// The list of statements being built, if this construct is not a switch.
    ast::StatementList statements_;

    /// Owned statement builders
    std::vector<std::unique_ptr<StatementBuilder>> builders_;
    /// True if Finalize() has been called.
    bool finalized_ = false;
  };

  /// Pushes an empty statement block onto the statements stack.
  /// @param action the completion action for this block
  void PushNewStatementBlock(const Construct* construct,
                             uint32_t end_id,
                             CompletionAction action);

  /// Emits an if-statement whose condition is the given flow guard
  /// variable, and pushes onto the statement stack the corresponding
  /// statement block ending (and not including) the given block.
  /// @param flow_guard name of the flow guard variable
  /// @param end_id first block after the if construct.
  void PushGuard(const std::string& flow_guard, uint32_t end_id);

  /// Emits an if-statement with 'true' condition, and pushes onto the
  /// statement stack the corresponding statement block ending (and not
  /// including) the given block.
  /// @param end_id first block after the if construct.
  void PushTrueGuard(uint32_t end_id);

  /// @returns a boolean true expression.
  ast::Expression* MakeTrue(const Source&) const;

  /// @returns a boolean false expression.
  ast::Expression* MakeFalse(const Source&) const;

  /// @param expr the expression to take the address of
  /// @returns a TypedExpression that is the address-of `expr` (`&expr`)
  /// @note `expr` must be a reference type
  TypedExpression AddressOf(TypedExpression expr);

  /// @param expr the expression to dereference
  /// @returns a TypedExpression that is the dereference-of `expr` (`*expr`)
  /// @note `expr` must be a pointer type
  TypedExpression Dereference(TypedExpression expr);

  /// Creates a new `ast::Node` owned by the ProgramBuilder.
  /// @param args the arguments to pass to the type constructor
  /// @returns the node pointer
  template <typename T, typename... ARGS>
  T* create(ARGS&&... args) const {
    return builder_.create<T>(std::forward<ARGS>(args)...);
  }

  using StatementsStack = std::vector<StatementBlock>;
  using PtrAs = ParserImpl::PtrAs;

  ParserImpl& parser_impl_;
  TypeManager& ty_;
  ProgramBuilder& builder_;
  spvtools::opt::IRContext& ir_context_;
  spvtools::opt::analysis::DefUseManager* def_use_mgr_;
  spvtools::opt::analysis::ConstantManager* constant_mgr_;
  spvtools::opt::analysis::TypeManager* type_mgr_;
  FailStream& fail_stream_;
  Namer& namer_;
  const spvtools::opt::Function& function_;

  // The SPIR-V ID for the SampleMask input variable.
  uint32_t sample_mask_in_id;
  // The SPIR-V ID for the SampleMask output variable.
  uint32_t sample_mask_out_id;

  // A stack of statement lists. Each list is contained in a construct in
  // the next deeper element of stack. The 0th entry represents the statements
  // for the entire function.  This stack is never empty.
  // The `construct` member for the 0th element is only valid during the
  // lifetime of the EmitFunctionBodyStatements method.
  StatementsStack statements_stack_;

  // The map of IDs that have already had an identifier name generated for it,
  // to their Type.
  std::unordered_map<uint32_t, const Type*> identifier_types_;
  // Mapping from SPIR-V ID that is used at most once, to its AST expression.
  std::unordered_map<uint32_t, TypedExpression> singly_used_values_;

  // The IDs of basic blocks, in reverse structured post-order (RSPO).
  // This is the output order for the basic blocks.
  std::vector<uint32_t> block_order_;

  // Mapping from block ID to its bookkeeping info.
  std::unordered_map<uint32_t, std::unique_ptr<BlockInfo>> block_info_;

  // Mapping from a locally-defined result ID to its bookkeeping info.
  std::unordered_map<uint32_t, std::unique_ptr<DefInfo>> def_info_;

  // Structured constructs, where enclosing constructs precede their children.
  ConstructList constructs_;

  // Information about entry point, if this function is referenced by one
  const EntryPointInfo* ep_info_ = nullptr;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_FUNCTION_H_
