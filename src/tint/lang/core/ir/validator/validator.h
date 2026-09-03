// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_CORE_IR_VALIDATOR_VALIDATOR_H_
#define SRC_TINT_LANG_CORE_IR_VALIDATOR_VALIDATOR_H_

#include <cstdint>
#include <functional>
#include <string>

#include "src/tint/lang/core/ir/access.h"
#include "src/tint/lang/core/ir/array_count.h"
#include "src/tint/lang/core/ir/binary.h"
#include "src/tint/lang/core/ir/block_param.h"
#include "src/tint/lang/core/ir/break_if.h"
#include "src/tint/lang/core/ir/construct.h"
#include "src/tint/lang/core/ir/continue.h"
#include "src/tint/lang/core/ir/control_instruction.h"
#include "src/tint/lang/core/ir/convert.h"
#include "src/tint/lang/core/ir/core_binary.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/disassembler.h"
#include "src/tint/lang/core/ir/discard.h"
#include "src/tint/lang/core/ir/exit_if.h"
#include "src/tint/lang/core/ir/exit_loop.h"
#include "src/tint/lang/core/ir/exit_switch.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/function_param.h"
#include "src/tint/lang/core/ir/if.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/lang/core/ir/instruction_result.h"
#include "src/tint/lang/core/ir/let.h"
#include "src/tint/lang/core/ir/load.h"
#include "src/tint/lang/core/ir/load_vector_element.h"
#include "src/tint/lang/core/ir/loop.h"
#include "src/tint/lang/core/ir/member_builtin_call.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/next_iteration.h"
#include "src/tint/lang/core/ir/override.h"
#include "src/tint/lang/core/ir/phony.h"
#include "src/tint/lang/core/ir/referenced_module_vars.h"
#include "src/tint/lang/core/ir/return.h"
#include "src/tint/lang/core/ir/store.h"
#include "src/tint/lang/core/ir/store_vector_element.h"
#include "src/tint/lang/core/ir/switch.h"
#include "src/tint/lang/core/ir/swizzle.h"
#include "src/tint/lang/core/ir/unary.h"
#include "src/tint/lang/core/ir/unreachable.h"
#include "src/tint/lang/core/ir/user_call.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/swizzle_view.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/result.h"
#include "src/tint/utils/rtti/castable.h"
#include "src/tint/utils/text/styled_text.h"

namespace tint::core::ir::validator {

/// How an attribute is being used, a tuple of the shader stage and IO direction
enum class IOAttributeUsage : uint8_t {
    kComputeInputUsage,
    kComputeOutputUsage,
    kComputeResourceUsage,
    kFragmentInputUsage,
    kFragmentOutputUsage,
    kFragmentResourceUsage,
    kVertexInputUsage,
    kVertexOutputUsage,
    kVertexResourceUsage,
    kUndefinedUsage,
};
std::string ToString(IOAttributeUsage value);

enum class IODirection : uint8_t {
    kInput,
    kOutput,
    kResource,
};
std::string_view ToString(IODirection value);

/// Annotations that can be associated with a value that are used for shader IO,
/// e.g. binding_points, @location, being in workgroup address space, etc.
/// These are a subset of IOAttributes.
enum class IOAnnotation : uint8_t {
    kBindingPoint,
    kLocation,
    kBuiltin,
    kWorkgroup,
    kColor,
};
std::string ToString(IOAnnotation value);

enum class ShaderIOKind : uint8_t {
    kInputParam,
    kResultValue,
    kModuleScopeVar,
};
std::string ToString(ShaderIOKind value);

/// State for validating IO attributes that needs to shared across impl invocations within the same
/// entry point.
struct IOAttributeContext {
    Hashmap<BuiltinValue, uint32_t, 4> input_builtins;
    Hashmap<BuiltinValue, uint32_t, 4> output_builtins;
};

/// State for validating blend_src attributes shared across multiple passes within the same entry
/// point.
struct BlendSrcContext {
    Function::PipelineStage stage{};
    Hashmap<uint32_t, const Value*, 4> locations;
    Hashset<uint32_t, 2> blend_srcs;
    const core::type::Type* blend_src_type = nullptr;
    IODirection dir{};
};

using SupportedStages = tint::EnumSet<Function::PipelineStage>;

/// The core IR validator.
class Validator {
  public:
    /// IOAttributeChecker is the interface used to check that a usage of an IO attribute
    /// meets the spec rules for a given context.
    struct IOAttributeChecker {
        /// What kinda of IO attribute is being checked
        IOAttributeKind kind;

        /// What combination of stage and IO direction is this attribute legal for.
        EnumSet<IOAttributeUsage> valid_usages;

        /// What type of shader IO values is this attribute legal for.
        EnumSet<ShaderIOKind> valid_io_kinds;

        /// Implements the validation logic for a specific attribute.
        using CheckFn = Result<SuccessType, std::string>(const core::type::Type* ty,
                                                         const IOAttributes& attr,
                                                         const Properties& prop,
                                                         IOAttributeUsage usage);

        /// The validation function.
        CheckFn* const check;

        /// Implements logic for checking if the given type is valid or not. Is not a data entry
        /// (i.e. a type or set of types), because types are part of the IR module and created at
        /// runtime.
        using TypeCheckFn = bool(const core::type::Type* type, const Properties& props);

        /// @see #TypeCheckFn
        TypeCheckFn* const type_check;

        /// Message for logging if the type check fails. Cannot be easily generated at runtime,
        /// because the type check is a function, not just a data entry.
        const char* type_error;
    };

    /// Create a core validator
    /// @param mod the module to be validated
    /// @param source the source of the program, WGSL or IR
    Validator(Module& mod, ErrorSource error_source);

    /// Destructor
    ~Validator() = default;

    /// Runs the validator over the module provided during construction
    /// @returns success or failure

    Result<SuccessType> Run();

  private:
    struct UseInfo {
        Usage use;
        /// Variable/buffer size
        uint32_t storage_size{};
        /// Accumulated offset to the pointer
        uint32_t offset{};
        /// Pointed to size
        uint32_t pointer_size{};
    };

    // Returns true if we're validating in the context of WGSL. The other option is we're validating
    // as IR. The primary difference is how const-eval checks are run as the semantics are
    // different.
    bool IsWGSLValidation() const;
    // Returns true if we're validating in the context of IR.
    bool IsIRValidation() const;

    /// Helper for walking a type that maybe a struct, calling an impl function for the type and
    /// each of its members.
    /// @param ctx a context object to pass to the implementation function
    /// @param type the type to walk
    /// @param attr the attributes for @p type
    /// @param impl a function called for each type with the signature
    ///             `void(const core::type::Type*, const IOAttributes&, CTX&)`
    template <typename CTX, typename IMPL>
    void WalkTypeAndMembers(CTX& ctx,
                            const core::type::Type* type,
                            const IOAttributes& attr,
                            IMPL&& impl) {
        impl(ctx, type, attr);
        tint::Switch(
            type, [&](const core::type::Struct* s) { WalkStructMembers(ctx, s, impl); },
            [&](const core::type::Array* a) { WalkArrayElements(ctx, a, impl); });
    }

    /// Helper that walks the members of a struct, called from WalkTypeAndMembers and its helpers
    /// @param ctx a context object to pass to the impl function
    /// @param str the struct to walk the members of
    /// @param impl an impl function to be run, see WalkTypeAndMembers for details
    template <typename CTX, typename IMPL>
    void WalkStructMembers(CTX& ctx, const core::type::Struct* str, IMPL&& impl) {
        for (auto* member : str->Members()) {
            WalkTypeAndMembers(ctx, member->Type(), member->Attributes(), impl);
        }
    }

    /// Helper that walks an array's element type, called from WalkTypeAndMembers and its helpers
    /// @param ctx a context object to pass to the impl function
    /// @param arr the array to walk the element type of
    /// @param impl an impl function to be run, see WalkTypeAndMembers for details
    template <typename CTX, typename IMPL>
    void WalkArrayElements(CTX& ctx, const core::type::Array* arr, IMPL&& impl) {
        WalkTypeAndMembers(ctx, arr->ElemType(), IOAttributes{}, impl);
    }

    /// Runs validation to confirm the structural soundness of the module.
    /// Also runs any validation that is not dependent on the entire module being
    /// sound and sets up data structures for later checks.
    void RunStructuralSoundnessChecks();

    /// Checks that there is no direct or indirect recursion.
    /// Depends on CheckStructuralSoundness() having previously been run.
    void CheckForRecursion();

    /// Checks that there are no orphaned instructions
    /// Depends on CheckStructuralSoundness() having previously been run
    void CheckForOrphanedInstructions();

    /// Checks that entry points do not use instructions that are not supported by their stage.
    /// Depends on CheckStructuralSoundness() having previously been run
    void CheckStageRestrictedInstructions();

    ir::Disassembler& Disassemble();

    Source SourceOf(const Function* func);
    Source SourceOf(const FunctionParam* param);
    Source SourceOf(const Instruction* inst);
    Source SourceOf(const Instruction* inst, size_t idx);

    diag::Diagnostic& AddError(const Instruction* inst);
    diag::Diagnostic& AddError(const InstructionResult* inst);
    diag::Diagnostic& AddError(const Instruction* inst, size_t idx);
    diag::Diagnostic& AddError(const Block* blk);
    diag::Diagnostic& AddError(const BlockParam* param);
    diag::Diagnostic& AddError(const Function* func);
    diag::Diagnostic& AddError(const FunctionParam* param);
    diag::Diagnostic& AddError(const Value* param);
    diag::Diagnostic& AddError(Source src);

    diag::Diagnostic& AddResultError(const Instruction* inst, size_t idx);

    diag::Diagnostic& AddNote(const Instruction* inst);
    diag::Diagnostic& AddNote(const Function* func);
    diag::Diagnostic& AddNote(const Block* blk);
    diag::Diagnostic& AddNote(Source src = {});
    diag::Diagnostic& AddNote(const Instruction* inst, size_t idx);

    diag::Diagnostic& AddOperandNote(const Instruction* inst, size_t idx);
    diag::Diagnostic& AddResultNote(const Instruction* inst, size_t idx);

    void AddDeclarationNote(const Block* block);
    void AddDeclarationNote(const BlockParam* param);
    void AddDeclarationNote(const Function* fn);
    void AddDeclarationNote(const FunctionParam* param);
    void AddDeclarationNote(const Instruction* inst);
    void AddDeclarationNote(const InstructionResult* res);
    void AddDeclarationNote(const Value* res);

    StyledText NameOf(const core::type::Type* ty);
    StyledText NameOf(const Value* v);
    StyledText NameOf(const Instruction* inst);
    StyledText NameOf(const Block* block);

    bool CheckResult(const Instruction* inst, size_t idx);
    bool CheckResults(const ir::Instruction* inst, std::optional<size_t> count = {});
    bool CheckResultsAndOperandRange(const ir::Instruction* inst,
                                     size_t num_results,
                                     size_t min_operands,
                                     std::optional<size_t> max_operands = {});
    bool CheckResultsAndOperands(const ir::Instruction* inst,
                                 size_t num_results,
                                 size_t num_operands);
    bool CheckOperand(const Instruction* inst, size_t idx);
    bool CheckOperands(const ir::Instruction* inst,
                       size_t min_count,
                       std::optional<size_t> max_count);
    bool CheckOperands(const ir::Instruction* inst, std::optional<size_t> count = {});

    /// NOTE: Expects to be called on a 'root' type, i.e. the type of a variable declaration or a
    ///       function param, not in the middle a walk of elements of a composite.
    void CheckType(const core::type::Type* root, std::function<diag::Diagnostic&()> diag);
    bool CheckStruct(const core::type::Struct* str, std::function<diag::Diagnostic&()>& diag);
    bool CheckRef(const core::type::Reference* ref,
                  std::function<diag::Diagnostic&()>& diag,
                  const core::type::Type* root);
    bool CheckPtr(const core::type::Pointer* ptr, std::function<diag::Diagnostic&()>& diag);
    bool CheckSwizzleView(const core::type::SwizzleView* sv,
                          std::function<diag::Diagnostic&()>& diag);
    bool CheckArray(const core::type::Array* arr, std::function<diag::Diagnostic&()>& diag);
    bool CheckVector(const core::type::Vector* vec, std::function<diag::Diagnostic&()>& diag);
    bool CheckMatrix(const core::type::Matrix* mat, std::function<diag::Diagnostic&()>& diag);
    bool CheckAtomic(const core::type::Atomic* atom, std::function<diag::Diagnostic&()>& diag);
    bool CheckSampledTexture(const core::type::SampledTexture* s,
                             std::function<diag::Diagnostic&()>& diag);
    bool CheckMultisampledTexture(const core::type::MultisampledTexture* ms,
                                  std::function<diag::Diagnostic&()>& diag);
    bool CheckStorageTexture(const core::type::StorageTexture* storage,
                             std::function<diag::Diagnostic&()>& diag);
    bool CheckInputAttachment(const core::type::InputAttachment* ia,
                              std::function<diag::Diagnostic&()>& diag);
    bool CheckSubgroupMatrix(const core::type::SubgroupMatrix* m,
                             std::function<diag::Diagnostic&()>& diag,
                             core::AddressSpace addrspace);
    bool CheckBindingArray(const core::type::BindingArray* ba,
                           std::function<diag::Diagnostic&()>& diag,
                           core::AddressSpace addrspace);
    bool CheckBuffer(const core::type::Buffer* buf, std::function<diag::Diagnostic&()>& diag);

    bool Check8BitInteger(std::function<diag::Diagnostic&()>& diag, const core::type::Type* parent);
    bool Check16BitInteger(std::function<diag::Diagnostic&()>& diag);
    bool Check64BitInteger(std::function<diag::Diagnostic&()>& diag);
    bool Check16BitFloat(std::function<diag::Diagnostic&()>& diag);

    /// NOTE: Expects to be called by CheckType, i.e. on a 'root' type, not in the middle a walk of
    ///       elements of a composite..
    bool CheckNestDepth(const core::type::Type* type, std::function<diag::Diagnostic&()> diag);
    const core::type::Type* GetVectorPtrElementType(const Instruction* inst, size_t idx);
    bool CanLoad(const core::type::Type* ty);

    void CheckRootBlock(const Block* blk);
    void CheckOnlyUsedInRootBlock(const Instruction* inst);

    void CheckFunction(const Function* func);
    bool CheckFunctionParam(const Function* func,
                            const FunctionParam* param,
                            Hashset<const FunctionParam*, 4>& param_set);
    void CheckEntryPoint(const Function* func);
    void CheckWorkgroupSize(const Function* func);
    void CheckSubgroupSize(const Function* func);

    void ValidateIOAttributes(const Function* func);
    void ValidateIOAttributesImpl(IOAttributeContext& ctx,
                                  const Value* msg_anchor,
                                  const core::type::Type* ty,
                                  const IOAttributes& attr,
                                  Function::PipelineStage stage,
                                  IODirection dir,
                                  ShaderIOKind io_kind);
    void ValidateShaderIOAnnotations(const Value* msg_anchor,
                                     const core::type::Type* ty,
                                     const std::optional<BindingPoint>& binding_point,
                                     const IOAttributes& attr,
                                     ShaderIOKind kind);
    bool CheckStructMemberAttributes(const core::type::StructMember* member,
                                     std::function<diag::Diagnostic&()> make_diag);

    void CheckNotBool(const Value* msg_anchor, const core::type::Type* ty, const std::string& err);
    void CheckPositionPresentForVertexOutput(const Function* ep);
    void CheckFrontFacingIfBool(const Value* msg_anchor,
                                const IOAttributes& attr,
                                const core::type::Type* ty,
                                const std::string& err);
    void CheckBlendSrc(BlendSrcContext& ctx,
                       const Value* target,
                       const core::type::Type* ty,
                       const IOAttributes& attr);
    void CheckBlendSrcImpl(BlendSrcContext& ctx,
                           const Value* target,
                           const core::type::Type* ty,
                           const IOAttributes& attr);
    void CheckLocation(Hashmap<uint32_t, const Value*, 4>& locations,
                       const Value* target,
                       const IOAttributes& attr,
                       Function::PipelineStage stage,
                       const core::type::Type* type,
                       IODirection dir);
    void CheckInterpolation(const Value* anchor,
                            const core::type::Type* ty,
                            const IOAttributes& attr,
                            Function::PipelineStage stage,
                            IODirection dir);
    void CheckBindingPoint(const Value* anchor,
                           const core::type::Type* ty,
                           const IOAttributes& attr,
                           const ShaderIOKind& io_kind);

    void CheckInstruction(const Instruction* inst);
    void CheckOverride(const Override* o);
    void CheckVar(const Var* var);
    void CheckLet(const Let* l);
    void CheckCall(const Call* call);
    void CheckBuiltinCall(const BuiltinCall* call);
    void CheckMemberBuiltinCall(const MemberBuiltinCall* call);
    void CheckConstruct(const Construct* construct);
    void CheckConvert(const Convert* convert);
    void CheckDiscard(const Discard* discard);
    void CheckUserCall(const UserCall* call);
    void CheckAccess(const Access* a);
    void CheckBinary(const Binary* b);
    void CheckUnary(const Unary* u);
    void CheckIf(const If* if_);
    void CheckLoop(const Loop* l);
    void CheckLoopContinuing(const Loop* loop);
    void CheckSwitch(const Switch* s);
    void CheckSwizzle(const Swizzle* s);
    void CheckTerminator(const Terminator* b);
    void CheckBreakIf(const BreakIf* b);
    void CheckContinue(const Continue* c);
    void CheckExit(const Exit* e);
    void CheckNextIteration(const NextIteration* n);
    void CheckExitIf(const ExitIf* e);
    void CheckReturn(const Return* r);
    void CheckUnreachable(const Unreachable* u);
    void CheckExitSwitch(const ExitSwitch* s);
    void CheckExitLoop(const ExitLoop* l);
    void CheckLoad(const Load* l);
    void CheckStore(const Store* s);
    void CheckLoadVectorElement(const LoadVectorElement* l);
    void CheckStoreVectorElement(const StoreVectorElement* s);
    void CheckPhony(const Phony* p);

    void CheckCoreBinaryCall(const CoreBinary* call);
    void CheckBinaryDivModCall(const CoreBinary* call);
    void CheckBinaryShiftCall(const CoreBinary* call);

    void CheckCoreBuiltinCall(const CoreBuiltinCall* call,
                              const core::intrinsic::Overload& overload);
    void CheckSubgroupCall(const CoreBuiltinCall* call);
    void CheckExtractBitsCall(const CoreBuiltinCall* call);
    void CheckInsertBitsCall(const CoreBuiltinCall* call);
    void CheckLdexpCall(const CoreBuiltinCall* call);
    void CheckQuantizeToF16(const CoreBuiltinCall* call);
    void CheckPack2x16float(const CoreBuiltinCall* call);
    void CheckClampCall(const CoreBuiltinCall* call);
    void CheckSmoothstepCall(const CoreBuiltinCall* call);
    void CheckSubgroupMatrixOpOffset(const CoreBuiltinCall* call);

    void CheckBuffersAndMatrices(const Var* var);
    bool CheckBufferView(const CoreBuiltinCall* call, const Var* var, uint32_t buffer_size);
    bool CheckSubgroupMatrixMemory(const CoreBuiltinCall* call,
                                   const Var* var,
                                   const UseInfo& info);

    // Validates the alignment of the given instruction
    /// @param inst the instruction to validate
    void CheckAlignment(const Instruction* inst);

    void CheckControlsAllowingIf(const Exit* exit, const Instruction* control);

    void CheckOperandsMatchTarget(const Instruction* source_inst,
                                  size_t source_operand_offset,
                                  size_t source_operand_count,
                                  const MultiInBlock* target,
                                  VectorRef<const Value*> target_values);

    void CheckOperandsMatchTarget(const Instruction* source_inst,
                                  size_t source_operand_offset,
                                  size_t source_operand_count,
                                  const ControlInstruction* target,
                                  VectorRef<const Value*> target_values);

    void ProcessTasks();
    void QueueBlock(const Block* blk);
    void QueueInstructions(const Instruction* inst);
    void QueueTasks(std::function<void()> begin,
                    std::function<void()> mid,
                    std::function<void()> end);
    std::function<void()> QueueNestedTasks(std::function<void()> begin,
                                           std::function<void()> mid,
                                           std::function<void()> end);

    /// Must be paired with a call to EndBlock().
    void BeginBlock(const Block* blk);
    void EndBlock();

    /// Calculates the total number elements contained in a type, i.e. the number of values required
    /// for an initializer.
    uint64_t ElementsCount(const core::type::Type* ty);

    const ir::Function* ContainingFunction(const ir::Instruction* inst);
    Hashset<const ir::Function*, 4> ContainingEndPoints(const ir::Function* f);

    std::function<void()> PushControlStack(const ControlInstruction* ctrl);
    std::function<void()> PopControlStack();
    std::function<void()> BeginBlockTask(const Block* blk);
    std::function<void()> EndBlockTask(const Block* blk);

    Vector<const IOAttributeChecker*, 4> IOAttributeCheckersFor(const IOAttributes& attr,
                                                                bool skip_builtin);

    /// ScopeStack holds a stack of values that are currently in scope
    struct ScopeStack {
        void Push() { stack_.Push({}); }
        void Pop() { stack_.Pop(); }
        void Add(const Value* value) { stack_.Back().Add(value); }
        bool Contains(const Value* value) {
            return stack_.Any([&](auto& v) { return v.Contains(value); });
        }
        bool IsEmpty() const { return stack_.IsEmpty(); }

      private:
        Vector<Hashset<const Value*, 8>, 4> stack_;
    };

    Module& ir_;
    ErrorSource error_source_ = ErrorSource::kIr;
    diag::List diag_;

    constant::Eval const_eval_;

    std::optional<ir::Disassembler> disassembler_;  // Use Disassemble()

    SymbolTable symbols_ = SymbolTable::Wrap(ir_.symbols);
    core::type::Manager type_mgr_ = core::type::Manager::Wrap(ir_.Types());
    ReferencedModuleVars<const Module> referenced_module_vars_;

    Vector<const ControlInstruction*, 8> control_stack_;
    Vector<const Block*, 8> block_stack_;
    ScopeStack scope_stack_;

    // The task processing queue is required due to how WGSL is translated into IR. In IR, an `else
    // if` turns into a nested control instruction inside the parent `false` block. This means the
    // nesting depth of an IR module can be a lot larger then the nesting depth of a WGSL program.
    // So, while we've validated the maximum nesting depth in WGSL, that does not translate to the
    // maximum depth in IR.  So, we use a task list. This lets us do a controlled depth-first
    // iteration of the blocks in the IR along with pushing information for scoping and control
    // structures at the right times.
    Vector<std::function<void()>, 16> tasks_;

    Hashset<const Function*, 4> all_functions_;
    Hashset<std::string, 4> entry_point_names_;
    Hashset<const Instruction*, 4> visited_instructions_;
    Hashset<const core::type::Type*, 16> validated_types_{};
    Hashset<OverrideId, 8> seen_override_ids_;

    Hashmap<const ir::Block*, const ir::Function*, 64> block_to_function_{};
    Hashmap<const ir::Function*, Hashset<const ir::UserCall*, 4>, 4> user_func_calls_;
    Hashmap<const ir::Instruction*, SupportedStages, 4> stage_restricted_instructions_;
    Hashmap<const core::type::Type*, uint64_t, 16> max_nest_depth_{};
    Hashmap<const core::type::Type*, uint64_t, 16> elements_counts_;
    Hashmap<const Loop*, const Continue*, 4> first_continues_;

    uint64_t total_private_bytes_ = 0;
};

}  // namespace tint::core::ir::validator

#endif  // SRC_TINT_LANG_CORE_IR_VALIDATOR_VALIDATOR_H_
