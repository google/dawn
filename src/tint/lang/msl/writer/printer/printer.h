// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_MSL_WRITER_PRINTER_PRINTER_H_
#define SRC_TINT_LANG_MSL_WRITER_PRINTER_PRINTER_H_

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/generator/text_generator.h"
#include "src/tint/utils/text/string_stream.h"

// Forward declarations
namespace tint::ir {
class Binary;
class ExitIf;
class If;
class Let;
class Load;
class Return;
class Unreachable;
class Var;
}  // namespace tint::ir

namespace tint::msl::writer {

/// Implementation class for the MSL generator
class Printer : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param module the Tint IR module to generate
    explicit Printer(ir::Module* module);
    ~Printer() override;

    /// @returns true on successful generation; false otherwise
    bool Generate();

    /// @copydoc tint::TextGenerator::Result
    std::string Result() const override;

    /// Emit the function
    /// @param func the function to emit
    void EmitFunction(ir::Function* func);

    /// Emit a block
    /// @param block the block to emit
    void EmitBlock(ir::Block* block);
    /// Emit the instructions in a block
    /// @param block the block with the instructions to emit
    void EmitBlockInstructions(ir::Block* block);

    /// Emit an if instruction
    /// @param if_ the if instruction
    void EmitIf(ir::If* if_);
    /// Emit an exit-if instruction
    /// @param e the exit-if instruction
    void EmitExitIf(ir::ExitIf* e);

    /// Emit a let instruction
    /// @param l the let instruction
    void EmitLet(ir::Let* l);
    /// Emit a var instruction
    /// @param v the var instruction
    void EmitVar(ir::Var* v);
    /// Emit a load instruction
    /// @param l the load instruction
    void EmitLoad(ir::Load* l);

    /// Emit a return instruction
    /// @param r the return instruction
    void EmitReturn(ir::Return* r);
    /// Emit an unreachable instruction
    void EmitUnreachable();

    /// Emit a binary instruction
    /// @param b the binary instruction
    void EmitBinary(ir::Binary* b);

    /// Emit a type
    /// @param out the stream to emit too
    /// @param ty the type to emit
    void EmitType(StringStream& out, const type::Type* ty);

    /// Handles generating an array declaration
    /// @param out the output stream
    /// @param arr the array to emit
    void EmitArrayType(StringStream& out, const type::Array* arr);
    /// Handles generating an atomic declaration
    /// @param out the output stream
    /// @param atomic the atomic to emit
    void EmitAtomicType(StringStream& out, const type::Atomic* atomic);
    /// Handles generating a pointer declaration
    /// @param out the output stream
    /// @param ptr the pointer to emit
    void EmitPointerType(StringStream& out, const type::Pointer* ptr);
    /// Handles generating a vector declaration
    /// @param out the output stream
    /// @param vec the vector to emit
    void EmitVectorType(StringStream& out, const type::Vector* vec);
    /// Handles generating a matrix declaration
    /// @param out the output stream
    /// @param mat the matrix to emit
    void EmitMatrixType(StringStream& out, const type::Matrix* mat);
    /// Handles generating a texture declaration
    /// @param out the output stream
    /// @param tex the texture to emit
    void EmitTextureType(StringStream& out, const type::Texture* tex);
    /// Handles generating a struct declaration. If the structure has already been emitted, then
    /// this function will simply return without emitting anything.
    /// @param str the struct to generate
    void EmitStructType(const type::Struct* str);

    /// Handles generating a address space
    /// @param out the output of the type stream
    /// @param sc the address space to generate
    void EmitAddressSpace(StringStream& out, builtin::AddressSpace sc);

    /// Handles ir::Constant values
    /// @param out the stream to write the constant too
    /// @param c the constant to emit
    void EmitConstant(StringStream& out, ir::Constant* c);
    /// Handles constant::Value values
    /// @param out the stream to write the constant too
    /// @param c the constant to emit
    void EmitConstant(StringStream& out, const constant::Value* c);

    /// Emits the zero value for the given type
    /// @param out the stream to emit too
    /// @param ty the type
    void EmitZeroValue(StringStream& out, const type::Type* ty);

    /// @returns the name of the templated `tint_array` helper type, generating it if needed
    const std::string& ArrayTemplateName();

    /// Unique name of the tint_array<T, N> template.
    /// Non-empty only if the template has been generated.
    std::string array_template_name_;

  private:
    /// @param s the structure
    /// @returns the name of the structure, taking special care of builtin structures that start
    /// with double underscores. If the structure is a builtin, then the returned name will be a
    /// unique name without the leading underscores.
    std::string StructName(const type::Struct* s);

    /// @return a new, unique identifier with the given prefix.
    /// @param prefix optional prefix to apply to the generated identifier. If empty "tint_symbol"
    /// will be used.
    std::string UniqueIdentifier(const std::string& prefix = "");

    /// Map of builtin structure to unique generated name
    std::unordered_map<const type::Struct*, std::string> builtin_struct_names_;

    ir::Module* const ir_;

    /// The buffer holding preamble text
    TextBuffer preamble_buffer_;

    /// Unique name of the 'TINT_INVARIANT' preprocessor define.
    /// Non-empty only if an invariant attribute has been generated.
    std::string invariant_define_name_;

    std::unordered_set<const type::Struct*> emitted_structs_;

    /// The current function being emitted
    ir::Function* current_function_ = nullptr;
    /// The current block being emitted
    ir::Block* current_block_ = nullptr;

    /// The representation for an IR pointer type
    enum class PtrKind {
        kPtr,  // IR pointer is represented in a pointer
        kRef,  // IR pointer is represented in a reference
    };

    /// The structure for a value held by a 'let', 'var' or parameter.
    struct VariableValue {
        Symbol name;  // Name of the variable
        PtrKind ptr_kind = PtrKind::kRef;
    };

    /// The structure for an inlined value
    struct InlinedValue {
        std::string expr;
        PtrKind ptr_kind = PtrKind::kRef;
    };

    /// Empty struct used as a sentinel value to indicate that an string expression has been
    /// consumed by its single place of usage. Attempting to use this value a second time should
    /// result in an ICE.
    struct ConsumedValue {};

    using ValueBinding = std::variant<VariableValue, InlinedValue, ConsumedValue>;

    /// IR values to their representation
    Hashmap<ir::Value*, ValueBinding, 32> bindings_;

    /// Values that can be inlined.
    Hashset<ir::Value*, 64> can_inline_;

    /// Returns the expression for the given value
    /// @param value the value to lookup
    /// @param want_ptr_kind the pointer information for the return
    /// @returns the string expression
    std::string Expr(ir::Value* value, PtrKind want_ptr_kind = PtrKind::kRef);

    /// Returns the given expression converted to the given pointer kind
    /// @param in the input expression
    /// @param got the pointer kind we have
    /// @param want the pointer kind we want
    std::string ToPtrKind(const std::string& in, PtrKind got, PtrKind want);

    /// Associates an IR value with a result expression
    /// @param value the IR value
    /// @param expr the result expression
    /// @param ptr_kind defines how pointer values are represented by the expression
    void Bind(ir::Value* value, const std::string& expr, PtrKind ptr_kind = PtrKind::kRef);

    /// Associates an IR value the 'var', 'let' or parameter of the given name
    /// @param value the IR value
    /// @param name the name for the value
    /// @param ptr_kind defines how pointer values are represented by @p expr.
    void Bind(ir::Value* value, Symbol name, PtrKind ptr_kind = PtrKind::kRef);

    /// Marks instructions in a block for inlineability
    /// @param block the block
    void MarkInlinable(ir::Block* block);
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_PRINTER_PRINTER_H_
