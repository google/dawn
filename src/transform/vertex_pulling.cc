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

#include "src/transform/vertex_pulling.h"

#include <utility>

#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/program_builder.h"
#include "src/sem/variable.h"
#include "src/utils/get_or_create.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::VertexPulling::Config);

namespace tint {
namespace transform {

namespace {

struct State {
  State(CloneContext& context, const VertexPulling::Config& c)
      : ctx(context), cfg(c) {}
  State(const State&) = default;
  ~State() = default;

  /// LocationReplacement describes an ast::Variable replacement for a
  /// location input.
  struct LocationReplacement {
    /// The variable to replace in the source Program
    ast::Variable* from;
    /// The replacement to use in the target ProgramBuilder
    ast::Variable* to;
  };

  CloneContext& ctx;
  VertexPulling::Config const cfg;
  std::unordered_map<uint32_t, std::function<ast::Expression*()>>
      location_to_expr;
  std::function<ast::Expression*()> vertex_index_expr = nullptr;
  std::function<ast::Expression*()> instance_index_expr = nullptr;
  Symbol pulling_position_name;
  Symbol struct_buffer_name;
  std::unordered_map<uint32_t, Symbol> vertex_buffer_names;
  ast::VariableList new_function_parameters;

  /// Generate the vertex buffer binding name
  /// @param index index to append to buffer name
  Symbol GetVertexBufferName(uint32_t index) {
    return utils::GetOrCreate(vertex_buffer_names, index, [&] {
      static const char kVertexBufferNamePrefix[] =
          "tint_pulling_vertex_buffer_";
      return ctx.dst->Symbols().New(kVertexBufferNamePrefix +
                                    std::to_string(index));
    });
  }

  /// Lazily generates the pulling position symbol
  Symbol GetPullingPositionName() {
    if (!pulling_position_name.IsValid()) {
      static const char kPullingPosVarName[] = "tint_pulling_pos";
      pulling_position_name = ctx.dst->Symbols().New(kPullingPosVarName);
    }
    return pulling_position_name;
  }

  /// Lazily generates the structure buffer symbol
  Symbol GetStructBufferName() {
    if (!struct_buffer_name.IsValid()) {
      static const char kStructBufferName[] = "tint_vertex_data";
      struct_buffer_name = ctx.dst->Symbols().New(kStructBufferName);
    }
    return struct_buffer_name;
  }

  /// Inserts vertex_index binding, or finds the existing one
  void FindOrInsertVertexIndexIfUsed() {
    bool uses_vertex_step_mode = false;
    for (const VertexBufferLayoutDescriptor& buffer_layout : cfg.vertex_state) {
      if (buffer_layout.step_mode == InputStepMode::kVertex) {
        uses_vertex_step_mode = true;
        break;
      }
    }
    if (!uses_vertex_step_mode) {
      return;
    }

    // Look for an existing vertex index builtin
    for (auto* v : ctx.src->AST().GlobalVariables()) {
      auto* sem = ctx.src->Sem().Get(v);
      if (sem->StorageClass() != ast::StorageClass::kInput) {
        continue;
      }

      for (auto* d : v->decorations()) {
        if (auto* builtin = d->As<ast::BuiltinDecoration>()) {
          if (builtin->value() == ast::Builtin::kVertexIndex) {
            vertex_index_expr = [this, v]() {
              return ctx.dst->Expr(ctx.Clone(v->symbol()));
            };
            return;
          }
        }
      }
    }

    // We didn't find a vertex index builtin, so create one
    auto name = ctx.dst->Symbols().New("tint_pulling_vertex_index");
    vertex_index_expr = [this, name]() { return ctx.dst->Expr(name); };

    ctx.dst->Global(name, ctx.dst->ty.u32(), ast::StorageClass::kInput, nullptr,
                    ast::DecorationList{
                        ctx.dst->Builtin(ast::Builtin::kVertexIndex),
                    });
  }

  /// Inserts instance_index binding, or finds the existing one
  void FindOrInsertInstanceIndexIfUsed() {
    bool uses_instance_step_mode = false;
    for (const VertexBufferLayoutDescriptor& buffer_layout : cfg.vertex_state) {
      if (buffer_layout.step_mode == InputStepMode::kInstance) {
        uses_instance_step_mode = true;
        break;
      }
    }
    if (!uses_instance_step_mode) {
      return;
    }

    // Look for an existing instance index builtin
    for (auto* v : ctx.src->AST().GlobalVariables()) {
      auto* sem = ctx.src->Sem().Get(v);
      if (sem->StorageClass() != ast::StorageClass::kInput) {
        continue;
      }

      for (auto* d : v->decorations()) {
        if (auto* builtin = d->As<ast::BuiltinDecoration>()) {
          if (builtin->value() == ast::Builtin::kInstanceIndex) {
            instance_index_expr = [this, v]() {
              return ctx.dst->Expr(ctx.Clone(v->symbol()));
            };
            return;
          }
        }
      }
    }

    // We didn't find an instance index builtin, so create one
    auto name = ctx.dst->Symbols().New("tint_pulling_instance_index");
    instance_index_expr = [this, name]() { return ctx.dst->Expr(name); };

    ctx.dst->Global(name, ctx.dst->ty.u32(), ast::StorageClass::kInput, nullptr,
                    ast::DecorationList{
                        ctx.dst->Builtin(ast::Builtin::kInstanceIndex),
                    });
  }

  /// Converts var<in> with a location decoration to var<private>
  void ConvertVertexInputVariablesToPrivate() {
    for (auto* v : ctx.src->AST().GlobalVariables()) {
      auto* sem = ctx.src->Sem().Get(v);
      if (sem->StorageClass() != ast::StorageClass::kInput) {
        continue;
      }

      for (auto* d : v->decorations()) {
        if (auto* l = d->As<ast::LocationDecoration>()) {
          uint32_t location = l->value();
          // This is where the replacement is created. Expressions use
          // identifier strings instead of pointers, so we don't need to update
          // any other place in the AST.
          auto name = ctx.Clone(v->symbol());
          auto* replacement = ctx.dst->Var(name, ctx.Clone(v->type()),
                                           ast::StorageClass::kPrivate);
          location_to_expr[location] = [this, name]() {
            return ctx.dst->Expr(name);
          };
          ctx.Replace(v, replacement);
          break;
        }
      }
    }
  }

  /// Adds storage buffer decorated variables for the vertex buffers
  void AddVertexStorageBuffers() {
    // TODO(idanr): Make this readonly
    // https://github.com/gpuweb/gpuweb/issues/935

    // Creating the struct type
    static const char kStructName[] = "TintVertexData";
    auto* struct_type = ctx.dst->Structure(
        ctx.dst->Symbols().New(kStructName),
        {
            ctx.dst->Member(GetStructBufferName(),
                            ctx.dst->ty.array<ProgramBuilder::u32, 0>(4)),
        },
        {
            ctx.dst->create<ast::StructBlockDecoration>(),
        });
    for (uint32_t i = 0; i < cfg.vertex_state.size(); ++i) {
      auto* access =
          ctx.dst->ty.access(ast::AccessControl::kReadOnly, struct_type);
      // The decorated variable with struct type
      ctx.dst->Global(
          GetVertexBufferName(i), access, ast::StorageClass::kStorage, nullptr,
          ast::DecorationList{
              ctx.dst->create<ast::BindingDecoration>(i),
              ctx.dst->create<ast::GroupDecoration>(cfg.pulling_group),
          });
    }
  }

  /// Creates and returns the assignment to the variables from the buffers
  ast::BlockStatement* CreateVertexPullingPreamble() {
    // Assign by looking at the vertex descriptor to find attributes with
    // matching location.

    ast::StatementList stmts;

    // Declare the pulling position variable in the shader
    stmts.emplace_back(ctx.dst->Decl(
        ctx.dst->Var(GetPullingPositionName(), ctx.dst->ty.u32())));

    for (uint32_t i = 0; i < cfg.vertex_state.size(); ++i) {
      const VertexBufferLayoutDescriptor& buffer_layout = cfg.vertex_state[i];

      for (const VertexAttributeDescriptor& attribute_desc :
           buffer_layout.attributes) {
        auto it = location_to_expr.find(attribute_desc.shader_location);
        if (it == location_to_expr.end()) {
          continue;
        }
        auto* ident = it->second();

        auto* index_expr = buffer_layout.step_mode == InputStepMode::kVertex
                               ? vertex_index_expr()
                               : instance_index_expr();

        // An expression for the start of the read in the buffer in bytes
        auto* pos_value = ctx.dst->Add(
            ctx.dst->Mul(index_expr,
                         static_cast<uint32_t>(buffer_layout.array_stride)),
            static_cast<uint32_t>(attribute_desc.offset));

        // Update position of the read
        auto* set_pos_expr =
            ctx.dst->Assign(ctx.dst->Expr(GetPullingPositionName()), pos_value);
        stmts.emplace_back(set_pos_expr);

        stmts.emplace_back(
            ctx.dst->Assign(ident, AccessByFormat(i, attribute_desc.format)));
      }
    }

    return ctx.dst->create<ast::BlockStatement>(stmts);
  }

  /// Generates an expression reading from a buffer a specific format.
  /// This reads the value wherever `kPullingPosVarName` points to at the time
  /// of the read.
  /// @param buffer the index of the vertex buffer
  /// @param format the format to read
  ast::Expression* AccessByFormat(uint32_t buffer, VertexFormat format) {
    // TODO(idanr): this doesn't account for the format of the attribute in the
    // shader. ex: vec<u32> in shader, and attribute claims VertexFormat::Float4
    // right now, we would try to assign a vec4<f32> to this attribute, but we
    // really need to assign a vec4<u32> by casting.
    // We could split this function to first do memory accesses and unpacking
    // into int/uint/float1-4/etc, then convert that variable to a var<in> with
    // the conversion defined in the WebGPU spec.
    switch (format) {
      case VertexFormat::kU32:
        return AccessU32(buffer, ctx.dst->Expr(GetPullingPositionName()));
      case VertexFormat::kI32:
        return AccessI32(buffer, ctx.dst->Expr(GetPullingPositionName()));
      case VertexFormat::kF32:
        return AccessF32(buffer, ctx.dst->Expr(GetPullingPositionName()));
      case VertexFormat::kVec2F32:
        return AccessVec(buffer, 4, ctx.dst->ty.f32(), VertexFormat::kF32, 2);
      case VertexFormat::kVec3F32:
        return AccessVec(buffer, 4, ctx.dst->ty.f32(), VertexFormat::kF32, 3);
      case VertexFormat::kVec4F32:
        return AccessVec(buffer, 4, ctx.dst->ty.f32(), VertexFormat::kF32, 4);
      default:
        return nullptr;
    }
  }

  /// Generates an expression reading a uint32 from a vertex buffer
  /// @param buffer the index of the vertex buffer
  /// @param pos an expression for the position of the access, in bytes
  ast::Expression* AccessU32(uint32_t buffer, ast::Expression* pos) {
    // Here we divide by 4, since the buffer is uint32 not uint8. The input
    // buffer has byte offsets for each attribute, and we will convert it to u32
    // indexes by dividing. Then, that element is going to be read, and if
    // needed, unpacked into an appropriate variable. All reads should end up
    // here as a base case.
    return ctx.dst->create<ast::ArrayAccessorExpression>(
        ctx.dst->MemberAccessor(GetVertexBufferName(buffer),
                                GetStructBufferName()),
        ctx.dst->Div(pos, 4u));
  }

  /// Generates an expression reading an int32 from a vertex buffer
  /// @param buffer the index of the vertex buffer
  /// @param pos an expression for the position of the access, in bytes
  ast::Expression* AccessI32(uint32_t buffer, ast::Expression* pos) {
    // as<T> reinterprets bits
    return ctx.dst->create<ast::BitcastExpression>(ctx.dst->ty.i32(),
                                                   AccessU32(buffer, pos));
  }

  /// Generates an expression reading a float from a vertex buffer
  /// @param buffer the index of the vertex buffer
  /// @param pos an expression for the position of the access, in bytes
  ast::Expression* AccessF32(uint32_t buffer, ast::Expression* pos) {
    // as<T> reinterprets bits
    return ctx.dst->create<ast::BitcastExpression>(ctx.dst->ty.f32(),
                                                   AccessU32(buffer, pos));
  }

  /// Generates an expression reading a basic type (u32, i32, f32) from a
  /// vertex buffer
  /// @param buffer the index of the vertex buffer
  /// @param pos an expression for the position of the access, in bytes
  /// @param format the underlying vertex format
  ast::Expression* AccessPrimitive(uint32_t buffer,
                                   ast::Expression* pos,
                                   VertexFormat format) {
    // This function uses a position expression to read, rather than using the
    // position variable. This allows us to read from offset positions relative
    // to |kPullingPosVarName|. We can't call AccessByFormat because it reads
    // only from the position variable.
    switch (format) {
      case VertexFormat::kU32:
        return AccessU32(buffer, pos);
      case VertexFormat::kI32:
        return AccessI32(buffer, pos);
      case VertexFormat::kF32:
        return AccessF32(buffer, pos);
      default:
        return nullptr;
    }
  }

  /// Generates an expression reading a vec2/3/4 from a vertex buffer.
  /// This reads the value wherever `kPullingPosVarName` points to at the time
  /// of the read.
  /// @param buffer the index of the vertex buffer
  /// @param element_stride stride between elements, in bytes
  /// @param base_type underlying AST type
  /// @param base_format underlying vertex format
  /// @param count how many elements the vector has
  ast::Expression* AccessVec(uint32_t buffer,
                             uint32_t element_stride,
                             ast::Type* base_type,
                             VertexFormat base_format,
                             uint32_t count) {
    ast::ExpressionList expr_list;
    for (uint32_t i = 0; i < count; ++i) {
      // Offset read position by element_stride for each component
      auto* cur_pos =
          ctx.dst->Add(GetPullingPositionName(), element_stride * i);
      expr_list.push_back(AccessPrimitive(buffer, cur_pos, base_format));
    }

    return ctx.dst->create<ast::TypeConstructorExpression>(
        ctx.dst->create<ast::Vector>(base_type, count), std::move(expr_list));
  }

  /// Process a non-struct entry point parameter.
  /// Generate function-scope variables for location parameters, and record
  /// vertex_index and instance_index builtins if present.
  /// @param func the entry point function
  /// @param param the parameter to process
  void ProcessNonStructParameter(ast::Function* func, ast::Variable* param) {
    if (auto* location =
            ast::GetDecoration<ast::LocationDecoration>(param->decorations())) {
      // Create a function-scope variable to replace the parameter.
      auto func_var_sym = ctx.Clone(param->symbol());
      auto* func_var_type = ctx.Clone(param->type());
      auto* func_var = ctx.dst->Var(func_var_sym, func_var_type);
      ctx.InsertBefore(func->body()->statements(), *func->body()->begin(),
                       ctx.dst->Decl(func_var));
      // Capture mapping from location to the new variable.
      location_to_expr[location->value()] = [this, func_var]() {
        return ctx.dst->Expr(func_var);
      };
    } else if (auto* builtin = ast::GetDecoration<ast::BuiltinDecoration>(
                   param->decorations())) {
      // Check for existing vertex_index and instance_index builtins.
      if (builtin->value() == ast::Builtin::kVertexIndex) {
        vertex_index_expr = [this, param]() {
          return ctx.dst->Expr(ctx.Clone(param->symbol()));
        };
      } else if (builtin->value() == ast::Builtin::kInstanceIndex) {
        instance_index_expr = [this, param]() {
          return ctx.dst->Expr(ctx.Clone(param->symbol()));
        };
      }
      new_function_parameters.push_back(ctx.Clone(param));
    } else {
      TINT_ICE(ctx.dst->Diagnostics()) << "Invalid entry point parameter";
    }
  }

  /// Process a struct entry point parameter.
  /// If the struct has members with location attributes, push the parameter to
  /// a function-scope variable and create a new struct parameter without those
  /// attributes. Record expressions for members that are vertex_index and
  /// instance_index builtins.
  /// @param func the entry point function
  /// @param param the parameter to process
  /// @param struct_ty the structure type
  void ProcessStructParameter(ast::Function* func,
                              ast::Variable* param,
                              const ast::Struct* struct_ty) {
    auto param_sym = ctx.Clone(param->symbol());

    // Process the struct members.
    bool has_locations = false;
    ast::StructMemberList members_to_clone;
    for (auto* member : struct_ty->members()) {
      auto member_sym = ctx.Clone(member->symbol());
      std::function<ast::Expression*()> member_expr = [this, param_sym,
                                                       member_sym]() {
        return ctx.dst->MemberAccessor(param_sym, member_sym);
      };

      if (auto* location = ast::GetDecoration<ast::LocationDecoration>(
              member->decorations())) {
        // Capture mapping from location to struct member.
        location_to_expr[location->value()] = member_expr;
        has_locations = true;
      } else if (auto* builtin = ast::GetDecoration<ast::BuiltinDecoration>(
                     member->decorations())) {
        // Check for existing vertex_index and instance_index builtins.
        if (builtin->value() == ast::Builtin::kVertexIndex) {
          vertex_index_expr = member_expr;
        } else if (builtin->value() == ast::Builtin::kInstanceIndex) {
          instance_index_expr = member_expr;
        }
        members_to_clone.push_back(member);
      } else {
        TINT_ICE(ctx.dst->Diagnostics()) << "Invalid entry point parameter";
      }
    }

    if (!has_locations) {
      // Nothing to do.
      new_function_parameters.push_back(ctx.Clone(param));
      return;
    }

    // Create a function-scope variable to replace the parameter.
    auto* func_var = ctx.dst->Var(param_sym, ctx.Clone(param->type()));
    ctx.InsertBefore(func->body()->statements(), *func->body()->begin(),
                     ctx.dst->Decl(func_var));

    if (!members_to_clone.empty()) {
      // Create a new struct without the location attributes.
      ast::StructMemberList new_members;
      for (auto* member : members_to_clone) {
        auto member_sym = ctx.Clone(member->symbol());
        auto* member_type = ctx.Clone(member->type());
        auto member_decos = ctx.Clone(member->decorations());
        new_members.push_back(
            ctx.dst->Member(member_sym, member_type, std::move(member_decos)));
      }
      auto* new_struct = ctx.dst->Structure(ctx.dst->Sym(), new_members);

      // Create a new function parameter with this struct.
      auto* new_param = ctx.dst->Param(ctx.dst->Sym(), new_struct);
      new_function_parameters.push_back(new_param);

      // Copy values from the new parameter to the function-scope variable.
      for (auto* member : members_to_clone) {
        auto member_name = ctx.Clone(member->symbol());
        ctx.InsertBefore(
            func->body()->statements(), *func->body()->begin(),
            ctx.dst->Assign(ctx.dst->MemberAccessor(func_var, member_name),
                            ctx.dst->MemberAccessor(new_param, member_name)));
      }
    }
  }

  /// Process an entry point function.
  /// @param func the entry point function
  void Process(ast::Function* func) {
    if (func->body()->empty()) {
      return;
    }

    // Process entry point parameters.
    for (auto* param : func->params()) {
      auto* sem = ctx.src->Sem().Get(param);
      if (auto* str = sem->Type()->As<sem::Struct>()) {
        ProcessStructParameter(func, param, str->Declaration());
      } else {
        ProcessNonStructParameter(func, param);
      }
    }

    // Insert new parameters for vertex_index and instance_index if needed.
    if (!vertex_index_expr) {
      for (const VertexBufferLayoutDescriptor& layout : cfg.vertex_state) {
        if (layout.step_mode == InputStepMode::kVertex) {
          auto name = ctx.dst->Symbols().New("tint_pulling_vertex_index");
          new_function_parameters.push_back(
              ctx.dst->Param(name, ctx.dst->ty.u32(),
                             {ctx.dst->Builtin(ast::Builtin::kVertexIndex)}));
          vertex_index_expr = [this, name]() { return ctx.dst->Expr(name); };
          break;
        }
      }
    }
    if (!instance_index_expr) {
      for (const VertexBufferLayoutDescriptor& layout : cfg.vertex_state) {
        if (layout.step_mode == InputStepMode::kInstance) {
          auto name = ctx.dst->Symbols().New("tint_pulling_instance_index");
          new_function_parameters.push_back(
              ctx.dst->Param(name, ctx.dst->ty.u32(),
                             {ctx.dst->Builtin(ast::Builtin::kInstanceIndex)}));
          instance_index_expr = [this, name]() { return ctx.dst->Expr(name); };
          break;
        }
      }
    }

    // Generate vertex pulling preamble.
    ctx.InsertBefore(func->body()->statements(), *func->body()->begin(),
                     CreateVertexPullingPreamble());

    // Rewrite the function header with the new parameters.
    auto func_sym = ctx.Clone(func->symbol());
    auto* ret_type = ctx.Clone(func->return_type());
    auto* body = ctx.Clone(func->body());
    auto decos = ctx.Clone(func->decorations());
    auto ret_decos = ctx.Clone(func->return_type_decorations());
    auto* new_func = ctx.dst->create<ast::Function>(
        func->source(), func_sym, new_function_parameters, ret_type, body,
        std::move(decos), std::move(ret_decos));
    ctx.Replace(func, new_func);
  }
};

}  // namespace

VertexPulling::VertexPulling() = default;
VertexPulling::~VertexPulling() = default;

Output VertexPulling::Run(const Program* in, const DataMap& data) {
  ProgramBuilder out;

  auto cfg = cfg_;
  if (auto* cfg_data = data.Get<Config>()) {
    cfg = *cfg_data;
  }

  // Find entry point
  auto* func = in->AST().Functions().Find(
      in->Symbols().Get(cfg.entry_point_name), ast::PipelineStage::kVertex);
  if (func == nullptr) {
    out.Diagnostics().add_error("Vertex stage entry point not found");
    return Output(Program(std::move(out)));
  }

  // TODO(idanr): Need to check shader locations in descriptor cover all
  // attributes

  // TODO(idanr): Make sure we covered all error cases, to guarantee the
  // following stages will pass

  CloneContext ctx(&out, in);

  State state{ctx, cfg};

  if (func->params().empty()) {
    // TODO(crbug.com/tint/697): Remove this path for the old shader IO syntax.
    state.FindOrInsertVertexIndexIfUsed();
    state.FindOrInsertInstanceIndexIfUsed();
    state.ConvertVertexInputVariablesToPrivate();
    state.AddVertexStorageBuffers();

    ctx.ReplaceAll([&](ast::Function* f) -> ast::Function* {
      if (f == func) {
        return CloneWithStatementsAtStart(
            &ctx, f, {state.CreateVertexPullingPreamble()});
      }
      return nullptr;  // Just clone func
    });
  } else {
    state.AddVertexStorageBuffers();
    state.Process(func);
  }

  ctx.Clone();

  return Output(Program(std::move(out)));
}

VertexPulling::Config::Config() = default;
VertexPulling::Config::Config(const Config&) = default;
VertexPulling::Config::~Config() = default;
VertexPulling::Config& VertexPulling::Config::operator=(const Config&) =
    default;

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor() = default;

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor(
    uint64_t in_array_stride,
    InputStepMode in_step_mode,
    std::vector<VertexAttributeDescriptor> in_attributes)
    : array_stride(in_array_stride),
      step_mode(in_step_mode),
      attributes(std::move(in_attributes)) {}

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor(
    const VertexBufferLayoutDescriptor& other) = default;

VertexBufferLayoutDescriptor& VertexBufferLayoutDescriptor::operator=(
    const VertexBufferLayoutDescriptor& other) = default;

VertexBufferLayoutDescriptor::~VertexBufferLayoutDescriptor() = default;

}  // namespace transform
}  // namespace tint
