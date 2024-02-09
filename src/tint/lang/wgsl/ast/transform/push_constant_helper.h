// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_TINT_LANG_WGSL_AST_TRANSFORM_PUSH_CONSTANT_HELPER_H_
#define SRC_TINT_LANG_WGSL_AST_TRANSFORM_PUSH_CONSTANT_HELPER_H_

#include <map>

#include "src/tint/utils/symbol/symbol.h"

namespace tint::program {
class CloneContext;
}
namespace tint::ast {
struct Type;
class Struct;
class StructMember;
class Variable;
}  // namespace tint::ast
namespace tint::ast::transform {

/// A helper that manages the finding, reading, and modifying of push_constant blocks.
/// This is used by transforms that wish to add new data to the single push_constant block
/// which is allowed per entry point.
///
/// Usage:
/// 1) Instantiate the helper with a CloneContext in which nodes are to be created.
/// 2) Add new data members to the push_context block with InsertMember().
/// 3) Call Run() to insert the push_constant block into the AST and remove the old block.
/// 4) Ensure that the helper lives until the CloneContext's Clone() method is called.
class PushConstantHelper {
  public:
    /// Constructor
    /// The caller must ensure that the helper lives until the passed-in CloneContext's Clone()
    /// method is called.
    explicit PushConstantHelper(program::CloneContext& c);

    /// Destructor
    ~PushConstantHelper();

    /// Insert a new data member into the push_constant block. Members can be inserted in
    /// any order, and will be written to the struct in sorted order. Note that collisions
    /// between names are *not* currently detected.
    /// @param name the name of the new struct member
    /// @param type the type of the new struct member
    /// @param offset the offset in bytes of the new struct member
    void InsertMember(const char* name, ast::Type type, uint32_t offset);

    /// Create the new PushConstant struct. Change all references from the old struct to the
    /// new one.
    /// @returns the name of the push_constant global variable holding the new block.
    Symbol Run();

  private:
    /// A map from byte offset to struct member of all the push constant members.
    std::map<uint32_t, const tint::ast::StructMember*> member_map;

    /// The clone context to be used for node creation.
    program::CloneContext& ctx;

    /// The new PushConstant struct type.
    const ast::Struct* new_struct = nullptr;

    /// The old push_constant global variable, if any.
    const ast::Variable* push_constants_var = nullptr;
};

}  // namespace tint::ast::transform

#endif  // SRC_TINT_LANG_WGSL_AST_TRANSFORM_PUSH_CONSTANT_HELPER_H_
