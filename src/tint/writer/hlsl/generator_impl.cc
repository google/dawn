/// Copyright 2020 The Tint Authors.
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

#include "src/tint/writer/hlsl/generator_impl.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iomanip>
#include <set>
#include <utility>
#include <vector>

#include "src/tint/ast/call_statement.h"
#include "src/tint/ast/fallthrough_statement.h"
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/internal_attribute.h"
#include "src/tint/ast/interpolate_attribute.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/debug.h"
#include "src/tint/sem/array.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/depth_multisampled_texture.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/module.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/type_constructor.h"
#include "src/tint/sem/type_conversion.h"
#include "src/tint/sem/variable.h"
#include "src/tint/transform/add_empty_entry_point.h"
#include "src/tint/transform/array_length_from_uniform.h"
#include "src/tint/transform/builtin_polyfill.h"
#include "src/tint/transform/calculate_array_length.h"
#include "src/tint/transform/canonicalize_entry_point_io.h"
#include "src/tint/transform/decompose_memory_access.h"
#include "src/tint/transform/expand_compound_assignment.h"
#include "src/tint/transform/fold_trivial_single_use_lets.h"
#include "src/tint/transform/localize_struct_array_assignment.h"
#include "src/tint/transform/loop_to_for_loop.h"
#include "src/tint/transform/manager.h"
#include "src/tint/transform/num_workgroups_from_uniform.h"
#include "src/tint/transform/promote_initializers_to_const_var.h"
#include "src/tint/transform/promote_side_effects_to_decl.h"
#include "src/tint/transform/remove_continue_in_switch.h"
#include "src/tint/transform/remove_phonies.h"
#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/unshadow.h"
#include "src/tint/transform/unwind_discard_functions.h"
#include "src/tint/transform/zero_init_workgroup_memory.h"
#include "src/tint/utils/defer.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/writer/append_vector.h"
#include "src/tint/writer/float_to_string.h"
#include "src/tint/writer/generate_external_texture_bindings.h"

namespace tint::writer::hlsl {
namespace {

const char kTempNamePrefix[] = "tint_tmp";
const char kSpecConstantPrefix[] = "WGSL_SPEC_CONSTANT_";

const char* image_format_to_rwtexture_type(ast::TexelFormat image_format) {
  switch (image_format) {
    case ast::TexelFormat::kRgba8Unorm:
    case ast::TexelFormat::kRgba8Snorm:
    case ast::TexelFormat::kRgba16Float:
    case ast::TexelFormat::kR32Float:
    case ast::TexelFormat::kRg32Float:
    case ast::TexelFormat::kRgba32Float:
      return "float4";
    case ast::TexelFormat::kRgba8Uint:
    case ast::TexelFormat::kRgba16Uint:
    case ast::TexelFormat::kR32Uint:
    case ast::TexelFormat::kRg32Uint:
    case ast::TexelFormat::kRgba32Uint:
      return "uint4";
    case ast::TexelFormat::kRgba8Sint:
    case ast::TexelFormat::kRgba16Sint:
    case ast::TexelFormat::kR32Sint:
    case ast::TexelFormat::kRg32Sint:
    case ast::TexelFormat::kRgba32Sint:
      return "int4";
    default:
      return nullptr;
  }
}

// Helper for writing " : register(RX, spaceY)", where R is the register, X is
// the binding point binding value, and Y is the binding point group value.
struct RegisterAndSpace {
  RegisterAndSpace(char r, ast::VariableBindingPoint bp)
      : reg(r), binding_point(bp) {}

  const char reg;
  ast::VariableBindingPoint const binding_point;
};

std::ostream& operator<<(std::ostream& s, const RegisterAndSpace& rs) {
  s << " : register(" << rs.reg << rs.binding_point.binding->value << ", space"
    << rs.binding_point.group->value << ")";
  return s;
}

const char* LoopAttribute() {
  // Force loops not to be unrolled to work around FXC compilation issues when
  // it attempts and fails to unroll loops when it contains gradient operations.
  // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-while
  return "[loop] ";
}

}  // namespace

SanitizedResult::SanitizedResult() = default;
SanitizedResult::~SanitizedResult() = default;
SanitizedResult::SanitizedResult(SanitizedResult&&) = default;

SanitizedResult Sanitize(const Program* in, const Options& options) {
  transform::Manager manager;
  transform::DataMap data;

  {  // Builtin polyfills
    transform::BuiltinPolyfill::Builtins polyfills;
    // TODO(crbug.com/tint/1449): Some of these can map to HLSL's `firstbitlow`
    // and `firstbithigh`.
    polyfills.count_leading_zeros = true;
    polyfills.count_trailing_zeros = true;
    polyfills.extract_bits = transform::BuiltinPolyfill::Level::kFull;
    polyfills.first_leading_bit = true;
    polyfills.first_trailing_bit = true;
    polyfills.insert_bits = transform::BuiltinPolyfill::Level::kFull;
    data.Add<transform::BuiltinPolyfill::Config>(polyfills);
    manager.Add<transform::BuiltinPolyfill>();
  }

  // Build the config for the internal ArrayLengthFromUniform transform.
  auto& array_length_from_uniform = options.array_length_from_uniform;
  transform::ArrayLengthFromUniform::Config array_length_from_uniform_cfg(
      array_length_from_uniform.ubo_binding);
  array_length_from_uniform_cfg.bindpoint_to_size_index =
      array_length_from_uniform.bindpoint_to_size_index;

  if (options.generate_external_texture_bindings) {
    auto new_bindings_map = GenerateExternalTextureBindings(in);
    data.Add<transform::MultiplanarExternalTexture::NewBindingPoints>(
        new_bindings_map);
  }
  manager.Add<transform::MultiplanarExternalTexture>();

  manager.Add<transform::Unshadow>();

  // LocalizeStructArrayAssignment must come after:
  // * SimplifyPointers, because it assumes assignment to arrays in structs are
  // done directly, not indirectly.
  // TODO(crbug.com/tint/1340): See if we can get rid of the duplicate
  // SimplifyPointers transform. Can't do it right now because
  // LocalizeStructArrayAssignment introduces pointers.
  manager.Add<transform::SimplifyPointers>();
  manager.Add<transform::LocalizeStructArrayAssignment>();

  // Attempt to convert `loop`s into for-loops. This is to try and massage the
  // output into something that will not cause FXC to choke or misbehave.
  manager.Add<transform::FoldTrivialSingleUseLets>();
  manager.Add<transform::LoopToForLoop>();

  if (!options.disable_workgroup_init) {
    // ZeroInitWorkgroupMemory must come before CanonicalizeEntryPointIO as
    // ZeroInitWorkgroupMemory may inject new builtin parameters.
    manager.Add<transform::ZeroInitWorkgroupMemory>();
  }
  manager.Add<transform::CanonicalizeEntryPointIO>();
  // NumWorkgroupsFromUniform must come after CanonicalizeEntryPointIO, as it
  // assumes that num_workgroups builtins only appear as struct members and are
  // only accessed directly via member accessors.
  manager.Add<transform::NumWorkgroupsFromUniform>();
  manager.Add<transform::ExpandCompoundAssignment>();
  manager.Add<transform::PromoteSideEffectsToDecl>();
  manager.Add<transform::UnwindDiscardFunctions>();
  manager.Add<transform::SimplifyPointers>();
  manager.Add<transform::RemovePhonies>();
  // ArrayLengthFromUniform must come after InlinePointerLets and Simplify, as
  // it assumes that the form of the array length argument is &var.array.
  manager.Add<transform::ArrayLengthFromUniform>();
  data.Add<transform::ArrayLengthFromUniform::Config>(
      std::move(array_length_from_uniform_cfg));
  // DecomposeMemoryAccess must come after:
  // * InlinePointerLets, as we cannot take the address of calls to
  //   DecomposeMemoryAccess::Intrinsic.
  // * Simplify, as we need to fold away the address-of and dereferences of
  // `*(&(intrinsic_load()))` expressions.
  // * RemovePhonies, as phonies can be assigned a pointer to a
  //   non-constructible buffer, or dynamic array, which DMA cannot cope with.
  manager.Add<transform::DecomposeMemoryAccess>();
  // CalculateArrayLength must come after DecomposeMemoryAccess, as
  // DecomposeMemoryAccess special-cases the arrayLength() intrinsic, which
  // will be transformed by CalculateArrayLength
  manager.Add<transform::CalculateArrayLength>();
  manager.Add<transform::PromoteInitializersToConstVar>();

  manager.Add<transform::RemoveContinueInSwitch>();

  manager.Add<transform::AddEmptyEntryPoint>();

  data.Add<transform::CanonicalizeEntryPointIO::Config>(
      transform::CanonicalizeEntryPointIO::ShaderStyle::kHlsl);
  data.Add<transform::NumWorkgroupsFromUniform::Config>(
      options.root_constant_binding_point);

  auto out = manager.Run(in, data);

  SanitizedResult result;
  result.program = std::move(out.program);
  if (auto* res = out.data.Get<transform::ArrayLengthFromUniform::Result>()) {
    result.used_array_length_from_uniform_indices =
        std::move(res->used_size_indices);
  }
  return result;
}

GeneratorImpl::GeneratorImpl(const Program* program) : TextGenerator(program) {}

GeneratorImpl::~GeneratorImpl() = default;

bool GeneratorImpl::Generate() {
  const TypeInfo* last_kind = nullptr;
  size_t last_padding_line = 0;

  auto* mod = builder_.Sem().Module();
  for (auto* decl : mod->DependencyOrderedDeclarations()) {
    if (decl->Is<ast::Alias>()) {
      continue;  // Ignore aliases.
    }

    // Emit a new line between declarations if the type of declaration has
    // changed, or we're about to emit a function
    auto* kind = &decl->TypeInfo();
    if (current_buffer_->lines.size() != last_padding_line) {
      if (last_kind && (last_kind != kind || decl->Is<ast::Function>())) {
        line();
        last_padding_line = current_buffer_->lines.size();
      }
    }
    last_kind = kind;

    bool ok = Switch(
        decl,
        [&](const ast::Variable* global) {  //
          return EmitGlobalVariable(global);
        },
        [&](const ast::Struct* str) {
          auto* ty = builder_.Sem().Get(str);
          auto storage_class_uses = ty->StorageClassUsage();
          if (storage_class_uses.size() !=
              (storage_class_uses.count(ast::StorageClass::kStorage) +
               storage_class_uses.count(ast::StorageClass::kUniform))) {
            // The structure is used as something other than a storage buffer or
            // uniform buffer, so it needs to be emitted.
            // Storage buffer are read and written to via a ByteAddressBuffer
            // instead of true structure.
            // Structures used as uniform buffer are read from an array of
            // vectors instead of true structure.
            return EmitStructType(current_buffer_, ty);
          }
          return true;
        },
        [&](const ast::Function* func) {
          if (func->IsEntryPoint()) {
            return EmitEntryPointFunction(func);
          }
          return EmitFunction(func);
        },
        [&](const ast::Enable*) {
          // Currently we don't have to do anything for using a extension in
          // HLSL
          return true;
        },
        [&](Default) {
          TINT_ICE(Writer, diagnostics_)
              << "unhandled module-scope declaration: "
              << decl->TypeInfo().name;
          return false;
        });

    if (!ok) {
      return false;
    }
  }

  if (!helpers_.lines.empty()) {
    current_buffer_->Insert(helpers_, 0, 0);
  }

  return true;
}

bool GeneratorImpl::EmitDynamicVectorAssignment(
    const ast::AssignmentStatement* stmt,
    const sem::Vector* vec) {
  auto name =
      utils::GetOrCreate(dynamic_vector_write_, vec, [&]() -> std::string {
        std::string fn;
        {
          std::ostringstream ss;
          if (!EmitType(ss, vec, tint::ast::StorageClass::kInvalid,
                        ast::Access::kUndefined, "")) {
            return "";
          }
          fn = UniqueIdentifier("set_" + ss.str());
        }
        {
          auto out = line(&helpers_);
          out << "void " << fn << "(inout ";
          if (!EmitTypeAndName(out, vec, ast::StorageClass::kInvalid,
                               ast::Access::kUndefined, "vec")) {
            return "";
          }
          out << ", int idx, ";
          if (!EmitTypeAndName(out, vec->type(), ast::StorageClass::kInvalid,
                               ast::Access::kUndefined, "val")) {
            return "";
          }
          out << ") {";
        }
        {
          ScopedIndent si(&helpers_);
          auto out = line(&helpers_);
          switch (vec->Width()) {
            case 2:
              out << "vec = (idx.xx == int2(0, 1)) ? val.xx : vec;";
              break;
            case 3:
              out << "vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;";
              break;
            case 4:
              out << "vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;";
              break;
            default:
              TINT_UNREACHABLE(Writer, builder_.Diagnostics())
                  << "invalid vector size " << vec->Width();
              break;
          }
        }
        line(&helpers_) << "}";
        line(&helpers_);
        return fn;
      });

  if (name.empty()) {
    return false;
  }

  auto* ast_access_expr = stmt->lhs->As<ast::IndexAccessorExpression>();

  auto out = line();
  out << name << "(";
  if (!EmitExpression(out, ast_access_expr->object)) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, ast_access_expr->index)) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, stmt->rhs)) {
    return false;
  }
  out << ");";

  return true;
}

bool GeneratorImpl::EmitDynamicMatrixVectorAssignment(
    const ast::AssignmentStatement* stmt,
    const sem::Matrix* mat) {
  auto name = utils::GetOrCreate(
      dynamic_matrix_vector_write_, mat, [&]() -> std::string {
        std::string fn;
        {
          std::ostringstream ss;
          if (!EmitType(ss, mat, tint::ast::StorageClass::kInvalid,
                        ast::Access::kUndefined, "")) {
            return "";
          }
          fn = UniqueIdentifier("set_vector_" + ss.str());
        }
        {
          auto out = line(&helpers_);
          out << "void " << fn << "(inout ";
          if (!EmitTypeAndName(out, mat, ast::StorageClass::kInvalid,
                               ast::Access::kUndefined, "mat")) {
            return "";
          }
          out << ", int col, ";
          if (!EmitTypeAndName(out, mat->ColumnType(),
                               ast::StorageClass::kInvalid,
                               ast::Access::kUndefined, "val")) {
            return "";
          }
          out << ") {";
        }
        {
          ScopedIndent si(&helpers_);
          line(&helpers_) << "switch (col) {";
          {
            ScopedIndent si2(&helpers_);
            for (uint32_t i = 0; i < mat->columns(); ++i) {
              line(&helpers_)
                  << "case " << i << ": mat[" << i << "] = val; break;";
            }
          }
          line(&helpers_) << "}";
        }
        line(&helpers_) << "}";
        line(&helpers_);
        return fn;
      });

  if (name.empty()) {
    return false;
  }

  auto* ast_access_expr = stmt->lhs->As<ast::IndexAccessorExpression>();

  auto out = line();
  out << name << "(";
  if (!EmitExpression(out, ast_access_expr->object)) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, ast_access_expr->index)) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, stmt->rhs)) {
    return false;
  }
  out << ");";

  return true;
}

bool GeneratorImpl::EmitDynamicMatrixScalarAssignment(
    const ast::AssignmentStatement* stmt,
    const sem::Matrix* mat) {
  auto* lhs_col_access = stmt->lhs->As<ast::IndexAccessorExpression>();
  auto* lhs_row_access =
      lhs_col_access->object->As<ast::IndexAccessorExpression>();

  auto name = utils::GetOrCreate(
      dynamic_matrix_scalar_write_, mat, [&]() -> std::string {
        std::string fn;
        {
          std::ostringstream ss;
          if (!EmitType(ss, mat, tint::ast::StorageClass::kInvalid,
                        ast::Access::kUndefined, "")) {
            return "";
          }
          fn = UniqueIdentifier("set_scalar_" + ss.str());
        }
        {
          auto out = line(&helpers_);
          out << "void " << fn << "(inout ";
          if (!EmitTypeAndName(out, mat, ast::StorageClass::kInvalid,
                               ast::Access::kUndefined, "mat")) {
            return "";
          }
          out << ", int col, int row, ";
          if (!EmitTypeAndName(out, mat->type(), ast::StorageClass::kInvalid,
                               ast::Access::kUndefined, "val")) {
            return "";
          }
          out << ") {";
        }
        {
          ScopedIndent si(&helpers_);
          line(&helpers_) << "switch (col) {";
          {
            ScopedIndent si2(&helpers_);
            auto* vec =
                TypeOf(lhs_row_access->object)->UnwrapRef()->As<sem::Vector>();
            for (uint32_t i = 0; i < mat->columns(); ++i) {
              line(&helpers_) << "case " << i << ":";
              {
                auto vec_name = "mat[" + std::to_string(i) + "]";
                ScopedIndent si3(&helpers_);
                {
                  auto out = line(&helpers_);
                  switch (mat->rows()) {
                    case 2:
                      out << vec_name
                          << " = (row.xx == int2(0, 1)) ? val.xx : " << vec_name
                          << ";";
                      break;
                    case 3:
                      out << vec_name
                          << " = (row.xxx == int3(0, 1, 2)) ? val.xxx : "
                          << vec_name << ";";
                      break;
                    case 4:
                      out << vec_name
                          << " = (row.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : "
                          << vec_name << ";";
                      break;
                    default:
                      TINT_UNREACHABLE(Writer, builder_.Diagnostics())
                          << "invalid vector size " << vec->Width();
                      break;
                  }
                }
                line(&helpers_) << "break;";
              }
            }
          }
          line(&helpers_) << "}";
        }
        line(&helpers_) << "}";
        line(&helpers_);
        return fn;
      });

  if (name.empty()) {
    return false;
  }

  auto out = line();
  out << name << "(";
  if (!EmitExpression(out, lhs_row_access->object)) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, lhs_col_access->index)) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, lhs_row_access->index)) {
    return false;
  }
  out << ", ";
  if (!EmitExpression(out, stmt->rhs)) {
    return false;
  }
  out << ");";

  return true;
}

bool GeneratorImpl::EmitIndexAccessor(
    std::ostream& out,
    const ast::IndexAccessorExpression* expr) {
  if (!EmitExpression(out, expr->object)) {
    return false;
  }
  out << "[";

  if (!EmitExpression(out, expr->index)) {
    return false;
  }
  out << "]";

  return true;
}

bool GeneratorImpl::EmitBitcast(std::ostream& out,
                                const ast::BitcastExpression* expr) {
  auto* type = TypeOf(expr);
  if (auto* vec = type->UnwrapRef()->As<sem::Vector>()) {
    type = vec->type();
  }

  if (!type->is_integer_scalar() && !type->is_float_scalar()) {
    diagnostics_.add_error(diag::System::Writer,
                           "Unable to do bitcast to type " +
                               type->FriendlyName(builder_.Symbols()));
    return false;
  }

  out << "as";
  if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                "")) {
    return false;
  }
  out << "(";
  if (!EmitExpression(out, expr->expr)) {
    return false;
  }
  out << ")";
  return true;
}

bool GeneratorImpl::EmitAssign(const ast::AssignmentStatement* stmt) {
  if (auto* lhs_access = stmt->lhs->As<ast::IndexAccessorExpression>()) {
    // BUG(crbug.com/tint/1333): work around assignment of scalar to matrices
    // with at least one dynamic index
    if (auto* lhs_sub_access =
            lhs_access->object->As<ast::IndexAccessorExpression>()) {
      if (auto* mat =
              TypeOf(lhs_sub_access->object)->UnwrapRef()->As<sem::Matrix>()) {
        auto* rhs_col_idx_sem = builder_.Sem().Get(lhs_access->index);
        auto* rhs_row_idx_sem = builder_.Sem().Get(lhs_sub_access->index);
        if (!rhs_col_idx_sem->ConstantValue().IsValid() ||
            !rhs_row_idx_sem->ConstantValue().IsValid()) {
          return EmitDynamicMatrixScalarAssignment(stmt, mat);
        }
      }
    }
    // BUG(crbug.com/tint/1333): work around assignment of vector to matrices
    // with dynamic indices
    const auto* lhs_access_type = TypeOf(lhs_access->object)->UnwrapRef();
    if (auto* mat = lhs_access_type->As<sem::Matrix>()) {
      auto* lhs_index_sem = builder_.Sem().Get(lhs_access->index);
      if (!lhs_index_sem->ConstantValue().IsValid()) {
        return EmitDynamicMatrixVectorAssignment(stmt, mat);
      }
    }
    // BUG(crbug.com/tint/534): work around assignment to vectors with dynamic
    // indices
    if (auto* vec = lhs_access_type->As<sem::Vector>()) {
      auto* rhs_sem = builder_.Sem().Get(lhs_access->index);
      if (!rhs_sem->ConstantValue().IsValid()) {
        return EmitDynamicVectorAssignment(stmt, vec);
      }
    }
  }

  auto out = line();
  if (!EmitExpression(out, stmt->lhs)) {
    return false;
  }
  out << " = ";
  if (!EmitExpression(out, stmt->rhs)) {
    return false;
  }
  out << ";";
  return true;
}

bool GeneratorImpl::EmitExpressionOrOneIfZero(std::ostream& out,
                                              const ast::Expression* expr) {
  // For constants, replace literal 0 with 1.
  sem::Constant::Scalars elems;
  if (const auto& val = builder_.Sem().Get(expr)->ConstantValue()) {
    if (!val.AnyZero()) {
      return EmitExpression(out, expr);
    }

    if (val.Type()->IsAnyOf<sem::I32, sem::U32>()) {
      return EmitValue(out, val.Type(), 1);
    }

    if (auto* vec = val.Type()->As<sem::Vector>()) {
      auto* elem_ty = vec->type();

      if (!EmitType(out, val.Type(), ast::StorageClass::kNone,
                    ast::Access::kUndefined, "")) {
        return false;
      }

      out << "(";
      for (size_t i = 0; i < val.Elements().size(); ++i) {
        if (i != 0) {
          out << ", ";
        }
        if (!val.WithScalarAt(i, [&](auto&& s) -> bool {
              // Use std::equal_to to work around -Wfloat-equal warnings
              auto equals_to =
                  std::equal_to<std::remove_reference_t<decltype(s)>>{};

              bool is_zero = equals_to(s, 0);
              return EmitValue(out, elem_ty, is_zero ? 1 : static_cast<int>(s));
            })) {
          return false;
        }
      }
      out << ")";
      return true;
    }

    TINT_ICE(Writer, diagnostics_)
        << "EmitExpressionOrOneIfZero expects integer scalar or vector";
    return false;
  }

  auto* ty = TypeOf(expr)->UnwrapRef();

  // For non-constants, we need to emit runtime code to check if the value is 0,
  // and return 1 in that case.
  std::string zero;
  {
    std::ostringstream ss;
    EmitValue(ss, ty, 0);
    zero = ss.str();
  }
  std::string one;
  {
    std::ostringstream ss;
    EmitValue(ss, ty, 1);
    one = ss.str();
  }

  // For identifiers, no need for a function call as it's fine to evaluate
  // `expr` more than once.
  if (expr->Is<ast::IdentifierExpression>()) {
    out << "(";
    if (!EmitExpression(out, expr)) {
      return false;
    }
    out << " == " << zero << " ? " << one << " : ";
    if (!EmitExpression(out, expr)) {
      return false;
    }
    out << ")";
    return true;
  }

  // For non-identifier expressions, call a function to make sure `expr` is only
  // evaluated once.
  auto name =
      utils::GetOrCreate(value_or_one_if_zero_, ty, [&]() -> std::string {
        // Example:
        // int4 tint_value_or_one_if_zero_int4(int4 value) {
        //   return value == 0 ? 0 : value;
        // }
        std::string ty_name;
        {
          std::ostringstream ss;
          if (!EmitType(ss, ty, tint::ast::StorageClass::kInvalid,
                        ast::Access::kUndefined, "")) {
            return "";
          }
          ty_name = ss.str();
        }

        std::string fn = UniqueIdentifier("value_or_one_if_zero_" + ty_name);
        line(&helpers_) << ty_name << " " << fn << "(" << ty_name
                        << " value) {";
        {
          ScopedIndent si(&helpers_);
          line(&helpers_) << "return value == " << zero << " ? " << one
                          << " : value;";
        }
        line(&helpers_) << "}";
        line(&helpers_);
        return fn;
      });

  if (name.empty()) {
    return false;
  }

  out << name << "(";
  if (!EmitExpression(out, expr)) {
    return false;
  }
  out << ")";
  return true;
}

bool GeneratorImpl::EmitBinary(std::ostream& out,
                               const ast::BinaryExpression* expr) {
  if (expr->op == ast::BinaryOp::kLogicalAnd ||
      expr->op == ast::BinaryOp::kLogicalOr) {
    auto name = UniqueIdentifier(kTempNamePrefix);

    {
      auto pre = line();
      pre << "bool " << name << " = ";
      if (!EmitExpression(pre, expr->lhs)) {
        return false;
      }
      pre << ";";
    }

    if (expr->op == ast::BinaryOp::kLogicalOr) {
      line() << "if (!" << name << ") {";
    } else {
      line() << "if (" << name << ") {";
    }

    {
      ScopedIndent si(this);
      auto pre = line();
      pre << name << " = ";
      if (!EmitExpression(pre, expr->rhs)) {
        return false;
      }
      pre << ";";
    }

    line() << "}";

    out << "(" << name << ")";
    return true;
  }

  auto* lhs_type = TypeOf(expr->lhs)->UnwrapRef();
  auto* rhs_type = TypeOf(expr->rhs)->UnwrapRef();
  // Multiplying by a matrix requires the use of `mul` in order to get the
  // type of multiply we desire.
  if (expr->op == ast::BinaryOp::kMultiply &&
      ((lhs_type->Is<sem::Vector>() && rhs_type->Is<sem::Matrix>()) ||
       (lhs_type->Is<sem::Matrix>() && rhs_type->Is<sem::Vector>()) ||
       (lhs_type->Is<sem::Matrix>() && rhs_type->Is<sem::Matrix>()))) {
    // Matrices are transposed, so swap LHS and RHS.
    out << "mul(";
    if (!EmitExpression(out, expr->rhs)) {
      return false;
    }
    out << ", ";
    if (!EmitExpression(out, expr->lhs)) {
      return false;
    }
    out << ")";

    return true;
  }

  out << "(";
  TINT_DEFER(out << ")");

  if (!EmitExpression(out, expr->lhs)) {
    return false;
  }
  out << " ";

  switch (expr->op) {
    case ast::BinaryOp::kAnd:
      out << "&";
      break;
    case ast::BinaryOp::kOr:
      out << "|";
      break;
    case ast::BinaryOp::kXor:
      out << "^";
      break;
    case ast::BinaryOp::kLogicalAnd:
    case ast::BinaryOp::kLogicalOr: {
      // These are both handled above.
      TINT_UNREACHABLE(Writer, diagnostics_);
      return false;
    }
    case ast::BinaryOp::kEqual:
      out << "==";
      break;
    case ast::BinaryOp::kNotEqual:
      out << "!=";
      break;
    case ast::BinaryOp::kLessThan:
      out << "<";
      break;
    case ast::BinaryOp::kGreaterThan:
      out << ">";
      break;
    case ast::BinaryOp::kLessThanEqual:
      out << "<=";
      break;
    case ast::BinaryOp::kGreaterThanEqual:
      out << ">=";
      break;
    case ast::BinaryOp::kShiftLeft:
      out << "<<";
      break;
    case ast::BinaryOp::kShiftRight:
      // TODO(dsinclair): MSL is based on C++14, and >> in C++14 has
      // implementation-defined behaviour for negative LHS.  We may have to
      // generate extra code to implement WGSL-specified behaviour for negative
      // LHS.
      out << R"(>>)";
      break;

    case ast::BinaryOp::kAdd:
      out << "+";
      break;
    case ast::BinaryOp::kSubtract:
      out << "-";
      break;
    case ast::BinaryOp::kMultiply:
      out << "*";
      break;
    case ast::BinaryOp::kDivide:
      out << "/";
      // BUG(crbug.com/tint/1083): Integer divide/modulo by zero is a FXC
      // compile error, and undefined behavior in WGSL.
      if (TypeOf(expr->rhs)->UnwrapRef()->is_integer_scalar_or_vector()) {
        out << " ";
        return EmitExpressionOrOneIfZero(out, expr->rhs);
      }
      break;
    case ast::BinaryOp::kModulo:
      out << "%";
      // BUG(crbug.com/tint/1083): Integer divide/modulo by zero is a FXC
      // compile error, and undefined behavior in WGSL.
      if (TypeOf(expr->rhs)->UnwrapRef()->is_integer_scalar_or_vector()) {
        out << " ";
        return EmitExpressionOrOneIfZero(out, expr->rhs);
      }
      break;
    case ast::BinaryOp::kNone:
      diagnostics_.add_error(diag::System::Writer,
                             "missing binary operation type");
      return false;
  }
  out << " ";

  if (!EmitExpression(out, expr->rhs)) {
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitStatements(const ast::StatementList& stmts) {
  for (auto* s : stmts) {
    if (!EmitStatement(s)) {
      return false;
    }
  }
  return true;
}

bool GeneratorImpl::EmitStatementsWithIndent(const ast::StatementList& stmts) {
  ScopedIndent si(this);
  return EmitStatements(stmts);
}

bool GeneratorImpl::EmitBlock(const ast::BlockStatement* stmt) {
  line() << "{";
  if (!EmitStatementsWithIndent(stmt->statements)) {
    return false;
  }
  line() << "}";
  return true;
}

bool GeneratorImpl::EmitBreak(const ast::BreakStatement*) {
  line() << "break;";
  return true;
}

bool GeneratorImpl::EmitCall(std::ostream& out,
                             const ast::CallExpression* expr) {
  auto* call = builder_.Sem().Get(expr);
  auto* target = call->Target();
  return Switch(
      target,
      [&](const sem::Function* func) {
        return EmitFunctionCall(out, call, func);
      },
      [&](const sem::Builtin* builtin) {
        return EmitBuiltinCall(out, call, builtin);
      },
      [&](const sem::TypeConversion* conv) {
        return EmitTypeConversion(out, call, conv);
      },
      [&](const sem::TypeConstructor* ctor) {
        return EmitTypeConstructor(out, call, ctor);
      },
      [&](Default) {
        TINT_ICE(Writer, diagnostics_)
            << "unhandled call target: " << target->TypeInfo().name;
        return false;
      });
}

bool GeneratorImpl::EmitFunctionCall(std::ostream& out,
                                     const sem::Call* call,
                                     const sem::Function* func) {
  auto* expr = call->Declaration();

  if (ast::HasAttribute<transform::CalculateArrayLength::BufferSizeIntrinsic>(
          func->Declaration()->attributes)) {
    // Special function generated by the CalculateArrayLength transform for
    // calling X.GetDimensions(Y)
    if (!EmitExpression(out, call->Arguments()[0]->Declaration())) {
      return false;
    }
    out << ".GetDimensions(";
    if (!EmitExpression(out, call->Arguments()[1]->Declaration())) {
      return false;
    }
    out << ")";
    return true;
  }

  if (auto* intrinsic =
          ast::GetAttribute<transform::DecomposeMemoryAccess::Intrinsic>(
              func->Declaration()->attributes)) {
    switch (intrinsic->storage_class) {
      case ast::StorageClass::kUniform:
        return EmitUniformBufferAccess(out, expr, intrinsic);
      case ast::StorageClass::kStorage:
        return EmitStorageBufferAccess(out, expr, intrinsic);
      default:
        TINT_UNREACHABLE(Writer, diagnostics_)
            << "unsupported DecomposeMemoryAccess::Intrinsic storage class:"
            << intrinsic->storage_class;
        return false;
    }
  }

  out << builder_.Symbols().NameFor(func->Declaration()->symbol) << "(";

  bool first = true;
  for (auto* arg : call->Arguments()) {
    if (!first) {
      out << ", ";
    }
    first = false;

    if (!EmitExpression(out, arg->Declaration())) {
      return false;
    }
  }

  out << ")";
  return true;
}

bool GeneratorImpl::EmitBuiltinCall(std::ostream& out,
                                    const sem::Call* call,
                                    const sem::Builtin* builtin) {
  auto* expr = call->Declaration();
  if (builtin->IsTexture()) {
    return EmitTextureCall(out, call, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kSelect) {
    return EmitSelectCall(out, expr);
  }
  if (builtin->Type() == sem::BuiltinType::kModf) {
    return EmitModfCall(out, expr, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kFrexp) {
    return EmitFrexpCall(out, expr, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kDegrees) {
    return EmitDegreesCall(out, expr, builtin);
  }
  if (builtin->Type() == sem::BuiltinType::kRadians) {
    return EmitRadiansCall(out, expr, builtin);
  }
  if (builtin->IsDataPacking()) {
    return EmitDataPackingCall(out, expr, builtin);
  }
  if (builtin->IsDataUnpacking()) {
    return EmitDataUnpackingCall(out, expr, builtin);
  }
  if (builtin->IsBarrier()) {
    return EmitBarrierCall(out, builtin);
  }
  if (builtin->IsAtomic()) {
    return EmitWorkgroupAtomicCall(out, expr, builtin);
  }
  auto name = generate_builtin_name(builtin);
  if (name.empty()) {
    return false;
  }

  out << name << "(";

  bool first = true;
  for (auto* arg : call->Arguments()) {
    if (!first) {
      out << ", ";
    }
    first = false;

    if (!EmitExpression(out, arg->Declaration())) {
      return false;
    }
  }

  out << ")";
  return true;
}

bool GeneratorImpl::EmitTypeConversion(std::ostream& out,
                                       const sem::Call* call,
                                       const sem::TypeConversion* conv) {
  if (!EmitType(out, conv->Target(), ast::StorageClass::kNone,
                ast::Access::kReadWrite, "")) {
    return false;
  }
  out << "(";

  if (!EmitExpression(out, call->Arguments()[0]->Declaration())) {
    return false;
  }

  out << ")";
  return true;
}

bool GeneratorImpl::EmitTypeConstructor(std::ostream& out,
                                        const sem::Call* call,
                                        const sem::TypeConstructor* ctor) {
  auto* type = call->Type();

  // If the type constructor is empty then we need to construct with the zero
  // value for all components.
  if (call->Arguments().empty()) {
    return EmitZeroValue(out, type);
  }

  bool brackets = type->IsAnyOf<sem::Array, sem::Struct>();

  // For single-value vector initializers, swizzle the scalar to the right
  // vector dimension using .x
  const bool is_single_value_vector_init =
      type->is_scalar_vector() && call->Arguments().size() == 1 &&
      ctor->Parameters()[0]->Type()->is_scalar();

  auto it = structure_builders_.find(As<sem::Struct>(type));
  if (it != structure_builders_.end()) {
    out << it->second << "(";
    brackets = false;
  } else if (brackets) {
    out << "{";
  } else {
    if (!EmitType(out, type, ast::StorageClass::kNone, ast::Access::kReadWrite,
                  "")) {
      return false;
    }
    out << "(";
  }

  if (is_single_value_vector_init) {
    out << "(";
  }

  bool first = true;
  for (auto* e : call->Arguments()) {
    if (!first) {
      out << ", ";
    }
    first = false;

    if (!EmitExpression(out, e->Declaration())) {
      return false;
    }
  }

  if (is_single_value_vector_init) {
    out << ")." << std::string(type->As<sem::Vector>()->Width(), 'x');
  }

  out << (brackets ? "}" : ")");
  return true;
}

bool GeneratorImpl::EmitUniformBufferAccess(
    std::ostream& out,
    const ast::CallExpression* expr,
    const transform::DecomposeMemoryAccess::Intrinsic* intrinsic) {
  const auto& args = expr->args;
  auto* offset_arg = builder_.Sem().Get(args[1]);

  uint32_t scalar_offset_value = 0;
  std::string scalar_offset_expr;

  // If true, use scalar_offset_value, otherwise use scalar_offset_expr
  bool scalar_offset_constant = false;

  if (auto val = offset_arg->ConstantValue()) {
    TINT_ASSERT(Writer, val.Type()->Is<sem::U32>());
    scalar_offset_value = val.Elements()[0].u32;
    scalar_offset_value /= 4;  // bytes -> scalar index
    scalar_offset_constant = true;
  }

  if (!scalar_offset_constant) {
    // UBO offset not compile-time known.
    // Calculate the scalar offset into a temporary.
    scalar_offset_expr = UniqueIdentifier("scalar_offset");
    auto pre = line();
    pre << "const uint " << scalar_offset_expr << " = (";
    if (!EmitExpression(pre, args[1])) {  // offset
      return false;
    }
    pre << ") / 4;";
  }

  using Op = transform::DecomposeMemoryAccess::Intrinsic::Op;
  using DataType = transform::DecomposeMemoryAccess::Intrinsic::DataType;
  switch (intrinsic->op) {
    case Op::kLoad: {
      auto cast = [&](const char* to, auto&& load) {
        out << to << "(";
        auto result = load();
        out << ")";
        return result;
      };
      auto load_scalar = [&]() {
        if (!EmitExpression(out, args[0])) {  // buffer
          return false;
        }
        if (scalar_offset_constant) {
          char swizzle[] = {'x', 'y', 'z', 'w'};
          out << "[" << (scalar_offset_value / 4) << "]."
              << swizzle[scalar_offset_value & 3];
        } else {
          out << "[" << scalar_offset_expr << " / 4][" << scalar_offset_expr
              << " % 4]";
        }
        return true;
      };
      // Has a minimum alignment of 8 bytes, so is either .xy or .zw
      auto load_vec2 = [&] {
        if (scalar_offset_constant) {
          if (!EmitExpression(out, args[0])) {  // buffer
            return false;
          }
          out << "[" << (scalar_offset_value / 4) << "]";
          out << ((scalar_offset_value & 2) == 0 ? ".xy" : ".zw");
        } else {
          std::string ubo_load = UniqueIdentifier("ubo_load");
          {
            auto pre = line();
            pre << "uint4 " << ubo_load << " = ";
            if (!EmitExpression(pre, args[0])) {  // buffer
              return false;
            }
            pre << "[" << scalar_offset_expr << " / 4];";
          }
          out << "((" << scalar_offset_expr << " & 2) ? " << ubo_load
              << ".zw : " << ubo_load << ".xy)";
        }
        return true;
      };
      // vec4 has a minimum alignment of 16 bytes, easiest case
      auto load_vec4 = [&] {
        if (!EmitExpression(out, args[0])) {  // buffer
          return false;
        }
        if (scalar_offset_constant) {
          out << "[" << (scalar_offset_value / 4) << "]";
        } else {
          out << "[" << scalar_offset_expr << " / 4]";
        }
        return true;
      };
      // vec3 has a minimum alignment of 16 bytes, so is just a .xyz swizzle
      auto load_vec3 = [&] {
        if (!load_vec4()) {
          return false;
        }
        out << ".xyz";
        return true;
      };
      switch (intrinsic->type) {
        case DataType::kU32:
          return load_scalar();
        case DataType::kF32:
          return cast("asfloat", load_scalar);
        case DataType::kI32:
          return cast("asint", load_scalar);
        case DataType::kVec2U32:
          return load_vec2();
        case DataType::kVec2F32:
          return cast("asfloat", load_vec2);
        case DataType::kVec2I32:
          return cast("asint", load_vec2);
        case DataType::kVec3U32:
          return load_vec3();
        case DataType::kVec3F32:
          return cast("asfloat", load_vec3);
        case DataType::kVec3I32:
          return cast("asint", load_vec3);
        case DataType::kVec4U32:
          return load_vec4();
        case DataType::kVec4F32:
          return cast("asfloat", load_vec4);
        case DataType::kVec4I32:
          return cast("asint", load_vec4);
      }
      TINT_UNREACHABLE(Writer, diagnostics_)
          << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
          << static_cast<int>(intrinsic->type);
      return false;
    }
    default:
      break;
  }
  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported DecomposeMemoryAccess::Intrinsic::Op: "
      << static_cast<int>(intrinsic->op);
  return false;
}

bool GeneratorImpl::EmitStorageBufferAccess(
    std::ostream& out,
    const ast::CallExpression* expr,
    const transform::DecomposeMemoryAccess::Intrinsic* intrinsic) {
  const auto& args = expr->args;

  using Op = transform::DecomposeMemoryAccess::Intrinsic::Op;
  using DataType = transform::DecomposeMemoryAccess::Intrinsic::DataType;
  switch (intrinsic->op) {
    case Op::kLoad: {
      auto load = [&](const char* cast, int n) {
        if (cast) {
          out << cast << "(";
        }
        if (!EmitExpression(out, args[0])) {  // buffer
          return false;
        }
        out << ".Load";
        if (n > 1) {
          out << n;
        }
        ScopedParen sp(out);
        if (!EmitExpression(out, args[1])) {  // offset
          return false;
        }
        if (cast) {
          out << ")";
        }
        return true;
      };
      switch (intrinsic->type) {
        case DataType::kU32:
          return load(nullptr, 1);
        case DataType::kF32:
          return load("asfloat", 1);
        case DataType::kI32:
          return load("asint", 1);
        case DataType::kVec2U32:
          return load(nullptr, 2);
        case DataType::kVec2F32:
          return load("asfloat", 2);
        case DataType::kVec2I32:
          return load("asint", 2);
        case DataType::kVec3U32:
          return load(nullptr, 3);
        case DataType::kVec3F32:
          return load("asfloat", 3);
        case DataType::kVec3I32:
          return load("asint", 3);
        case DataType::kVec4U32:
          return load(nullptr, 4);
        case DataType::kVec4F32:
          return load("asfloat", 4);
        case DataType::kVec4I32:
          return load("asint", 4);
      }
      TINT_UNREACHABLE(Writer, diagnostics_)
          << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
          << static_cast<int>(intrinsic->type);
      return false;
    }

    case Op::kStore: {
      auto store = [&](int n) {
        if (!EmitExpression(out, args[0])) {  // buffer
          return false;
        }
        out << ".Store";
        if (n > 1) {
          out << n;
        }
        ScopedParen sp1(out);
        if (!EmitExpression(out, args[1])) {  // offset
          return false;
        }
        out << ", asuint";
        ScopedParen sp2(out);
        if (!EmitExpression(out, args[2])) {  // value
          return false;
        }
        return true;
      };
      switch (intrinsic->type) {
        case DataType::kU32:
          return store(1);
        case DataType::kF32:
          return store(1);
        case DataType::kI32:
          return store(1);
        case DataType::kVec2U32:
          return store(2);
        case DataType::kVec2F32:
          return store(2);
        case DataType::kVec2I32:
          return store(2);
        case DataType::kVec3U32:
          return store(3);
        case DataType::kVec3F32:
          return store(3);
        case DataType::kVec3I32:
          return store(3);
        case DataType::kVec4U32:
          return store(4);
        case DataType::kVec4F32:
          return store(4);
        case DataType::kVec4I32:
          return store(4);
      }
      TINT_UNREACHABLE(Writer, diagnostics_)
          << "unsupported DecomposeMemoryAccess::Intrinsic::DataType: "
          << static_cast<int>(intrinsic->type);
      return false;
    }

    case Op::kAtomicLoad:
    case Op::kAtomicStore:
    case Op::kAtomicAdd:
    case Op::kAtomicSub:
    case Op::kAtomicMax:
    case Op::kAtomicMin:
    case Op::kAtomicAnd:
    case Op::kAtomicOr:
    case Op::kAtomicXor:
    case Op::kAtomicExchange:
    case Op::kAtomicCompareExchangeWeak:
      return EmitStorageAtomicCall(out, expr, intrinsic);
  }

  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported DecomposeMemoryAccess::Intrinsic::Op: "
      << static_cast<int>(intrinsic->op);
  return false;
}

bool GeneratorImpl::EmitStorageAtomicCall(
    std::ostream& out,
    const ast::CallExpression* expr,
    const transform::DecomposeMemoryAccess::Intrinsic* intrinsic) {
  using Op = transform::DecomposeMemoryAccess::Intrinsic::Op;

  auto* result_ty = TypeOf(expr);

  auto& buf = helpers_;

  // generate_helper() generates a helper function that translates the
  // DecomposeMemoryAccess::Intrinsic call into the corresponding HLSL
  // atomic intrinsic function.
  auto generate_helper = [&]() -> std::string {
    auto rmw = [&](const char* wgsl, const char* hlsl) -> std::string {
      auto name = UniqueIdentifier(wgsl);
      {
        auto fn = line(&buf);
        if (!EmitTypeAndName(fn, result_ty, ast::StorageClass::kNone,
                             ast::Access::kUndefined, name)) {
          return "";
        }
        fn << "(RWByteAddressBuffer buffer, uint offset, ";
        if (!EmitTypeAndName(fn, result_ty, ast::StorageClass::kNone,
                             ast::Access::kUndefined, "value")) {
          return "";
        }
        fn << ") {";
      }

      buf.IncrementIndent();
      TINT_DEFER({
        buf.DecrementIndent();
        line(&buf) << "}";
        line(&buf);
      });

      {
        auto l = line(&buf);
        if (!EmitTypeAndName(l, result_ty, ast::StorageClass::kNone,
                             ast::Access::kUndefined, "original_value")) {
          return "";
        }
        l << " = 0;";
      }
      {
        auto l = line(&buf);
        l << "buffer." << hlsl << "(offset, ";
        if (intrinsic->op == Op::kAtomicSub) {
          l << "-";
        }
        l << "value, original_value);";
      }
      line(&buf) << "return original_value;";
      return name;
    };

    switch (intrinsic->op) {
      case Op::kAtomicAdd:
        return rmw("atomicAdd", "InterlockedAdd");

      case Op::kAtomicSub:
        // Use add with the operand negated.
        return rmw("atomicSub", "InterlockedAdd");

      case Op::kAtomicMax:
        return rmw("atomicMax", "InterlockedMax");

      case Op::kAtomicMin:
        return rmw("atomicMin", "InterlockedMin");

      case Op::kAtomicAnd:
        return rmw("atomicAnd", "InterlockedAnd");

      case Op::kAtomicOr:
        return rmw("atomicOr", "InterlockedOr");

      case Op::kAtomicXor:
        return rmw("atomicXor", "InterlockedXor");

      case Op::kAtomicExchange:
        return rmw("atomicExchange", "InterlockedExchange");

      case Op::kAtomicLoad: {
        // HLSL does not have an InterlockedLoad, so we emulate it with
        // InterlockedOr using 0 as the OR value
        auto name = UniqueIdentifier("atomicLoad");
        {
          auto fn = line(&buf);
          if (!EmitTypeAndName(fn, result_ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, name)) {
            return "";
          }
          fn << "(RWByteAddressBuffer buffer, uint offset) {";
        }

        buf.IncrementIndent();
        TINT_DEFER({
          buf.DecrementIndent();
          line(&buf) << "}";
          line(&buf);
        });

        {
          auto l = line(&buf);
          if (!EmitTypeAndName(l, result_ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, "value")) {
            return "";
          }
          l << " = 0;";
        }

        line(&buf) << "buffer.InterlockedOr(offset, 0, value);";
        line(&buf) << "return value;";
        return name;
      }
      case Op::kAtomicStore: {
        // HLSL does not have an InterlockedStore, so we emulate it with
        // InterlockedExchange and discard the returned value
        auto* value_ty = TypeOf(expr->args[2])->UnwrapRef();
        auto name = UniqueIdentifier("atomicStore");
        {
          auto fn = line(&buf);
          fn << "void " << name << "(RWByteAddressBuffer buffer, uint offset, ";
          if (!EmitTypeAndName(fn, value_ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, "value")) {
            return "";
          }
          fn << ") {";
        }

        buf.IncrementIndent();
        TINT_DEFER({
          buf.DecrementIndent();
          line(&buf) << "}";
          line(&buf);
        });

        {
          auto l = line(&buf);
          if (!EmitTypeAndName(l, value_ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, "ignored")) {
            return "";
          }
          l << ";";
        }
        line(&buf) << "buffer.InterlockedExchange(offset, value, ignored);";
        return name;
      }
      case Op::kAtomicCompareExchangeWeak: {
        auto* value_ty = TypeOf(expr->args[2])->UnwrapRef();

        auto name = UniqueIdentifier("atomicCompareExchangeWeak");
        {
          auto fn = line(&buf);
          if (!EmitTypeAndName(fn, result_ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, name)) {
            return "";
          }
          fn << "(RWByteAddressBuffer buffer, uint offset, ";
          if (!EmitTypeAndName(fn, value_ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, "compare")) {
            return "";
          }
          fn << ", ";
          if (!EmitTypeAndName(fn, value_ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, "value")) {
            return "";
          }
          fn << ") {";
        }

        buf.IncrementIndent();
        TINT_DEFER({
          buf.DecrementIndent();
          line(&buf) << "}";
          line(&buf);
        });

        {  // T result = {0, 0};
          auto l = line(&buf);
          if (!EmitTypeAndName(l, result_ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, "result")) {
            return "";
          }
          l << " = {0, 0};";
        }
        line(&buf) << "buffer.InterlockedCompareExchange(offset, compare, "
                      "value, result.x);";
        line(&buf) << "result.y = result.x == compare;";
        line(&buf) << "return result;";
        return name;
      }
      default:
        break;
    }
    TINT_UNREACHABLE(Writer, diagnostics_)
        << "unsupported atomic DecomposeMemoryAccess::Intrinsic::Op: "
        << static_cast<int>(intrinsic->op);
    return "";
  };

  auto func = utils::GetOrCreate(dma_intrinsics_,
                                 DMAIntrinsic{intrinsic->op, intrinsic->type},
                                 generate_helper);
  if (func.empty()) {
    return false;
  }

  out << func;
  {
    ScopedParen sp(out);
    bool first = true;
    for (auto* arg : expr->args) {
      if (!first) {
        out << ", ";
      }
      first = false;
      if (!EmitExpression(out, arg)) {
        return false;
      }
    }
  }

  return true;
}

bool GeneratorImpl::EmitWorkgroupAtomicCall(std::ostream& out,
                                            const ast::CallExpression* expr,
                                            const sem::Builtin* builtin) {
  std::string result = UniqueIdentifier("atomic_result");

  if (!builtin->ReturnType()->Is<sem::Void>()) {
    auto pre = line();
    if (!EmitTypeAndName(pre, builtin->ReturnType(), ast::StorageClass::kNone,
                         ast::Access::kUndefined, result)) {
      return false;
    }
    pre << " = ";
    if (!EmitZeroValue(pre, builtin->ReturnType())) {
      return false;
    }
    pre << ";";
  }

  auto call = [&](const char* name) {
    auto pre = line();
    pre << name;

    {
      ScopedParen sp(pre);
      for (size_t i = 0; i < expr->args.size(); i++) {
        auto* arg = expr->args[i];
        if (i > 0) {
          pre << ", ";
        }
        if (i == 1 && builtin->Type() == sem::BuiltinType::kAtomicSub) {
          // Sub uses InterlockedAdd with the operand negated.
          pre << "-";
        }
        if (!EmitExpression(pre, arg)) {
          return false;
        }
      }

      pre << ", " << result;
    }

    pre << ";";

    out << result;
    return true;
  };

  switch (builtin->Type()) {
    case sem::BuiltinType::kAtomicLoad: {
      // HLSL does not have an InterlockedLoad, so we emulate it with
      // InterlockedOr using 0 as the OR value
      auto pre = line();
      pre << "InterlockedOr";
      {
        ScopedParen sp(pre);
        if (!EmitExpression(pre, expr->args[0])) {
          return false;
        }
        pre << ", 0, " << result;
      }
      pre << ";";

      out << result;
      return true;
    }
    case sem::BuiltinType::kAtomicStore: {
      // HLSL does not have an InterlockedStore, so we emulate it with
      // InterlockedExchange and discard the returned value
      {  // T result = 0;
        auto pre = line();
        auto* value_ty = builtin->Parameters()[1]->Type()->UnwrapRef();
        if (!EmitTypeAndName(pre, value_ty, ast::StorageClass::kNone,
                             ast::Access::kUndefined, result)) {
          return false;
        }
        pre << " = ";
        if (!EmitZeroValue(pre, value_ty)) {
          return false;
        }
        pre << ";";
      }

      out << "InterlockedExchange";
      {
        ScopedParen sp(out);
        if (!EmitExpression(out, expr->args[0])) {
          return false;
        }
        out << ", ";
        if (!EmitExpression(out, expr->args[1])) {
          return false;
        }
        out << ", " << result;
      }
      return true;
    }
    case sem::BuiltinType::kAtomicCompareExchangeWeak: {
      auto* dest = expr->args[0];
      auto* compare_value = expr->args[1];
      auto* value = expr->args[2];

      std::string compare = UniqueIdentifier("atomic_compare_value");

      {  // T compare_value = <compare_value>;
        auto pre = line();
        if (!EmitTypeAndName(pre, TypeOf(compare_value),
                             ast::StorageClass::kNone, ast::Access::kUndefined,
                             compare)) {
          return false;
        }
        pre << " = ";
        if (!EmitExpression(pre, compare_value)) {
          return false;
        }
        pre << ";";
      }

      {  // InterlockedCompareExchange(dst, compare, value, result.x);
        auto pre = line();
        pre << "InterlockedCompareExchange";
        {
          ScopedParen sp(pre);
          if (!EmitExpression(pre, dest)) {
            return false;
          }
          pre << ", " << compare << ", ";
          if (!EmitExpression(pre, value)) {
            return false;
          }
          pre << ", " << result << ".x";
        }
        pre << ";";
      }

      {  // result.y = result.x == compare;
        line() << result << ".y = " << result << ".x == " << compare << ";";
      }

      out << result;
      return true;
    }

    case sem::BuiltinType::kAtomicAdd:
    case sem::BuiltinType::kAtomicSub:
      return call("InterlockedAdd");

    case sem::BuiltinType::kAtomicMax:
      return call("InterlockedMax");

    case sem::BuiltinType::kAtomicMin:
      return call("InterlockedMin");

    case sem::BuiltinType::kAtomicAnd:
      return call("InterlockedAnd");

    case sem::BuiltinType::kAtomicOr:
      return call("InterlockedOr");

    case sem::BuiltinType::kAtomicXor:
      return call("InterlockedXor");

    case sem::BuiltinType::kAtomicExchange:
      return call("InterlockedExchange");

    default:
      break;
  }

  TINT_UNREACHABLE(Writer, diagnostics_)
      << "unsupported atomic builtin: " << builtin->Type();
  return false;
}

bool GeneratorImpl::EmitSelectCall(std::ostream& out,
                                   const ast::CallExpression* expr) {
  auto* expr_false = expr->args[0];
  auto* expr_true = expr->args[1];
  auto* expr_cond = expr->args[2];
  ScopedParen paren(out);
  if (!EmitExpression(out, expr_cond)) {
    return false;
  }

  out << " ? ";

  if (!EmitExpression(out, expr_true)) {
    return false;
  }

  out << " : ";

  if (!EmitExpression(out, expr_false)) {
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitModfCall(std::ostream& out,
                                 const ast::CallExpression* expr,
                                 const sem::Builtin* builtin) {
  return CallBuiltinHelper(
      out, expr, builtin,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        auto* ty = builtin->Parameters()[0]->Type();
        auto in = params[0];

        std::string width;
        if (auto* vec = ty->As<sem::Vector>()) {
          width = std::to_string(vec->Width());
        }

        // Emit the builtin return type unique to this overload. This does not
        // exist in the AST, so it will not be generated in Generate().
        if (!EmitStructType(&helpers_,
                            builtin->ReturnType()->As<sem::Struct>())) {
          return false;
        }

        line(b) << "float" << width << " whole;";
        line(b) << "float" << width << " fract = modf(" << in << ", whole);";
        {
          auto l = line(b);
          if (!EmitType(l, builtin->ReturnType(), ast::StorageClass::kNone,
                        ast::Access::kUndefined, "")) {
            return false;
          }
          l << " result = {fract, whole};";
        }
        line(b) << "return result;";
        return true;
      });
}

bool GeneratorImpl::EmitFrexpCall(std::ostream& out,
                                  const ast::CallExpression* expr,
                                  const sem::Builtin* builtin) {
  return CallBuiltinHelper(
      out, expr, builtin,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        auto* ty = builtin->Parameters()[0]->Type();
        auto in = params[0];

        std::string width;
        if (auto* vec = ty->As<sem::Vector>()) {
          width = std::to_string(vec->Width());
        }

        // Emit the builtin return type unique to this overload. This does not
        // exist in the AST, so it will not be generated in Generate().
        if (!EmitStructType(&helpers_,
                            builtin->ReturnType()->As<sem::Struct>())) {
          return false;
        }

        line(b) << "float" << width << " exp;";
        line(b) << "float" << width << " sig = frexp(" << in << ", exp);";
        {
          auto l = line(b);
          if (!EmitType(l, builtin->ReturnType(), ast::StorageClass::kNone,
                        ast::Access::kUndefined, "")) {
            return false;
          }
          l << " result = {sig, int" << width << "(exp)};";
        }
        line(b) << "return result;";
        return true;
      });
}

bool GeneratorImpl::EmitDegreesCall(std::ostream& out,
                                    const ast::CallExpression* expr,
                                    const sem::Builtin* builtin) {
  return CallBuiltinHelper(
      out, expr, builtin,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        line(b) << "return " << params[0] << " * " << std::setprecision(20)
                << sem::kRadToDeg << ";";
        return true;
      });
}

bool GeneratorImpl::EmitRadiansCall(std::ostream& out,
                                    const ast::CallExpression* expr,
                                    const sem::Builtin* builtin) {
  return CallBuiltinHelper(
      out, expr, builtin,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        line(b) << "return " << params[0] << " * " << std::setprecision(20)
                << sem::kDegToRad << ";";
        return true;
      });
}

bool GeneratorImpl::EmitDataPackingCall(std::ostream& out,
                                        const ast::CallExpression* expr,
                                        const sem::Builtin* builtin) {
  return CallBuiltinHelper(
      out, expr, builtin,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        uint32_t dims = 2;
        bool is_signed = false;
        uint32_t scale = 65535;
        if (builtin->Type() == sem::BuiltinType::kPack4x8snorm ||
            builtin->Type() == sem::BuiltinType::kPack4x8unorm) {
          dims = 4;
          scale = 255;
        }
        if (builtin->Type() == sem::BuiltinType::kPack4x8snorm ||
            builtin->Type() == sem::BuiltinType::kPack2x16snorm) {
          is_signed = true;
          scale = (scale - 1) / 2;
        }
        switch (builtin->Type()) {
          case sem::BuiltinType::kPack4x8snorm:
          case sem::BuiltinType::kPack4x8unorm:
          case sem::BuiltinType::kPack2x16snorm:
          case sem::BuiltinType::kPack2x16unorm: {
            {
              auto l = line(b);
              l << (is_signed ? "" : "u") << "int" << dims
                << " i = " << (is_signed ? "" : "u") << "int" << dims
                << "(round(clamp(" << params[0] << ", "
                << (is_signed ? "-1.0" : "0.0") << ", 1.0) * " << scale
                << ".0))";
              if (is_signed) {
                l << " & " << (dims == 4 ? "0xff" : "0xffff");
              }
              l << ";";
            }
            {
              auto l = line(b);
              l << "return ";
              if (is_signed) {
                l << "asuint";
              }
              l << "(i.x | i.y << " << (32 / dims);
              if (dims == 4) {
                l << " | i.z << 16 | i.w << 24";
              }
              l << ");";
            }
            break;
          }
          case sem::BuiltinType::kPack2x16float: {
            line(b) << "uint2 i = f32tof16(" << params[0] << ");";
            line(b) << "return i.x | (i.y << 16);";
            break;
          }
          default:
            diagnostics_.add_error(
                diag::System::Writer,
                "Internal error: unhandled data packing builtin");
            return false;
        }

        return true;
      });
}

bool GeneratorImpl::EmitDataUnpackingCall(std::ostream& out,
                                          const ast::CallExpression* expr,
                                          const sem::Builtin* builtin) {
  return CallBuiltinHelper(
      out, expr, builtin,
      [&](TextBuffer* b, const std::vector<std::string>& params) {
        uint32_t dims = 2;
        bool is_signed = false;
        uint32_t scale = 65535;
        if (builtin->Type() == sem::BuiltinType::kUnpack4x8snorm ||
            builtin->Type() == sem::BuiltinType::kUnpack4x8unorm) {
          dims = 4;
          scale = 255;
        }
        if (builtin->Type() == sem::BuiltinType::kUnpack4x8snorm ||
            builtin->Type() == sem::BuiltinType::kUnpack2x16snorm) {
          is_signed = true;
          scale = (scale - 1) / 2;
        }
        switch (builtin->Type()) {
          case sem::BuiltinType::kUnpack4x8snorm:
          case sem::BuiltinType::kUnpack2x16snorm: {
            line(b) << "int j = int(" << params[0] << ");";
            {  // Perform sign extension on the converted values.
              auto l = line(b);
              l << "int" << dims << " i = int" << dims << "(";
              if (dims == 2) {
                l << "j << 16, j) >> 16";
              } else {
                l << "j << 24, j << 16, j << 8, j) >> 24";
              }
              l << ";";
            }
            line(b) << "return clamp(float" << dims << "(i) / " << scale
                    << ".0, " << (is_signed ? "-1.0" : "0.0") << ", 1.0);";
            break;
          }
          case sem::BuiltinType::kUnpack4x8unorm:
          case sem::BuiltinType::kUnpack2x16unorm: {
            line(b) << "uint j = " << params[0] << ";";
            {
              auto l = line(b);
              l << "uint" << dims << " i = uint" << dims << "(";
              l << "j & " << (dims == 2 ? "0xffff" : "0xff") << ", ";
              if (dims == 4) {
                l << "(j >> " << (32 / dims)
                  << ") & 0xff, (j >> 16) & 0xff, j >> 24";
              } else {
                l << "j >> " << (32 / dims);
              }
              l << ");";
            }
            line(b) << "return float" << dims << "(i) / " << scale << ".0;";
            break;
          }
          case sem::BuiltinType::kUnpack2x16float:
            line(b) << "uint i = " << params[0] << ";";
            line(b) << "return f16tof32(uint2(i & 0xffff, i >> 16));";
            break;
          default:
            diagnostics_.add_error(
                diag::System::Writer,
                "Internal error: unhandled data packing builtin");
            return false;
        }

        return true;
      });
}

bool GeneratorImpl::EmitBarrierCall(std::ostream& out,
                                    const sem::Builtin* builtin) {
  // TODO(crbug.com/tint/661): Combine sequential barriers to a single
  // instruction.
  if (builtin->Type() == sem::BuiltinType::kWorkgroupBarrier) {
    out << "GroupMemoryBarrierWithGroupSync()";
  } else if (builtin->Type() == sem::BuiltinType::kStorageBarrier) {
    out << "DeviceMemoryBarrierWithGroupSync()";
  } else {
    TINT_UNREACHABLE(Writer, diagnostics_)
        << "unexpected barrier builtin type " << sem::str(builtin->Type());
    return false;
  }
  return true;
}

bool GeneratorImpl::EmitTextureCall(std::ostream& out,
                                    const sem::Call* call,
                                    const sem::Builtin* builtin) {
  using Usage = sem::ParameterUsage;

  auto& signature = builtin->Signature();
  auto* expr = call->Declaration();
  auto arguments = expr->args;

  // Returns the argument with the given usage
  auto arg = [&](Usage usage) {
    int idx = signature.IndexOf(usage);
    return (idx >= 0) ? arguments[idx] : nullptr;
  };

  auto* texture = arg(Usage::kTexture);
  if (!texture) {
    TINT_ICE(Writer, diagnostics_) << "missing texture argument";
    return false;
  }

  auto* texture_type = TypeOf(texture)->UnwrapRef()->As<sem::Texture>();

  switch (builtin->Type()) {
    case sem::BuiltinType::kTextureDimensions:
    case sem::BuiltinType::kTextureNumLayers:
    case sem::BuiltinType::kTextureNumLevels:
    case sem::BuiltinType::kTextureNumSamples: {
      // All of these builtins use the GetDimensions() method on the texture
      bool is_ms = texture_type->IsAnyOf<sem::MultisampledTexture,
                                         sem::DepthMultisampledTexture>();
      int num_dimensions = 0;
      std::string swizzle;

      switch (builtin->Type()) {
        case sem::BuiltinType::kTextureDimensions:
          switch (texture_type->dim()) {
            case ast::TextureDimension::kNone:
              TINT_ICE(Writer, diagnostics_) << "texture dimension is kNone";
              return false;
            case ast::TextureDimension::k1d:
              num_dimensions = 1;
              break;
            case ast::TextureDimension::k2d:
              num_dimensions = is_ms ? 3 : 2;
              swizzle = is_ms ? ".xy" : "";
              break;
            case ast::TextureDimension::k2dArray:
              num_dimensions = is_ms ? 4 : 3;
              swizzle = ".xy";
              break;
            case ast::TextureDimension::k3d:
              num_dimensions = 3;
              break;
            case ast::TextureDimension::kCube:
              num_dimensions = 2;
              break;
            case ast::TextureDimension::kCubeArray:
              num_dimensions = 3;
              swizzle = ".xy";
              break;
          }
          break;
        case sem::BuiltinType::kTextureNumLayers:
          switch (texture_type->dim()) {
            default:
              TINT_ICE(Writer, diagnostics_)
                  << "texture dimension is not arrayed";
              return false;
            case ast::TextureDimension::k2dArray:
              num_dimensions = is_ms ? 4 : 3;
              swizzle = ".z";
              break;
            case ast::TextureDimension::kCubeArray:
              num_dimensions = 3;
              swizzle = ".z";
              break;
          }
          break;
        case sem::BuiltinType::kTextureNumLevels:
          switch (texture_type->dim()) {
            default:
              TINT_ICE(Writer, diagnostics_)
                  << "texture dimension does not support mips";
              return false;
            case ast::TextureDimension::k1d:
              num_dimensions = 2;
              swizzle = ".y";
              break;
            case ast::TextureDimension::k2d:
            case ast::TextureDimension::kCube:
              num_dimensions = 3;
              swizzle = ".z";
              break;
            case ast::TextureDimension::k2dArray:
            case ast::TextureDimension::k3d:
            case ast::TextureDimension::kCubeArray:
              num_dimensions = 4;
              swizzle = ".w";
              break;
          }
          break;
        case sem::BuiltinType::kTextureNumSamples:
          switch (texture_type->dim()) {
            default:
              TINT_ICE(Writer, diagnostics_)
                  << "texture dimension does not support multisampling";
              return false;
            case ast::TextureDimension::k2d:
              num_dimensions = 3;
              swizzle = ".z";
              break;
            case ast::TextureDimension::k2dArray:
              num_dimensions = 4;
              swizzle = ".w";
              break;
          }
          break;
        default:
          TINT_ICE(Writer, diagnostics_) << "unexpected builtin";
          return false;
      }

      auto* level_arg = arg(Usage::kLevel);

      if (level_arg) {
        // `NumberOfLevels` is a non-optional argument if `MipLevel` was passed.
        // Increment the number of dimensions for the temporary vector to
        // accommodate this.
        num_dimensions++;

        // If the swizzle was empty, the expression will evaluate to the whole
        // vector. As we've grown the vector by one element, we now need to
        // swizzle to keep the result expression equivalent.
        if (swizzle.empty()) {
          static constexpr const char* swizzles[] = {"", ".x", ".xy", ".xyz"};
          swizzle = swizzles[num_dimensions - 1];
        }
      }

      if (num_dimensions > 4) {
        TINT_ICE(Writer, diagnostics_)
            << "Texture query builtin temporary vector has " << num_dimensions
            << " dimensions";
        return false;
      }

      // Declare a variable to hold the queried texture info
      auto dims = UniqueIdentifier(kTempNamePrefix);
      if (num_dimensions == 1) {
        line() << "int " << dims << ";";
      } else {
        line() << "int" << num_dimensions << " " << dims << ";";
      }

      {  // texture.GetDimensions(...)
        auto pre = line();
        if (!EmitExpression(pre, texture)) {
          return false;
        }
        pre << ".GetDimensions(";

        if (level_arg) {
          if (!EmitExpression(pre, level_arg)) {
            return false;
          }
          pre << ", ";
        } else if (builtin->Type() == sem::BuiltinType::kTextureNumLevels) {
          pre << "0, ";
        }

        if (num_dimensions == 1) {
          pre << dims;
        } else {
          static constexpr char xyzw[] = {'x', 'y', 'z', 'w'};
          if (num_dimensions < 0 || num_dimensions > 4) {
            TINT_ICE(Writer, diagnostics_)
                << "vector dimensions are " << num_dimensions;
            return false;
          }
          for (int i = 0; i < num_dimensions; i++) {
            if (i > 0) {
              pre << ", ";
            }
            pre << dims << "." << xyzw[i];
          }
        }

        pre << ");";
      }

      // The out parameters of the GetDimensions() call is now in temporary
      // `dims` variable. This may be packed with other data, so the final
      // expression may require a swizzle.
      out << dims << swizzle;
      return true;
    }
    default:
      break;
  }

  if (!EmitExpression(out, texture))
    return false;

  // If pack_level_in_coords is true, then the mip level will be appended as the
  // last value of the coordinates argument. If the WGSL builtin overload does
  // not have a level parameter and pack_level_in_coords is true, then a zero
  // mip level will be inserted.
  bool pack_level_in_coords = false;

  uint32_t hlsl_ret_width = 4u;

  switch (builtin->Type()) {
    case sem::BuiltinType::kTextureSample:
      out << ".Sample(";
      break;
    case sem::BuiltinType::kTextureSampleBias:
      out << ".SampleBias(";
      break;
    case sem::BuiltinType::kTextureSampleLevel:
      out << ".SampleLevel(";
      break;
    case sem::BuiltinType::kTextureSampleGrad:
      out << ".SampleGrad(";
      break;
    case sem::BuiltinType::kTextureSampleCompare:
      out << ".SampleCmp(";
      hlsl_ret_width = 1;
      break;
    case sem::BuiltinType::kTextureSampleCompareLevel:
      out << ".SampleCmpLevelZero(";
      hlsl_ret_width = 1;
      break;
    case sem::BuiltinType::kTextureLoad:
      out << ".Load(";
      // Multisampled textures do not support mip-levels.
      if (!texture_type->Is<sem::MultisampledTexture>()) {
        pack_level_in_coords = true;
      }
      break;
    case sem::BuiltinType::kTextureGather:
      out << ".Gather";
      if (builtin->Parameters()[0]->Usage() ==
          sem::ParameterUsage::kComponent) {
        switch (call->Arguments()[0]->ConstantValue().Elements()[0].i32) {
          case 0:
            out << "Red";
            break;
          case 1:
            out << "Green";
            break;
          case 2:
            out << "Blue";
            break;
          case 3:
            out << "Alpha";
            break;
        }
      }
      out << "(";
      break;
    case sem::BuiltinType::kTextureGatherCompare:
      out << ".GatherCmp(";
      break;
    case sem::BuiltinType::kTextureStore:
      out << "[";
      break;
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Internal compiler error: Unhandled texture builtin '" +
              std::string(builtin->str()) + "'");
      return false;
  }

  if (auto* sampler = arg(Usage::kSampler)) {
    if (!EmitExpression(out, sampler))
      return false;
    out << ", ";
  }

  auto* param_coords = arg(Usage::kCoords);
  if (!param_coords) {
    TINT_ICE(Writer, diagnostics_) << "missing coords argument";
    return false;
  }

  auto emit_vector_appended_with_i32_zero = [&](const ast::Expression* vector) {
    auto* i32 = builder_.create<sem::I32>();
    auto* zero = builder_.Expr(0);
    auto* stmt = builder_.Sem().Get(vector)->Stmt();
    builder_.Sem().Add(
        zero, builder_.create<sem::Expression>(zero, i32, stmt, sem::Constant{},
                                               /* has_side_effects */ false));
    auto* packed = AppendVector(&builder_, vector, zero);
    return EmitExpression(out, packed->Declaration());
  };

  auto emit_vector_appended_with_level = [&](const ast::Expression* vector) {
    if (auto* level = arg(Usage::kLevel)) {
      auto* packed = AppendVector(&builder_, vector, level);
      return EmitExpression(out, packed->Declaration());
    }
    return emit_vector_appended_with_i32_zero(vector);
  };

  if (auto* array_index = arg(Usage::kArrayIndex)) {
    // Array index needs to be appended to the coordinates.
    auto* packed = AppendVector(&builder_, param_coords, array_index);
    if (pack_level_in_coords) {
      // Then mip level needs to be appended to the coordinates.
      if (!emit_vector_appended_with_level(packed->Declaration())) {
        return false;
      }
    } else {
      if (!EmitExpression(out, packed->Declaration())) {
        return false;
      }
    }
  } else if (pack_level_in_coords) {
    // Mip level needs to be appended to the coordinates.
    if (!emit_vector_appended_with_level(param_coords)) {
      return false;
    }
  } else {
    if (!EmitExpression(out, param_coords)) {
      return false;
    }
  }

  for (auto usage : {Usage::kDepthRef, Usage::kBias, Usage::kLevel, Usage::kDdx,
                     Usage::kDdy, Usage::kSampleIndex, Usage::kOffset}) {
    if (usage == Usage::kLevel && pack_level_in_coords) {
      continue;  // mip level already packed in coordinates.
    }
    if (auto* e = arg(usage)) {
      out << ", ";
      if (!EmitExpression(out, e)) {
        return false;
      }
    }
  }

  if (builtin->Type() == sem::BuiltinType::kTextureStore) {
    out << "] = ";
    if (!EmitExpression(out, arg(Usage::kValue))) {
      return false;
    }
  } else {
    out << ")";

    // If the builtin return type does not match the number of elements of the
    // HLSL builtin, we need to swizzle the expression to generate the correct
    // number of components.
    uint32_t wgsl_ret_width = 1;
    if (auto* vec = builtin->ReturnType()->As<sem::Vector>()) {
      wgsl_ret_width = vec->Width();
    }
    if (wgsl_ret_width < hlsl_ret_width) {
      out << ".";
      for (uint32_t i = 0; i < wgsl_ret_width; i++) {
        out << "xyz"[i];
      }
    }
    if (wgsl_ret_width > hlsl_ret_width) {
      TINT_ICE(Writer, diagnostics_)
          << "WGSL return width (" << wgsl_ret_width
          << ") is wider than HLSL return width (" << hlsl_ret_width << ") for "
          << builtin->Type();
      return false;
    }
  }

  return true;
}

std::string GeneratorImpl::generate_builtin_name(const sem::Builtin* builtin) {
  switch (builtin->Type()) {
    case sem::BuiltinType::kAbs:
    case sem::BuiltinType::kAcos:
    case sem::BuiltinType::kAll:
    case sem::BuiltinType::kAny:
    case sem::BuiltinType::kAsin:
    case sem::BuiltinType::kAtan:
    case sem::BuiltinType::kAtan2:
    case sem::BuiltinType::kCeil:
    case sem::BuiltinType::kClamp:
    case sem::BuiltinType::kCos:
    case sem::BuiltinType::kCosh:
    case sem::BuiltinType::kCross:
    case sem::BuiltinType::kDeterminant:
    case sem::BuiltinType::kDistance:
    case sem::BuiltinType::kDot:
    case sem::BuiltinType::kExp:
    case sem::BuiltinType::kExp2:
    case sem::BuiltinType::kFloor:
    case sem::BuiltinType::kFrexp:
    case sem::BuiltinType::kLdexp:
    case sem::BuiltinType::kLength:
    case sem::BuiltinType::kLog:
    case sem::BuiltinType::kLog2:
    case sem::BuiltinType::kMax:
    case sem::BuiltinType::kMin:
    case sem::BuiltinType::kModf:
    case sem::BuiltinType::kNormalize:
    case sem::BuiltinType::kPow:
    case sem::BuiltinType::kReflect:
    case sem::BuiltinType::kRefract:
    case sem::BuiltinType::kRound:
    case sem::BuiltinType::kSign:
    case sem::BuiltinType::kSin:
    case sem::BuiltinType::kSinh:
    case sem::BuiltinType::kSqrt:
    case sem::BuiltinType::kStep:
    case sem::BuiltinType::kTan:
    case sem::BuiltinType::kTanh:
    case sem::BuiltinType::kTranspose:
    case sem::BuiltinType::kTrunc:
      return builtin->str();
    case sem::BuiltinType::kCountOneBits:
      return "countbits";
    case sem::BuiltinType::kDpdx:
      return "ddx";
    case sem::BuiltinType::kDpdxCoarse:
      return "ddx_coarse";
    case sem::BuiltinType::kDpdxFine:
      return "ddx_fine";
    case sem::BuiltinType::kDpdy:
      return "ddy";
    case sem::BuiltinType::kDpdyCoarse:
      return "ddy_coarse";
    case sem::BuiltinType::kDpdyFine:
      return "ddy_fine";
    case sem::BuiltinType::kFaceForward:
      return "faceforward";
    case sem::BuiltinType::kFract:
      return "frac";
    case sem::BuiltinType::kFma:
      return "mad";
    case sem::BuiltinType::kFwidth:
    case sem::BuiltinType::kFwidthCoarse:
    case sem::BuiltinType::kFwidthFine:
      return "fwidth";
    case sem::BuiltinType::kInverseSqrt:
      return "rsqrt";
    case sem::BuiltinType::kMix:
      return "lerp";
    case sem::BuiltinType::kReverseBits:
      return "reversebits";
    case sem::BuiltinType::kSmoothstep:
    case sem::BuiltinType::kSmoothStep:
      return "smoothstep";
    default:
      diagnostics_.add_error(
          diag::System::Writer,
          "Unknown builtin method: " + std::string(builtin->str()));
  }

  return "";
}

bool GeneratorImpl::EmitCase(const ast::SwitchStatement* s, size_t case_idx) {
  auto* stmt = s->body[case_idx];
  if (stmt->IsDefault()) {
    line() << "default: {";
  } else {
    for (auto* selector : stmt->selectors) {
      auto out = line();
      out << "case ";
      if (!EmitLiteral(out, selector)) {
        return false;
      }
      out << ":";
      if (selector == stmt->selectors.back()) {
        out << " {";
      }
    }
  }

  increment_indent();
  TINT_DEFER({
    decrement_indent();
    line() << "}";
  });

  // Emit the case statement
  if (!EmitStatements(stmt->body->statements)) {
    return false;
  }

  // Inline all fallthrough case statements. FXC cannot handle fallthroughs.
  while (tint::Is<ast::FallthroughStatement>(stmt->body->Last())) {
    case_idx++;
    stmt = s->body[case_idx];
    // Generate each fallthrough case statement in a new block. This is done to
    // prevent symbol collision of variables declared in these cases statements.
    if (!EmitBlock(stmt->body)) {
      return false;
    }
  }

  if (!tint::IsAnyOf<ast::BreakStatement, ast::FallthroughStatement>(
          stmt->body->Last())) {
    line() << "break;";
  }

  return true;
}

bool GeneratorImpl::EmitContinue(const ast::ContinueStatement*) {
  if (!emit_continuing_()) {
    return false;
  }
  line() << "continue;";
  return true;
}

bool GeneratorImpl::EmitDiscard(const ast::DiscardStatement*) {
  // TODO(dsinclair): Verify this is correct when the discard semantics are
  // defined for WGSL (https://github.com/gpuweb/gpuweb/issues/361)
  line() << "discard;";
  return true;
}

bool GeneratorImpl::EmitExpression(std::ostream& out,
                                   const ast::Expression* expr) {
  return Switch(
      expr,
      [&](const ast::IndexAccessorExpression* a) {  //
        return EmitIndexAccessor(out, a);
      },
      [&](const ast::BinaryExpression* b) {  //
        return EmitBinary(out, b);
      },
      [&](const ast::BitcastExpression* b) {  //
        return EmitBitcast(out, b);
      },
      [&](const ast::CallExpression* c) {  //
        return EmitCall(out, c);
      },
      [&](const ast::IdentifierExpression* i) {  //
        return EmitIdentifier(out, i);
      },
      [&](const ast::LiteralExpression* l) {  //
        return EmitLiteral(out, l);
      },
      [&](const ast::MemberAccessorExpression* m) {  //
        return EmitMemberAccessor(out, m);
      },
      [&](const ast::UnaryOpExpression* u) {  //
        return EmitUnaryOp(out, u);
      },
      [&](Default) {  //
        diagnostics_.add_error(
            diag::System::Writer,
            "unknown expression type: " + std::string(expr->TypeInfo().name));
        return false;
      });
}

bool GeneratorImpl::EmitIdentifier(std::ostream& out,
                                   const ast::IdentifierExpression* expr) {
  out << builder_.Symbols().NameFor(expr->symbol);
  return true;
}

bool GeneratorImpl::EmitIf(const ast::IfStatement* stmt) {
  {
    auto out = line();
    out << "if (";
    if (!EmitExpression(out, stmt->condition)) {
      return false;
    }
    out << ") {";
  }

  if (!EmitStatementsWithIndent(stmt->body->statements)) {
    return false;
  }

  for (auto* e : stmt->else_statements) {
    if (e->condition) {
      line() << "} else {";
      increment_indent();

      {
        auto out = line();
        out << "if (";
        if (!EmitExpression(out, e->condition)) {
          return false;
        }
        out << ") {";
      }
    } else {
      line() << "} else {";
    }

    if (!EmitStatementsWithIndent(e->body->statements)) {
      return false;
    }
  }

  line() << "}";

  for (auto* e : stmt->else_statements) {
    if (e->condition) {
      decrement_indent();
      line() << "}";
    }
  }
  return true;
}

bool GeneratorImpl::EmitFunction(const ast::Function* func) {
  auto* sem = builder_.Sem().Get(func);

  if (ast::HasAttribute<ast::InternalAttribute>(func->attributes)) {
    // An internal function. Do not emit.
    return true;
  }

  {
    auto out = line();
    auto name = builder_.Symbols().NameFor(func->symbol);
    // If the function returns an array, then we need to declare a typedef for
    // this.
    if (sem->ReturnType()->Is<sem::Array>()) {
      auto typedef_name = UniqueIdentifier(name + "_ret");
      auto pre = line();
      pre << "typedef ";
      if (!EmitTypeAndName(pre, sem->ReturnType(), ast::StorageClass::kNone,
                           ast::Access::kReadWrite, typedef_name)) {
        return false;
      }
      pre << ";";
      out << typedef_name;
    } else {
      if (!EmitType(out, sem->ReturnType(), ast::StorageClass::kNone,
                    ast::Access::kReadWrite, "")) {
        return false;
      }
    }

    out << " " << name << "(";

    bool first = true;

    for (auto* v : sem->Parameters()) {
      if (!first) {
        out << ", ";
      }
      first = false;

      auto const* type = v->Type();

      if (auto* ptr = type->As<sem::Pointer>()) {
        // Transform pointer parameters in to `inout` parameters.
        // The WGSL spec is highly restrictive in what can be passed in pointer
        // parameters, which allows for this transformation. See:
        // https://gpuweb.github.io/gpuweb/wgsl/#function-restriction
        out << "inout ";
        type = ptr->StoreType();
      }

      // Note: WGSL only allows for StorageClass::kNone on parameters, however
      // the sanitizer transforms generates load / store functions for storage
      // or uniform buffers. These functions have a buffer parameter with
      // StorageClass::kStorage or StorageClass::kUniform. This is required to
      // correctly translate the parameter to a [RW]ByteAddressBuffer for
      // storage buffers and a uint4[N] for uniform buffers.
      if (!EmitTypeAndName(
              out, type, v->StorageClass(), v->Access(),
              builder_.Symbols().NameFor(v->Declaration()->symbol))) {
        return false;
      }
    }
    out << ") {";
  }

  if (sem->HasDiscard() && !sem->ReturnType()->Is<sem::Void>()) {
    // BUG(crbug.com/tint/1081): work around non-void functions with discard
    // failing compilation sometimes
    if (!EmitFunctionBodyWithDiscard(func)) {
      return false;
    }
  } else {
    if (!EmitStatementsWithIndent(func->body->statements)) {
      return false;
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitFunctionBodyWithDiscard(const ast::Function* func) {
  // FXC sometimes fails to compile functions that discard with 'Not all control
  // paths return a value'. We work around this by wrapping the function body
  // within an "if (true) { <body> } return <default return type obj>;" so that
  // there is always an (unused) return statement.

  auto* sem = builder_.Sem().Get(func);
  TINT_ASSERT(Writer, sem->HasDiscard() && !sem->ReturnType()->Is<sem::Void>());

  ScopedIndent si(this);
  line() << "if (true) {";

  if (!EmitStatementsWithIndent(func->body->statements)) {
    return false;
  }

  line() << "}";

  // Return an unused result that matches the type of the return value
  auto name = builder_.Symbols().NameFor(builder_.Symbols().New("unused"));
  {
    auto out = line();
    if (!EmitTypeAndName(out, sem->ReturnType(), ast::StorageClass::kNone,
                         ast::Access::kReadWrite, name)) {
      return false;
    }
    out << ";";
  }
  line() << "return " << name << ";";

  return true;
}

bool GeneratorImpl::EmitGlobalVariable(const ast::Variable* global) {
  if (global->is_const) {
    return EmitProgramConstVariable(global);
  }

  auto* sem = builder_.Sem().Get(global);
  switch (sem->StorageClass()) {
    case ast::StorageClass::kUniform:
      return EmitUniformVariable(sem);
    case ast::StorageClass::kStorage:
      return EmitStorageVariable(sem);
    case ast::StorageClass::kHandle:
      return EmitHandleVariable(sem);
    case ast::StorageClass::kPrivate:
      return EmitPrivateVariable(sem);
    case ast::StorageClass::kWorkgroup:
      return EmitWorkgroupVariable(sem);
    default:
      break;
  }

  TINT_ICE(Writer, diagnostics_)
      << "unhandled storage class " << sem->StorageClass();
  return false;
}

bool GeneratorImpl::EmitUniformVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto binding_point = decl->BindingPoint();
  auto* type = var->Type()->UnwrapRef();
  auto name = builder_.Symbols().NameFor(decl->symbol);
  line() << "cbuffer cbuffer_" << name << RegisterAndSpace('b', binding_point)
         << " {";

  {
    ScopedIndent si(this);
    auto out = line();
    if (!EmitTypeAndName(out, type, ast::StorageClass::kUniform, var->Access(),
                         name)) {
      return false;
    }
    out << ";";
  }

  line() << "};";

  return true;
}

bool GeneratorImpl::EmitStorageVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto* type = var->Type()->UnwrapRef();
  auto out = line();
  if (!EmitTypeAndName(out, type, ast::StorageClass::kStorage, var->Access(),
                       builder_.Symbols().NameFor(decl->symbol))) {
    return false;
  }

  out << RegisterAndSpace(var->Access() == ast::Access::kRead ? 't' : 'u',
                          decl->BindingPoint())
      << ";";

  return true;
}

bool GeneratorImpl::EmitHandleVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto* unwrapped_type = var->Type()->UnwrapRef();
  auto out = line();

  auto name = builder_.Symbols().NameFor(decl->symbol);
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  const char* register_space = nullptr;

  if (unwrapped_type->Is<sem::Texture>()) {
    register_space = "t";
    if (unwrapped_type->Is<sem::StorageTexture>()) {
      register_space = "u";
    }
  } else if (unwrapped_type->Is<sem::Sampler>()) {
    register_space = "s";
  }

  if (register_space) {
    auto bp = decl->BindingPoint();
    out << " : register(" << register_space << bp.binding->value << ", space"
        << bp.group->value << ")";
  }

  out << ";";
  return true;
}

bool GeneratorImpl::EmitPrivateVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto out = line();

  out << "static ";

  auto name = builder_.Symbols().NameFor(decl->symbol);
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  out << " = ";
  if (auto* constructor = decl->constructor) {
    if (!EmitExpression(out, constructor)) {
      return false;
    }
  } else {
    if (!EmitZeroValue(out, var->Type()->UnwrapRef())) {
      return false;
    }
  }

  out << ";";
  return true;
}

bool GeneratorImpl::EmitWorkgroupVariable(const sem::Variable* var) {
  auto* decl = var->Declaration();
  auto out = line();

  out << "groupshared ";

  auto name = builder_.Symbols().NameFor(decl->symbol);
  auto* type = var->Type()->UnwrapRef();
  if (!EmitTypeAndName(out, type, var->StorageClass(), var->Access(), name)) {
    return false;
  }

  if (auto* constructor = decl->constructor) {
    out << " = ";
    if (!EmitExpression(out, constructor)) {
      return false;
    }
  }

  out << ";";
  return true;
}

std::string GeneratorImpl::builtin_to_attribute(ast::Builtin builtin) const {
  switch (builtin) {
    case ast::Builtin::kPosition:
      return "SV_Position";
    case ast::Builtin::kVertexIndex:
      return "SV_VertexID";
    case ast::Builtin::kInstanceIndex:
      return "SV_InstanceID";
    case ast::Builtin::kFrontFacing:
      return "SV_IsFrontFace";
    case ast::Builtin::kFragDepth:
      return "SV_Depth";
    case ast::Builtin::kLocalInvocationId:
      return "SV_GroupThreadID";
    case ast::Builtin::kLocalInvocationIndex:
      return "SV_GroupIndex";
    case ast::Builtin::kGlobalInvocationId:
      return "SV_DispatchThreadID";
    case ast::Builtin::kWorkgroupId:
      return "SV_GroupID";
    case ast::Builtin::kSampleIndex:
      return "SV_SampleIndex";
    case ast::Builtin::kSampleMask:
      return "SV_Coverage";
    default:
      break;
  }
  return "";
}

std::string GeneratorImpl::interpolation_to_modifiers(
    ast::InterpolationType type,
    ast::InterpolationSampling sampling) const {
  std::string modifiers;
  switch (type) {
    case ast::InterpolationType::kPerspective:
      modifiers += "linear ";
      break;
    case ast::InterpolationType::kLinear:
      modifiers += "noperspective ";
      break;
    case ast::InterpolationType::kFlat:
      modifiers += "nointerpolation ";
      break;
  }
  switch (sampling) {
    case ast::InterpolationSampling::kCentroid:
      modifiers += "centroid ";
      break;
    case ast::InterpolationSampling::kSample:
      modifiers += "sample ";
      break;
    case ast::InterpolationSampling::kCenter:
    case ast::InterpolationSampling::kNone:
      break;
  }
  return modifiers;
}

bool GeneratorImpl::EmitEntryPointFunction(const ast::Function* func) {
  auto* func_sem = builder_.Sem().Get(func);

  {
    auto out = line();
    if (func->PipelineStage() == ast::PipelineStage::kCompute) {
      // Emit the workgroup_size attribute.
      auto wgsize = func_sem->WorkgroupSize();
      out << "[numthreads(";
      for (int i = 0; i < 3; i++) {
        if (i > 0) {
          out << ", ";
        }

        if (wgsize[i].overridable_const) {
          auto* global = builder_.Sem().Get<sem::GlobalVariable>(
              wgsize[i].overridable_const);
          if (!global->IsOverridable()) {
            TINT_ICE(Writer, builder_.Diagnostics())
                << "expected a pipeline-overridable constant";
          }
          out << kSpecConstantPrefix << global->ConstantId();
        } else {
          out << std::to_string(wgsize[i].value);
        }
      }
      out << ")]" << std::endl;
    }

    out << func->return_type->FriendlyName(builder_.Symbols());

    out << " " << builder_.Symbols().NameFor(func->symbol) << "(";

    bool first = true;

    // Emit entry point parameters.
    for (auto* var : func->params) {
      auto* sem = builder_.Sem().Get(var);
      auto* type = sem->Type();
      if (!type->Is<sem::Struct>()) {
        // ICE likely indicates that the CanonicalizeEntryPointIO transform was
        // not run, or a builtin parameter was added after it was run.
        TINT_ICE(Writer, diagnostics_)
            << "Unsupported non-struct entry point parameter";
      }

      if (!first) {
        out << ", ";
      }
      first = false;

      if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                           builder_.Symbols().NameFor(var->symbol))) {
        return false;
      }
    }

    out << ") {";
  }

  {
    ScopedIndent si(this);

    if (!EmitStatements(func->body->statements)) {
      return false;
    }

    if (!Is<ast::ReturnStatement>(func->body->Last())) {
      ast::ReturnStatement ret(ProgramID(), Source{});
      if (!EmitStatement(&ret)) {
        return false;
      }
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitLiteral(std::ostream& out,
                                const ast::LiteralExpression* lit) {
  return Switch(
      lit,
      [&](const ast::BoolLiteralExpression* l) {
        out << (l->value ? "true" : "false");
        return true;
      },
      [&](const ast::FloatLiteralExpression* fl) {
        if (std::isinf(fl->value)) {
          out << (fl->value >= 0 ? "asfloat(0x7f800000u)"
                                 : "asfloat(0xff800000u)");
        } else if (std::isnan(fl->value)) {
          out << "asfloat(0x7fc00000u)";
        } else {
          out << FloatToString(fl->value) << "f";
        }
        return true;
      },
      [&](const ast::SintLiteralExpression* sl) {
        out << sl->value;
        return true;
      },
      [&](const ast::UintLiteralExpression* ul) {
        out << ul->value << "u";
        return true;
      },
      [&](Default) {
        diagnostics_.add_error(diag::System::Writer, "unknown literal type");
        return false;
      });
}

bool GeneratorImpl::EmitValue(std::ostream& out,
                              const sem::Type* type,
                              int value) {
  return Switch(
      type,
      [&](const sem::Bool*) {
        out << (value == 0 ? "false" : "true");
        return true;
      },
      [&](const sem::F32*) {
        out << value << ".0f";
        return true;
      },
      [&](const sem::I32*) {
        out << value;
        return true;
      },
      [&](const sem::U32*) {
        out << value << "u";
        return true;
      },
      [&](const sem::Vector* vec) {
        if (!EmitType(out, type, ast::StorageClass::kNone,
                      ast::Access::kReadWrite, "")) {
          return false;
        }
        ScopedParen sp(out);
        for (uint32_t i = 0; i < vec->Width(); i++) {
          if (i != 0) {
            out << ", ";
          }
          if (!EmitValue(out, vec->type(), value)) {
            return false;
          }
        }
        return true;
      },
      [&](const sem::Matrix* mat) {
        if (!EmitType(out, type, ast::StorageClass::kNone,
                      ast::Access::kReadWrite, "")) {
          return false;
        }
        ScopedParen sp(out);
        for (uint32_t i = 0; i < (mat->rows() * mat->columns()); i++) {
          if (i != 0) {
            out << ", ";
          }
          if (!EmitValue(out, mat->type(), value)) {
            return false;
          }
        }
        return true;
      },
      [&](const sem::Struct*) {
        out << "(";
        TINT_DEFER(out << ")" << value);
        return EmitType(out, type, ast::StorageClass::kNone,
                        ast::Access::kUndefined, "");
      },
      [&](const sem::Array*) {
        out << "(";
        TINT_DEFER(out << ")" << value);
        return EmitType(out, type, ast::StorageClass::kNone,
                        ast::Access::kUndefined, "");
      },
      [&](Default) {
        diagnostics_.add_error(diag::System::Writer,
                               "Invalid type for value emission: " +
                                   type->FriendlyName(builder_.Symbols()));
        return false;
      });
}

bool GeneratorImpl::EmitZeroValue(std::ostream& out, const sem::Type* type) {
  return EmitValue(out, type, 0);
}

bool GeneratorImpl::EmitLoop(const ast::LoopStatement* stmt) {
  auto emit_continuing = [this, stmt]() {
    if (stmt->continuing && !stmt->continuing->Empty()) {
      if (!EmitBlock(stmt->continuing)) {
        return false;
      }
    }
    return true;
  };

  TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
  line() << LoopAttribute() << "while (true) {";
  {
    ScopedIndent si(this);
    if (!EmitStatements(stmt->body->statements)) {
      return false;
    }
    if (!emit_continuing_()) {
      return false;
    }
  }
  line() << "}";

  return true;
}

bool GeneratorImpl::EmitForLoop(const ast::ForLoopStatement* stmt) {
  // Nest a for loop with a new block. In HLSL the initializer scope is not
  // nested by the for-loop, so we may get variable redefinitions.
  line() << "{";
  increment_indent();
  TINT_DEFER({
    decrement_indent();
    line() << "}";
  });

  TextBuffer init_buf;
  if (auto* init = stmt->initializer) {
    TINT_SCOPED_ASSIGNMENT(current_buffer_, &init_buf);
    if (!EmitStatement(init)) {
      return false;
    }
  }

  TextBuffer cond_pre;
  std::stringstream cond_buf;
  if (auto* cond = stmt->condition) {
    TINT_SCOPED_ASSIGNMENT(current_buffer_, &cond_pre);
    if (!EmitExpression(cond_buf, cond)) {
      return false;
    }
  }

  TextBuffer cont_buf;
  if (auto* cont = stmt->continuing) {
    TINT_SCOPED_ASSIGNMENT(current_buffer_, &cont_buf);
    if (!EmitStatement(cont)) {
      return false;
    }
  }

  // If the for-loop has a multi-statement conditional and / or continuing, then
  // we cannot emit this as a regular for-loop in HLSL. Instead we need to
  // generate a `while(true)` loop.
  bool emit_as_loop = cond_pre.lines.size() > 0 || cont_buf.lines.size() > 1;

  // If the for-loop has multi-statement initializer, or is going to be emitted
  // as a `while(true)` loop, then declare the initializer statement(s) before
  // the loop.
  if (init_buf.lines.size() > 1 || (stmt->initializer && emit_as_loop)) {
    current_buffer_->Append(init_buf);
    init_buf.lines.clear();  // Don't emit the initializer again in the 'for'
  }

  if (emit_as_loop) {
    auto emit_continuing = [&]() {
      current_buffer_->Append(cont_buf);
      return true;
    };

    TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
    line() << LoopAttribute() << "while (true) {";
    increment_indent();
    TINT_DEFER({
      decrement_indent();
      line() << "}";
    });

    if (stmt->condition) {
      current_buffer_->Append(cond_pre);
      line() << "if (!(" << cond_buf.str() << ")) { break; }";
    }

    if (!EmitStatements(stmt->body->statements)) {
      return false;
    }

    if (!emit_continuing_()) {
      return false;
    }
  } else {
    // For-loop can be generated.
    {
      auto out = line();
      out << LoopAttribute() << "for";
      {
        ScopedParen sp(out);

        if (!init_buf.lines.empty()) {
          out << init_buf.lines[0].content << " ";
        } else {
          out << "; ";
        }

        out << cond_buf.str() << "; ";

        if (!cont_buf.lines.empty()) {
          out << TrimSuffix(cont_buf.lines[0].content, ";");
        }
      }
      out << " {";
    }
    {
      auto emit_continuing = [] { return true; };
      TINT_SCOPED_ASSIGNMENT(emit_continuing_, emit_continuing);
      if (!EmitStatementsWithIndent(stmt->body->statements)) {
        return false;
      }
    }
    line() << "}";
  }

  return true;
}

bool GeneratorImpl::EmitMemberAccessor(
    std::ostream& out,
    const ast::MemberAccessorExpression* expr) {
  if (!EmitExpression(out, expr->structure)) {
    return false;
  }
  out << ".";

  // Swizzles output the name directly
  if (builder_.Sem().Get(expr)->Is<sem::Swizzle>()) {
    out << builder_.Symbols().NameFor(expr->member->symbol);
  } else if (!EmitExpression(out, expr->member)) {
    return false;
  }

  return true;
}

bool GeneratorImpl::EmitReturn(const ast::ReturnStatement* stmt) {
  if (stmt->value) {
    auto out = line();
    out << "return ";
    if (!EmitExpression(out, stmt->value)) {
      return false;
    }
    out << ";";
  } else {
    line() << "return;";
  }
  return true;
}

bool GeneratorImpl::EmitStatement(const ast::Statement* stmt) {
  return Switch(
      stmt,
      [&](const ast::AssignmentStatement* a) {  //
        return EmitAssign(a);
      },
      [&](const ast::BlockStatement* b) {  //
        return EmitBlock(b);
      },
      [&](const ast::BreakStatement* b) {  //
        return EmitBreak(b);
      },
      [&](const ast::CallStatement* c) {  //
        auto out = line();
        if (!EmitCall(out, c->expr)) {
          return false;
        }
        out << ";";
        return true;
      },
      [&](const ast::ContinueStatement* c) {  //
        return EmitContinue(c);
      },
      [&](const ast::DiscardStatement* d) {  //
        return EmitDiscard(d);
      },
      [&](const ast::FallthroughStatement*) {  //
        line() << "/* fallthrough */";
        return true;
      },
      [&](const ast::IfStatement* i) {  //
        return EmitIf(i);
      },
      [&](const ast::LoopStatement* l) {  //
        return EmitLoop(l);
      },
      [&](const ast::ForLoopStatement* l) {  //
        return EmitForLoop(l);
      },
      [&](const ast::ReturnStatement* r) {  //
        return EmitReturn(r);
      },
      [&](const ast::SwitchStatement* s) {  //
        return EmitSwitch(s);
      },
      [&](const ast::VariableDeclStatement* v) {  //
        return EmitVariable(v->variable);
      },
      [&](Default) {  //
        diagnostics_.add_error(
            diag::System::Writer,
            "unknown statement type: " + std::string(stmt->TypeInfo().name));
        return false;
      });
}

bool GeneratorImpl::EmitDefaultOnlySwitch(const ast::SwitchStatement* stmt) {
  TINT_ASSERT(Writer, stmt->body.size() == 1 && stmt->body[0]->IsDefault());

  // FXC fails to compile a switch with just a default case, ignoring the
  // default case body. We work around this here by emitting the default case
  // without the switch.

  // Emit the switch condition as-is in case it has side-effects (e.g.
  // function call). Note that's it's fine not to assign the result of the
  // expression.
  {
    auto out = line();
    if (!EmitExpression(out, stmt->condition)) {
      return false;
    }
    out << ";";
  }

  // Emit "do { <default case body> } while(false);". We use a 'do' loop so
  // that break statements work as expected, and make it 'while (false)' in
  // case there isn't a break statement.
  line() << "do {";
  {
    ScopedIndent si(this);
    if (!EmitStatements(stmt->body[0]->body->statements)) {
      return false;
    }
  }
  line() << "} while (false);";
  return true;
}

bool GeneratorImpl::EmitSwitch(const ast::SwitchStatement* stmt) {
  // BUG(crbug.com/tint/1188): work around default-only switches
  if (stmt->body.size() == 1 && stmt->body[0]->IsDefault()) {
    return EmitDefaultOnlySwitch(stmt);
  }

  {  // switch(expr) {
    auto out = line();
    out << "switch(";
    if (!EmitExpression(out, stmt->condition)) {
      return false;
    }
    out << ") {";
  }

  {
    ScopedIndent si(this);
    for (size_t i = 0; i < stmt->body.size(); i++) {
      if (!EmitCase(stmt, i)) {
        return false;
      }
    }
  }

  line() << "}";

  return true;
}

bool GeneratorImpl::EmitType(std::ostream& out,
                             const sem::Type* type,
                             ast::StorageClass storage_class,
                             ast::Access access,
                             const std::string& name,
                             bool* name_printed /* = nullptr */) {
  if (name_printed) {
    *name_printed = false;
  }
  switch (storage_class) {
    case ast::StorageClass::kStorage:
      if (access != ast::Access::kRead) {
        out << "RW";
      }
      out << "ByteAddressBuffer";
      return true;
    case ast::StorageClass::kUniform: {
      auto array_length = (type->Size() + 15) / 16;
      out << "uint4 " << name << "[" << array_length << "]";
      if (name_printed) {
        *name_printed = true;
      }
      return true;
    }
    default:
      break;
  }

  return Switch(
      type,
      [&](const sem::Array* ary) {
        const sem::Type* base_type = ary;
        std::vector<uint32_t> sizes;
        while (auto* arr = base_type->As<sem::Array>()) {
          if (arr->IsRuntimeSized()) {
            TINT_ICE(Writer, diagnostics_)
                << "Runtime arrays may only exist in storage buffers, which "
                   "should "
                   "have been transformed into a ByteAddressBuffer";
            return false;
          }
          sizes.push_back(arr->Count());
          base_type = arr->ElemType();
        }
        if (!EmitType(out, base_type, storage_class, access, "")) {
          return false;
        }
        if (!name.empty()) {
          out << " " << name;
          if (name_printed) {
            *name_printed = true;
          }
        }
        for (uint32_t size : sizes) {
          out << "[" << size << "]";
        }
        return true;
      },
      [&](const sem::Bool*) {
        out << "bool";
        return true;
      },
      [&](const sem::F32*) {
        out << "float";
        return true;
      },
      [&](const sem::I32*) {
        out << "int";
        return true;
      },
      [&](const sem::Matrix* mat) {
        if (!EmitType(out, mat->type(), storage_class, access, "")) {
          return false;
        }
        // Note: HLSL's matrices are declared as <type>NxM, where N is the
        // number of rows and M is the number of columns. Despite HLSL's
        // matrices being column-major by default, the index operator and
        // constructors actually operate on row-vectors, where as WGSL operates
        // on column vectors. To simplify everything we use the transpose of the
        // matrices. See:
        // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-per-component-math#matrix-ordering
        out << mat->columns() << "x" << mat->rows();
        return true;
      },
      [&](const sem::Pointer*) {
        TINT_ICE(Writer, diagnostics_)
            << "Attempting to emit pointer type. These should have been "
               "removed with the InlinePointerLets transform";
        return false;
      },
      [&](const sem::Sampler* sampler) {
        out << "Sampler";
        if (sampler->IsComparison()) {
          out << "Comparison";
        }
        out << "State";
        return true;
      },
      [&](const sem::Struct* str) {
        out << StructName(str);
        return true;
      },
      [&](const sem::Texture* tex) {
        if (tex->Is<sem::ExternalTexture>()) {
          TINT_ICE(Writer, diagnostics_)
              << "Multiplanar external texture transform was not run.";
          return false;
        }

        auto* storage = tex->As<sem::StorageTexture>();
        auto* ms = tex->As<sem::MultisampledTexture>();
        auto* depth_ms = tex->As<sem::DepthMultisampledTexture>();
        auto* sampled = tex->As<sem::SampledTexture>();

        if (storage && storage->access() != ast::Access::kRead) {
          out << "RW";
        }
        out << "Texture";

        switch (tex->dim()) {
          case ast::TextureDimension::k1d:
            out << "1D";
            break;
          case ast::TextureDimension::k2d:
            out << ((ms || depth_ms) ? "2DMS" : "2D");
            break;
          case ast::TextureDimension::k2dArray:
            out << ((ms || depth_ms) ? "2DMSArray" : "2DArray");
            break;
          case ast::TextureDimension::k3d:
            out << "3D";
            break;
          case ast::TextureDimension::kCube:
            out << "Cube";
            break;
          case ast::TextureDimension::kCubeArray:
            out << "CubeArray";
            break;
          default:
            TINT_UNREACHABLE(Writer, diagnostics_)
                << "unexpected TextureDimension " << tex->dim();
            return false;
        }

        if (storage) {
          auto* component =
              image_format_to_rwtexture_type(storage->texel_format());
          if (component == nullptr) {
            TINT_ICE(Writer, diagnostics_)
                << "Unsupported StorageTexture TexelFormat: "
                << static_cast<int>(storage->texel_format());
            return false;
          }
          out << "<" << component << ">";
        } else if (depth_ms) {
          out << "<float4>";
        } else if (sampled || ms) {
          auto* subtype = sampled ? sampled->type() : ms->type();
          out << "<";
          if (subtype->Is<sem::F32>()) {
            out << "float4";
          } else if (subtype->Is<sem::I32>()) {
            out << "int4";
          } else if (subtype->Is<sem::U32>()) {
            out << "uint4";
          } else {
            TINT_ICE(Writer, diagnostics_)
                << "Unsupported multisampled texture type";
            return false;
          }
          out << ">";
        }
        return true;
      },
      [&](const sem::U32*) {
        out << "uint";
        return true;
      },
      [&](const sem::Vector* vec) {
        auto width = vec->Width();
        if (vec->type()->Is<sem::F32>() && width >= 1 && width <= 4) {
          out << "float" << width;
        } else if (vec->type()->Is<sem::I32>() && width >= 1 && width <= 4) {
          out << "int" << width;
        } else if (vec->type()->Is<sem::U32>() && width >= 1 && width <= 4) {
          out << "uint" << width;
        } else if (vec->type()->Is<sem::Bool>() && width >= 1 && width <= 4) {
          out << "bool" << width;
        } else {
          out << "vector<";
          if (!EmitType(out, vec->type(), storage_class, access, "")) {
            return false;
          }
          out << ", " << width << ">";
        }
        return true;
      },
      [&](const sem::Atomic* atomic) {
        return EmitType(out, atomic->Type(), storage_class, access, name);
      },
      [&](const sem::Void*) {
        out << "void";
        return true;
      },
      [&](Default) {
        diagnostics_.add_error(diag::System::Writer,
                               "unknown type in EmitType");
        return false;
      });
}

bool GeneratorImpl::EmitTypeAndName(std::ostream& out,
                                    const sem::Type* type,
                                    ast::StorageClass storage_class,
                                    ast::Access access,
                                    const std::string& name) {
  bool name_printed = false;
  if (!EmitType(out, type, storage_class, access, name, &name_printed)) {
    return false;
  }
  if (!name.empty() && !name_printed) {
    out << " " << name;
  }
  return true;
}

bool GeneratorImpl::EmitStructType(TextBuffer* b, const sem::Struct* str) {
  line(b) << "struct " << StructName(str) << " {";
  {
    ScopedIndent si(b);
    for (auto* mem : str->Members()) {
      auto mem_name = builder_.Symbols().NameFor(mem->Name());

      auto* ty = mem->Type();

      auto out = line(b);

      std::string pre, post;

      if (auto* decl = mem->Declaration()) {
        for (auto* attr : decl->attributes) {
          if (auto* location = attr->As<ast::LocationAttribute>()) {
            auto& pipeline_stage_uses = str->PipelineStageUses();
            if (pipeline_stage_uses.size() != 1) {
              TINT_ICE(Writer, diagnostics_)
                  << "invalid entry point IO struct uses";
            }

            if (pipeline_stage_uses.count(
                    sem::PipelineStageUsage::kVertexInput)) {
              post += " : TEXCOORD" + std::to_string(location->value);
            } else if (pipeline_stage_uses.count(
                           sem::PipelineStageUsage::kVertexOutput)) {
              post += " : TEXCOORD" + std::to_string(location->value);
            } else if (pipeline_stage_uses.count(
                           sem::PipelineStageUsage::kFragmentInput)) {
              post += " : TEXCOORD" + std::to_string(location->value);
            } else if (pipeline_stage_uses.count(
                           sem::PipelineStageUsage::kFragmentOutput)) {
              post += " : SV_Target" + std::to_string(location->value);
            } else {
              TINT_ICE(Writer, diagnostics_)
                  << "invalid use of location attribute";
            }
          } else if (auto* builtin = attr->As<ast::BuiltinAttribute>()) {
            auto name = builtin_to_attribute(builtin->builtin);
            if (name.empty()) {
              diagnostics_.add_error(diag::System::Writer,
                                     "unsupported builtin");
              return false;
            }
            post += " : " + name;
          } else if (auto* interpolate =
                         attr->As<ast::InterpolateAttribute>()) {
            auto mod = interpolation_to_modifiers(interpolate->type,
                                                  interpolate->sampling);
            if (mod.empty()) {
              diagnostics_.add_error(diag::System::Writer,
                                     "unsupported interpolation");
              return false;
            }
            pre += mod;

          } else if (attr->Is<ast::InvariantAttribute>()) {
            // Note: `precise` is not exactly the same as `invariant`, but is
            // stricter and therefore provides the necessary guarantees.
            // See discussion here: https://github.com/gpuweb/gpuweb/issues/893
            pre += "precise ";
          } else if (!attr->IsAnyOf<ast::StructMemberAlignAttribute,
                                    ast::StructMemberOffsetAttribute,
                                    ast::StructMemberSizeAttribute>()) {
            TINT_ICE(Writer, diagnostics_)
                << "unhandled struct member attribute: " << attr->Name();
            return false;
          }
        }
      }

      out << pre;
      if (!EmitTypeAndName(out, ty, ast::StorageClass::kNone,
                           ast::Access::kReadWrite, mem_name)) {
        return false;
      }
      out << post << ";";
    }
  }

  line(b) << "};";

  return true;
}

bool GeneratorImpl::EmitUnaryOp(std::ostream& out,
                                const ast::UnaryOpExpression* expr) {
  switch (expr->op) {
    case ast::UnaryOp::kIndirection:
    case ast::UnaryOp::kAddressOf:
      return EmitExpression(out, expr->expr);
    case ast::UnaryOp::kComplement:
      out << "~";
      break;
    case ast::UnaryOp::kNot:
      out << "!";
      break;
    case ast::UnaryOp::kNegation:
      out << "-";
      break;
  }
  out << "(";

  if (!EmitExpression(out, expr->expr)) {
    return false;
  }

  out << ")";

  return true;
}

bool GeneratorImpl::EmitVariable(const ast::Variable* var) {
  auto* sem = builder_.Sem().Get(var);
  auto* type = sem->Type()->UnwrapRef();

  // TODO(dsinclair): Handle variable attributes
  if (!var->attributes.empty()) {
    diagnostics_.add_error(diag::System::Writer,
                           "Variable attributes are not handled yet");
    return false;
  }

  auto out = line();
  if (var->is_const) {
    out << "const ";
  }
  if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                       builder_.Symbols().NameFor(var->symbol))) {
    return false;
  }

  out << " = ";

  if (var->constructor) {
    if (!EmitExpression(out, var->constructor)) {
      return false;
    }
  } else {
    if (!EmitZeroValue(out, type)) {
      return false;
    }
  }
  out << ";";

  return true;
}

bool GeneratorImpl::EmitProgramConstVariable(const ast::Variable* var) {
  for (auto* d : var->attributes) {
    if (!d->Is<ast::IdAttribute>()) {
      diagnostics_.add_error(diag::System::Writer,
                             "Decorated const values not valid");
      return false;
    }
  }
  if (!var->is_const) {
    diagnostics_.add_error(diag::System::Writer, "Expected a const value");
    return false;
  }

  auto* sem = builder_.Sem().Get(var);
  auto* type = sem->Type();

  auto* global = sem->As<sem::GlobalVariable>();
  if (global && global->IsOverridable()) {
    auto const_id = global->ConstantId();

    line() << "#ifndef " << kSpecConstantPrefix << const_id;

    if (var->constructor != nullptr) {
      auto out = line();
      out << "#define " << kSpecConstantPrefix << const_id << " ";
      if (!EmitExpression(out, var->constructor)) {
        return false;
      }
    } else {
      line() << "#error spec constant required for constant id " << const_id;
    }
    line() << "#endif";
    {
      auto out = line();
      out << "static const ";
      if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                           builder_.Symbols().NameFor(var->symbol))) {
        return false;
      }
      out << " = " << kSpecConstantPrefix << const_id << ";";
    }
  } else {
    auto out = line();
    out << "static const ";
    if (!EmitTypeAndName(out, type, sem->StorageClass(), sem->Access(),
                         builder_.Symbols().NameFor(var->symbol))) {
      return false;
    }
    out << " = ";
    if (!EmitExpression(out, var->constructor)) {
      return false;
    }
    out << ";";
  }

  return true;
}

template <typename F>
bool GeneratorImpl::CallBuiltinHelper(std::ostream& out,
                                      const ast::CallExpression* call,
                                      const sem::Builtin* builtin,
                                      F&& build) {
  // Generate the helper function if it hasn't been created already
  auto fn = utils::GetOrCreate(builtins_, builtin, [&]() -> std::string {
    TextBuffer b;
    TINT_DEFER(helpers_.Append(b));

    auto fn_name =
        UniqueIdentifier(std::string("tint_") + sem::str(builtin->Type()));
    std::vector<std::string> parameter_names;
    {
      auto decl = line(&b);
      if (!EmitTypeAndName(decl, builtin->ReturnType(),
                           ast::StorageClass::kNone, ast::Access::kUndefined,
                           fn_name)) {
        return "";
      }
      {
        ScopedParen sp(decl);
        for (auto* param : builtin->Parameters()) {
          if (!parameter_names.empty()) {
            decl << ", ";
          }
          auto param_name = "param_" + std::to_string(parameter_names.size());
          const auto* ty = param->Type();
          if (auto* ptr = ty->As<sem::Pointer>()) {
            decl << "inout ";
            ty = ptr->StoreType();
          }
          if (!EmitTypeAndName(decl, ty, ast::StorageClass::kNone,
                               ast::Access::kUndefined, param_name)) {
            return "";
          }
          parameter_names.emplace_back(std::move(param_name));
        }
      }
      decl << " {";
    }
    {
      ScopedIndent si(&b);
      if (!build(&b, parameter_names)) {
        return "";
      }
    }
    line(&b) << "}";
    line(&b);
    return fn_name;
  });

  if (fn.empty()) {
    return false;
  }

  // Call the helper
  out << fn;
  {
    ScopedParen sp(out);
    bool first = true;
    for (auto* arg : call->args) {
      if (!first) {
        out << ", ";
      }
      first = false;
      if (!EmitExpression(out, arg)) {
        return false;
      }
    }
  }
  return true;
}

}  // namespace tint::writer::hlsl
