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

#include "src/tint/transform/vertex_pulling.h"

#include <algorithm>
#include <utility>

#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/math.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::VertexPulling);
TINT_INSTANTIATE_TYPEINFO(tint::transform::VertexPulling::Config);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

namespace {

/// The base type of a component.
/// The format type is either this type or a vector of this type.
enum class BaseType {
    kInvalid,
    kU32,
    kI32,
    kF32,
};

/// Writes the BaseType to the std::ostream.
/// @param out the std::ostream to write to
/// @param format the BaseType to write
/// @returns out so calls can be chained
std::ostream& operator<<(std::ostream& out, BaseType format) {
    switch (format) {
        case BaseType::kInvalid:
            return out << "invalid";
        case BaseType::kU32:
            return out << "u32";
        case BaseType::kI32:
            return out << "i32";
        case BaseType::kF32:
            return out << "f32";
    }
    return out << "<unknown>";
}

/// Writes the VertexFormat to the std::ostream.
/// @param out the std::ostream to write to
/// @param format the VertexFormat to write
/// @returns out so calls can be chained
std::ostream& operator<<(std::ostream& out, VertexFormat format) {
    switch (format) {
        case VertexFormat::kUint8x2:
            return out << "uint8x2";
        case VertexFormat::kUint8x4:
            return out << "uint8x4";
        case VertexFormat::kSint8x2:
            return out << "sint8x2";
        case VertexFormat::kSint8x4:
            return out << "sint8x4";
        case VertexFormat::kUnorm8x2:
            return out << "unorm8x2";
        case VertexFormat::kUnorm8x4:
            return out << "unorm8x4";
        case VertexFormat::kSnorm8x2:
            return out << "snorm8x2";
        case VertexFormat::kSnorm8x4:
            return out << "snorm8x4";
        case VertexFormat::kUint16x2:
            return out << "uint16x2";
        case VertexFormat::kUint16x4:
            return out << "uint16x4";
        case VertexFormat::kSint16x2:
            return out << "sint16x2";
        case VertexFormat::kSint16x4:
            return out << "sint16x4";
        case VertexFormat::kUnorm16x2:
            return out << "unorm16x2";
        case VertexFormat::kUnorm16x4:
            return out << "unorm16x4";
        case VertexFormat::kSnorm16x2:
            return out << "snorm16x2";
        case VertexFormat::kSnorm16x4:
            return out << "snorm16x4";
        case VertexFormat::kFloat16x2:
            return out << "float16x2";
        case VertexFormat::kFloat16x4:
            return out << "float16x4";
        case VertexFormat::kFloat32:
            return out << "float32";
        case VertexFormat::kFloat32x2:
            return out << "float32x2";
        case VertexFormat::kFloat32x3:
            return out << "float32x3";
        case VertexFormat::kFloat32x4:
            return out << "float32x4";
        case VertexFormat::kUint32:
            return out << "uint32";
        case VertexFormat::kUint32x2:
            return out << "uint32x2";
        case VertexFormat::kUint32x3:
            return out << "uint32x3";
        case VertexFormat::kUint32x4:
            return out << "uint32x4";
        case VertexFormat::kSint32:
            return out << "sint32";
        case VertexFormat::kSint32x2:
            return out << "sint32x2";
        case VertexFormat::kSint32x3:
            return out << "sint32x3";
        case VertexFormat::kSint32x4:
            return out << "sint32x4";
    }
    return out << "<unknown>";
}

/// A vertex attribute data format.
struct DataType {
    BaseType base_type;
    uint32_t width;  // 1 for scalar, 2+ for a vector
};

DataType DataTypeOf(const sem::Type* ty) {
    if (ty->Is<sem::I32>()) {
        return {BaseType::kI32, 1};
    }
    if (ty->Is<sem::U32>()) {
        return {BaseType::kU32, 1};
    }
    if (ty->Is<sem::F32>()) {
        return {BaseType::kF32, 1};
    }
    if (auto* vec = ty->As<sem::Vector>()) {
        return {DataTypeOf(vec->type()).base_type, vec->Width()};
    }
    return {BaseType::kInvalid, 0};
}

DataType DataTypeOf(VertexFormat format) {
    switch (format) {
        case VertexFormat::kUint32:
            return {BaseType::kU32, 1};
        case VertexFormat::kUint8x2:
        case VertexFormat::kUint16x2:
        case VertexFormat::kUint32x2:
            return {BaseType::kU32, 2};
        case VertexFormat::kUint32x3:
            return {BaseType::kU32, 3};
        case VertexFormat::kUint8x4:
        case VertexFormat::kUint16x4:
        case VertexFormat::kUint32x4:
            return {BaseType::kU32, 4};
        case VertexFormat::kSint32:
            return {BaseType::kI32, 1};
        case VertexFormat::kSint8x2:
        case VertexFormat::kSint16x2:
        case VertexFormat::kSint32x2:
            return {BaseType::kI32, 2};
        case VertexFormat::kSint32x3:
            return {BaseType::kI32, 3};
        case VertexFormat::kSint8x4:
        case VertexFormat::kSint16x4:
        case VertexFormat::kSint32x4:
            return {BaseType::kI32, 4};
        case VertexFormat::kFloat32:
            return {BaseType::kF32, 1};
        case VertexFormat::kUnorm8x2:
        case VertexFormat::kSnorm8x2:
        case VertexFormat::kUnorm16x2:
        case VertexFormat::kSnorm16x2:
        case VertexFormat::kFloat16x2:
        case VertexFormat::kFloat32x2:
            return {BaseType::kF32, 2};
        case VertexFormat::kFloat32x3:
            return {BaseType::kF32, 3};
        case VertexFormat::kUnorm8x4:
        case VertexFormat::kSnorm8x4:
        case VertexFormat::kUnorm16x4:
        case VertexFormat::kSnorm16x4:
        case VertexFormat::kFloat16x4:
        case VertexFormat::kFloat32x4:
            return {BaseType::kF32, 4};
    }
    return {BaseType::kInvalid, 0};
}

struct State {
    State(CloneContext& context, const VertexPulling::Config& c) : ctx(context), cfg(c) {}
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

    struct LocationInfo {
        std::function<const ast::Expression*()> expr;
        const sem::Type* type;
    };

    CloneContext& ctx;
    VertexPulling::Config const cfg;
    std::unordered_map<uint32_t, LocationInfo> location_info;
    std::function<const ast::Expression*()> vertex_index_expr = nullptr;
    std::function<const ast::Expression*()> instance_index_expr = nullptr;
    Symbol pulling_position_name;
    Symbol struct_buffer_name;
    std::unordered_map<uint32_t, Symbol> vertex_buffer_names;
    ast::VariableList new_function_parameters;

    /// Generate the vertex buffer binding name
    /// @param index index to append to buffer name
    Symbol GetVertexBufferName(uint32_t index) {
        return utils::GetOrCreate(vertex_buffer_names, index, [&] {
            static const char kVertexBufferNamePrefix[] = "tint_pulling_vertex_buffer_";
            return ctx.dst->Symbols().New(kVertexBufferNamePrefix + std::to_string(index));
        });
    }

    /// Lazily generates the structure buffer symbol
    Symbol GetStructBufferName() {
        if (!struct_buffer_name.IsValid()) {
            static const char kStructBufferName[] = "tint_vertex_data";
            struct_buffer_name = ctx.dst->Symbols().New(kStructBufferName);
        }
        return struct_buffer_name;
    }

    /// Adds storage buffer decorated variables for the vertex buffers
    void AddVertexStorageBuffers() {
        // Creating the struct type
        static const char kStructName[] = "TintVertexData";
        auto* struct_type =
            ctx.dst->Structure(ctx.dst->Symbols().New(kStructName),
                               {
                                   ctx.dst->Member(GetStructBufferName(), ctx.dst->ty.array<u32>()),
                               });
        for (uint32_t i = 0; i < cfg.vertex_state.size(); ++i) {
            // The decorated variable with struct type
            ctx.dst->Global(GetVertexBufferName(i), ctx.dst->ty.Of(struct_type),
                            ast::StorageClass::kStorage, ast::Access::kRead,
                            ast::AttributeList{
                                ctx.dst->create<ast::BindingAttribute>(i),
                                ctx.dst->create<ast::GroupAttribute>(cfg.pulling_group),
                            });
        }
    }

    /// Creates and returns the assignment to the variables from the buffers
    ast::BlockStatement* CreateVertexPullingPreamble() {
        // Assign by looking at the vertex descriptor to find attributes with
        // matching location.

        ast::StatementList stmts;

        for (uint32_t buffer_idx = 0; buffer_idx < cfg.vertex_state.size(); ++buffer_idx) {
            const VertexBufferLayoutDescriptor& buffer_layout = cfg.vertex_state[buffer_idx];

            if ((buffer_layout.array_stride & 3) != 0) {
                ctx.dst->Diagnostics().add_error(
                    diag::System::Transform,
                    "WebGPU requires that vertex stride must be a multiple of 4 bytes, "
                    "but VertexPulling array stride for buffer " +
                        std::to_string(buffer_idx) + " was " +
                        std::to_string(buffer_layout.array_stride) + " bytes");
                return nullptr;
            }

            auto* index_expr = buffer_layout.step_mode == VertexStepMode::kVertex
                                   ? vertex_index_expr()
                                   : instance_index_expr();

            // buffer_array_base is the base array offset for all the vertex
            // attributes. These are units of uint (4 bytes).
            auto buffer_array_base =
                ctx.dst->Symbols().New("buffer_array_base_" + std::to_string(buffer_idx));

            auto* attribute_offset = index_expr;
            if (buffer_layout.array_stride != 4) {
                attribute_offset = ctx.dst->Mul(index_expr, u32(buffer_layout.array_stride / 4u));
            }

            // let pulling_offset_n = <attribute_offset>
            stmts.emplace_back(
                ctx.dst->Decl(ctx.dst->Let(buffer_array_base, nullptr, attribute_offset)));

            for (const VertexAttributeDescriptor& attribute_desc : buffer_layout.attributes) {
                auto it = location_info.find(attribute_desc.shader_location);
                if (it == location_info.end()) {
                    continue;
                }
                auto& var = it->second;

                // Data type of the target WGSL variable
                auto var_dt = DataTypeOf(var.type);
                // Data type of the vertex stream attribute
                auto fmt_dt = DataTypeOf(attribute_desc.format);

                // Base types must match between the vertex stream and the WGSL variable
                if (var_dt.base_type != fmt_dt.base_type) {
                    std::stringstream err;
                    err << "VertexAttributeDescriptor for location "
                        << std::to_string(attribute_desc.shader_location) << " has format "
                        << attribute_desc.format << " but shader expects "
                        << var.type->FriendlyName(ctx.src->Symbols());
                    ctx.dst->Diagnostics().add_error(diag::System::Transform, err.str());
                    return nullptr;
                }

                // Load the attribute value
                auto* fetch = Fetch(buffer_array_base, attribute_desc.offset, buffer_idx,
                                    attribute_desc.format);

                // The attribute value may not be of the desired vector width. If it is
                // not, we'll need to either reduce the width with a swizzle, or append
                // 0's and / or a 1.
                auto* value = fetch;
                if (var_dt.width < fmt_dt.width) {
                    // WGSL variable vector width is smaller than the loaded vector width
                    switch (var_dt.width) {
                        case 1:
                            value = ctx.dst->MemberAccessor(fetch, "x");
                            break;
                        case 2:
                            value = ctx.dst->MemberAccessor(fetch, "xy");
                            break;
                        case 3:
                            value = ctx.dst->MemberAccessor(fetch, "xyz");
                            break;
                        default:
                            TINT_UNREACHABLE(Transform, ctx.dst->Diagnostics()) << var_dt.width;
                            return nullptr;
                    }
                } else if (var_dt.width > fmt_dt.width) {
                    // WGSL variable vector width is wider than the loaded vector width
                    const ast::Type* ty = nullptr;
                    ast::ExpressionList values{fetch};
                    switch (var_dt.base_type) {
                        case BaseType::kI32:
                            ty = ctx.dst->ty.i32();
                            for (uint32_t i = fmt_dt.width; i < var_dt.width; i++) {
                                values.emplace_back(ctx.dst->Expr((i == 3) ? 1_i : 0_i));
                            }
                            break;
                        case BaseType::kU32:
                            ty = ctx.dst->ty.u32();
                            for (uint32_t i = fmt_dt.width; i < var_dt.width; i++) {
                                values.emplace_back(ctx.dst->Expr((i == 3) ? 1_u : 0_u));
                            }
                            break;
                        case BaseType::kF32:
                            ty = ctx.dst->ty.f32();
                            for (uint32_t i = fmt_dt.width; i < var_dt.width; i++) {
                                values.emplace_back(ctx.dst->Expr((i == 3) ? 1_f : 0_f));
                            }
                            break;
                        default:
                            TINT_UNREACHABLE(Transform, ctx.dst->Diagnostics()) << var_dt.base_type;
                            return nullptr;
                    }
                    value = ctx.dst->Construct(ctx.dst->ty.vec(ty, var_dt.width), values);
                }

                // Assign the value to the WGSL variable
                stmts.emplace_back(ctx.dst->Assign(var.expr(), value));
            }
        }

        if (stmts.empty()) {
            return nullptr;
        }

        return ctx.dst->create<ast::BlockStatement>(stmts);
    }

    /// Generates an expression reading from a buffer a specific format.
    /// @param array_base the symbol of the variable holding the base array offset
    /// of the vertex array (each index is 4-bytes).
    /// @param offset the byte offset of the data from `buffer_base`
    /// @param buffer the index of the vertex buffer
    /// @param format the format to read
    const ast::Expression* Fetch(Symbol array_base,
                                 uint32_t offset,
                                 uint32_t buffer,
                                 VertexFormat format) {
        // Returns a u32 loaded from buffer_base + offset.
        auto load_u32 = [&] {
            return LoadPrimitive(array_base, offset, buffer, VertexFormat::kUint32);
        };

        // Returns a i32 loaded from buffer_base + offset.
        auto load_i32 = [&] { return ctx.dst->Bitcast<i32>(load_u32()); };

        // Returns a u32 loaded from buffer_base + offset + 4.
        auto load_next_u32 = [&] {
            return LoadPrimitive(array_base, offset + 4, buffer, VertexFormat::kUint32);
        };

        // Returns a i32 loaded from buffer_base + offset + 4.
        auto load_next_i32 = [&] { return ctx.dst->Bitcast<i32>(load_next_u32()); };

        // Returns a u16 loaded from offset, packed in the high 16 bits of a u32.
        // The low 16 bits are 0.
        // `min_alignment` must be a power of two.
        // `offset` must be `min_alignment` bytes aligned.
        auto load_u16_h = [&] {
            auto low_u32_offset = offset & ~3u;
            auto* low_u32 =
                LoadPrimitive(array_base, low_u32_offset, buffer, VertexFormat::kUint32);
            switch (offset & 3) {
                case 0:
                    return ctx.dst->Shl(low_u32, 16_u);
                case 1:
                    return ctx.dst->And(ctx.dst->Shl(low_u32, 8_u), 0xffff0000_u);
                case 2:
                    return ctx.dst->And(low_u32, 0xffff0000_u);
                default: {  // 3:
                    auto* high_u32 = LoadPrimitive(array_base, low_u32_offset + 4, buffer,
                                                   VertexFormat::kUint32);
                    auto* shr = ctx.dst->Shr(low_u32, 8_u);
                    auto* shl = ctx.dst->Shl(high_u32, 24_u);
                    return ctx.dst->And(ctx.dst->Or(shl, shr), 0xffff0000_u);
                }
            }
        };

        // Returns a u16 loaded from offset, packed in the low 16 bits of a u32.
        // The high 16 bits are 0.
        auto load_u16_l = [&] {
            auto low_u32_offset = offset & ~3u;
            auto* low_u32 =
                LoadPrimitive(array_base, low_u32_offset, buffer, VertexFormat::kUint32);
            switch (offset & 3) {
                case 0:
                    return ctx.dst->And(low_u32, 0xffff_u);
                case 1:
                    return ctx.dst->And(ctx.dst->Shr(low_u32, 8_u), 0xffff_u);
                case 2:
                    return ctx.dst->Shr(low_u32, 16_u);
                default: {  // 3:
                    auto* high_u32 = LoadPrimitive(array_base, low_u32_offset + 4, buffer,
                                                   VertexFormat::kUint32);
                    auto* shr = ctx.dst->Shr(low_u32, 24_u);
                    auto* shl = ctx.dst->Shl(high_u32, 8_u);
                    return ctx.dst->And(ctx.dst->Or(shl, shr), 0xffff_u);
                }
            }
        };

        // Returns a i16 loaded from offset, packed in the high 16 bits of a u32.
        // The low 16 bits are 0.
        auto load_i16_h = [&] { return ctx.dst->Bitcast<i32>(load_u16_h()); };

        // Assumptions are made that alignment must be at least as large as the size
        // of a single component.
        switch (format) {
            // Basic primitives
            case VertexFormat::kUint32:
            case VertexFormat::kSint32:
            case VertexFormat::kFloat32:
                return LoadPrimitive(array_base, offset, buffer, format);

                // Vectors of basic primitives
            case VertexFormat::kUint32x2:
                return LoadVec(array_base, offset, buffer, 4, ctx.dst->ty.u32(),
                               VertexFormat::kUint32, 2);
            case VertexFormat::kUint32x3:
                return LoadVec(array_base, offset, buffer, 4, ctx.dst->ty.u32(),
                               VertexFormat::kUint32, 3);
            case VertexFormat::kUint32x4:
                return LoadVec(array_base, offset, buffer, 4, ctx.dst->ty.u32(),
                               VertexFormat::kUint32, 4);
            case VertexFormat::kSint32x2:
                return LoadVec(array_base, offset, buffer, 4, ctx.dst->ty.i32(),
                               VertexFormat::kSint32, 2);
            case VertexFormat::kSint32x3:
                return LoadVec(array_base, offset, buffer, 4, ctx.dst->ty.i32(),
                               VertexFormat::kSint32, 3);
            case VertexFormat::kSint32x4:
                return LoadVec(array_base, offset, buffer, 4, ctx.dst->ty.i32(),
                               VertexFormat::kSint32, 4);
            case VertexFormat::kFloat32x2:
                return LoadVec(array_base, offset, buffer, 4, ctx.dst->ty.f32(),
                               VertexFormat::kFloat32, 2);
            case VertexFormat::kFloat32x3:
                return LoadVec(array_base, offset, buffer, 4, ctx.dst->ty.f32(),
                               VertexFormat::kFloat32, 3);
            case VertexFormat::kFloat32x4:
                return LoadVec(array_base, offset, buffer, 4, ctx.dst->ty.f32(),
                               VertexFormat::kFloat32, 4);

            case VertexFormat::kUint8x2: {
                // yyxx0000, yyxx0000
                auto* u16s = ctx.dst->vec2<u32>(load_u16_h());
                // xx000000, yyxx0000
                auto* shl = ctx.dst->Shl(u16s, ctx.dst->vec2<u32>(8_u, 0_u));
                // 000000xx, 000000yy
                return ctx.dst->Shr(shl, ctx.dst->vec2<u32>(24_u));
            }
            case VertexFormat::kUint8x4: {
                // wwzzyyxx, wwzzyyxx, wwzzyyxx, wwzzyyxx
                auto* u32s = ctx.dst->vec4<u32>(load_u32());
                // xx000000, yyxx0000, zzyyxx00, wwzzyyxx
                auto* shl = ctx.dst->Shl(u32s, ctx.dst->vec4<u32>(24_u, 16_u, 8_u, 0_u));
                // 000000xx, 000000yy, 000000zz, 000000ww
                return ctx.dst->Shr(shl, ctx.dst->vec4<u32>(24_u));
            }
            case VertexFormat::kUint16x2: {
                // yyyyxxxx, yyyyxxxx
                auto* u32s = ctx.dst->vec2<u32>(load_u32());
                // xxxx0000, yyyyxxxx
                auto* shl = ctx.dst->Shl(u32s, ctx.dst->vec2<u32>(16_u, 0_u));
                // 0000xxxx, 0000yyyy
                return ctx.dst->Shr(shl, ctx.dst->vec2<u32>(16_u));
            }
            case VertexFormat::kUint16x4: {
                // yyyyxxxx, wwwwzzzz
                auto* u32s = ctx.dst->vec2<u32>(load_u32(), load_next_u32());
                // yyyyxxxx, yyyyxxxx, wwwwzzzz, wwwwzzzz
                auto* xxyy = ctx.dst->MemberAccessor(u32s, "xxyy");
                // xxxx0000, yyyyxxxx, zzzz0000, wwwwzzzz
                auto* shl = ctx.dst->Shl(xxyy, ctx.dst->vec4<u32>(16_u, 0_u, 16_u, 0_u));
                // 0000xxxx, 0000yyyy, 0000zzzz, 0000wwww
                return ctx.dst->Shr(shl, ctx.dst->vec4<u32>(16_u));
            }
            case VertexFormat::kSint8x2: {
                // yyxx0000, yyxx0000
                auto* i16s = ctx.dst->vec2<i32>(load_i16_h());
                // xx000000, yyxx0000
                auto* shl = ctx.dst->Shl(i16s, ctx.dst->vec2<u32>(8_u, 0_u));
                // ssssssxx, ssssssyy
                return ctx.dst->Shr(shl, ctx.dst->vec2<u32>(24_u));
            }
            case VertexFormat::kSint8x4: {
                // wwzzyyxx, wwzzyyxx, wwzzyyxx, wwzzyyxx
                auto* i32s = ctx.dst->vec4<i32>(load_i32());
                // xx000000, yyxx0000, zzyyxx00, wwzzyyxx
                auto* shl = ctx.dst->Shl(i32s, ctx.dst->vec4<u32>(24_u, 16_u, 8_u, 0_u));
                // ssssssxx, ssssssyy, sssssszz, ssssssww
                return ctx.dst->Shr(shl, ctx.dst->vec4<u32>(24_u));
            }
            case VertexFormat::kSint16x2: {
                // yyyyxxxx, yyyyxxxx
                auto* i32s = ctx.dst->vec2<i32>(load_i32());
                // xxxx0000, yyyyxxxx
                auto* shl = ctx.dst->Shl(i32s, ctx.dst->vec2<u32>(16_u, 0_u));
                // ssssxxxx, ssssyyyy
                return ctx.dst->Shr(shl, ctx.dst->vec2<u32>(16_u));
            }
            case VertexFormat::kSint16x4: {
                // yyyyxxxx, wwwwzzzz
                auto* i32s = ctx.dst->vec2<i32>(load_i32(), load_next_i32());
                // yyyyxxxx, yyyyxxxx, wwwwzzzz, wwwwzzzz
                auto* xxyy = ctx.dst->MemberAccessor(i32s, "xxyy");
                // xxxx0000, yyyyxxxx, zzzz0000, wwwwzzzz
                auto* shl = ctx.dst->Shl(xxyy, ctx.dst->vec4<u32>(16_u, 0_u, 16_u, 0_u));
                // ssssxxxx, ssssyyyy, sssszzzz, sssswwww
                return ctx.dst->Shr(shl, ctx.dst->vec4<u32>(16_u));
            }
            case VertexFormat::kUnorm8x2:
                return ctx.dst->MemberAccessor(ctx.dst->Call("unpack4x8unorm", load_u16_l()), "xy");
            case VertexFormat::kSnorm8x2:
                return ctx.dst->MemberAccessor(ctx.dst->Call("unpack4x8snorm", load_u16_l()), "xy");
            case VertexFormat::kUnorm8x4:
                return ctx.dst->Call("unpack4x8unorm", load_u32());
            case VertexFormat::kSnorm8x4:
                return ctx.dst->Call("unpack4x8snorm", load_u32());
            case VertexFormat::kUnorm16x2:
                return ctx.dst->Call("unpack2x16unorm", load_u32());
            case VertexFormat::kSnorm16x2:
                return ctx.dst->Call("unpack2x16snorm", load_u32());
            case VertexFormat::kFloat16x2:
                return ctx.dst->Call("unpack2x16float", load_u32());
            case VertexFormat::kUnorm16x4:
                return ctx.dst->vec4<f32>(ctx.dst->Call("unpack2x16unorm", load_u32()),
                                          ctx.dst->Call("unpack2x16unorm", load_next_u32()));
            case VertexFormat::kSnorm16x4:
                return ctx.dst->vec4<f32>(ctx.dst->Call("unpack2x16snorm", load_u32()),
                                          ctx.dst->Call("unpack2x16snorm", load_next_u32()));
            case VertexFormat::kFloat16x4:
                return ctx.dst->vec4<f32>(ctx.dst->Call("unpack2x16float", load_u32()),
                                          ctx.dst->Call("unpack2x16float", load_next_u32()));
        }

        TINT_UNREACHABLE(Transform, ctx.dst->Diagnostics())
            << "format " << static_cast<int>(format);
        return nullptr;
    }

    /// Generates an expression reading an aligned basic type (u32, i32, f32) from
    /// a vertex buffer.
    /// @param array_base the symbol of the variable holding the base array offset
    /// of the vertex array (each index is 4-bytes).
    /// @param offset the byte offset of the data from `buffer_base`
    /// @param buffer the index of the vertex buffer
    /// @param format VertexFormat::kUint32, VertexFormat::kSint32 or
    /// VertexFormat::kFloat32
    const ast::Expression* LoadPrimitive(Symbol array_base,
                                         uint32_t offset,
                                         uint32_t buffer,
                                         VertexFormat format) {
        const ast::Expression* u = nullptr;
        if ((offset & 3) == 0) {
            // Aligned load.

            const ast ::Expression* index = nullptr;
            if (offset > 0) {
                index = ctx.dst->Add(array_base, u32(offset / 4));
            } else {
                index = ctx.dst->Expr(array_base);
            }
            u = ctx.dst->IndexAccessor(
                ctx.dst->MemberAccessor(GetVertexBufferName(buffer), GetStructBufferName()), index);

        } else {
            // Unaligned load
            uint32_t offset_aligned = offset & ~3u;
            auto* low = LoadPrimitive(array_base, offset_aligned, buffer, VertexFormat::kUint32);
            auto* high =
                LoadPrimitive(array_base, offset_aligned + 4u, buffer, VertexFormat::kUint32);

            uint32_t shift = 8u * (offset & 3u);

            auto* low_shr = ctx.dst->Shr(low, u32(shift));
            auto* high_shl = ctx.dst->Shl(high, u32(32u - shift));
            u = ctx.dst->Or(low_shr, high_shl);
        }

        switch (format) {
            case VertexFormat::kUint32:
                return u;
            case VertexFormat::kSint32:
                return ctx.dst->Bitcast(ctx.dst->ty.i32(), u);
            case VertexFormat::kFloat32:
                return ctx.dst->Bitcast(ctx.dst->ty.f32(), u);
            default:
                break;
        }
        TINT_UNREACHABLE(Transform, ctx.dst->Diagnostics())
            << "invalid format for LoadPrimitive" << static_cast<int>(format);
        return nullptr;
    }

    /// Generates an expression reading a vec2/3/4 from a vertex buffer.
    /// @param array_base the symbol of the variable holding the base array offset
    /// of the vertex array (each index is 4-bytes).
    /// @param offset the byte offset of the data from `buffer_base`
    /// @param buffer the index of the vertex buffer
    /// @param element_stride stride between elements, in bytes
    /// @param base_type underlying AST type
    /// @param base_format underlying vertex format
    /// @param count how many elements the vector has
    const ast::Expression* LoadVec(Symbol array_base,
                                   uint32_t offset,
                                   uint32_t buffer,
                                   uint32_t element_stride,
                                   const ast::Type* base_type,
                                   VertexFormat base_format,
                                   uint32_t count) {
        ast::ExpressionList expr_list;
        for (uint32_t i = 0; i < count; ++i) {
            // Offset read position by element_stride for each component
            uint32_t primitive_offset = offset + element_stride * i;
            expr_list.push_back(LoadPrimitive(array_base, primitive_offset, buffer, base_format));
        }

        return ctx.dst->Construct(ctx.dst->create<ast::Vector>(base_type, count),
                                  std::move(expr_list));
    }

    /// Process a non-struct entry point parameter.
    /// Generate function-scope variables for location parameters, and record
    /// vertex_index and instance_index builtins if present.
    /// @param func the entry point function
    /// @param param the parameter to process
    void ProcessNonStructParameter(const ast::Function* func, const ast::Variable* param) {
        if (auto* location = ast::GetAttribute<ast::LocationAttribute>(param->attributes)) {
            // Create a function-scope variable to replace the parameter.
            auto func_var_sym = ctx.Clone(param->symbol);
            auto* func_var_type = ctx.Clone(param->type);
            auto* func_var = ctx.dst->Var(func_var_sym, func_var_type);
            ctx.InsertFront(func->body->statements, ctx.dst->Decl(func_var));
            // Capture mapping from location to the new variable.
            LocationInfo info;
            info.expr = [this, func_var]() { return ctx.dst->Expr(func_var); };
            info.type = ctx.src->Sem().Get(param)->Type();
            location_info[location->value] = info;
        } else if (auto* builtin = ast::GetAttribute<ast::BuiltinAttribute>(param->attributes)) {
            // Check for existing vertex_index and instance_index builtins.
            if (builtin->builtin == ast::Builtin::kVertexIndex) {
                vertex_index_expr = [this, param]() {
                    return ctx.dst->Expr(ctx.Clone(param->symbol));
                };
            } else if (builtin->builtin == ast::Builtin::kInstanceIndex) {
                instance_index_expr = [this, param]() {
                    return ctx.dst->Expr(ctx.Clone(param->symbol));
                };
            }
            new_function_parameters.push_back(ctx.Clone(param));
        } else {
            TINT_ICE(Transform, ctx.dst->Diagnostics()) << "Invalid entry point parameter";
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
    void ProcessStructParameter(const ast::Function* func,
                                const ast::Variable* param,
                                const ast::Struct* struct_ty) {
        auto param_sym = ctx.Clone(param->symbol);

        // Process the struct members.
        bool has_locations = false;
        ast::StructMemberList members_to_clone;
        for (auto* member : struct_ty->members) {
            auto member_sym = ctx.Clone(member->symbol);
            std::function<const ast::Expression*()> member_expr = [this, param_sym, member_sym]() {
                return ctx.dst->MemberAccessor(param_sym, member_sym);
            };

            if (auto* location = ast::GetAttribute<ast::LocationAttribute>(member->attributes)) {
                // Capture mapping from location to struct member.
                LocationInfo info;
                info.expr = member_expr;
                info.type = ctx.src->Sem().Get(member)->Type();
                location_info[location->value] = info;
                has_locations = true;
            } else if (auto* builtin =
                           ast::GetAttribute<ast::BuiltinAttribute>(member->attributes)) {
                // Check for existing vertex_index and instance_index builtins.
                if (builtin->builtin == ast::Builtin::kVertexIndex) {
                    vertex_index_expr = member_expr;
                } else if (builtin->builtin == ast::Builtin::kInstanceIndex) {
                    instance_index_expr = member_expr;
                }
                members_to_clone.push_back(member);
            } else {
                TINT_ICE(Transform, ctx.dst->Diagnostics()) << "Invalid entry point parameter";
            }
        }

        if (!has_locations) {
            // Nothing to do.
            new_function_parameters.push_back(ctx.Clone(param));
            return;
        }

        // Create a function-scope variable to replace the parameter.
        auto* func_var = ctx.dst->Var(param_sym, ctx.Clone(param->type));
        ctx.InsertFront(func->body->statements, ctx.dst->Decl(func_var));

        if (!members_to_clone.empty()) {
            // Create a new struct without the location attributes.
            ast::StructMemberList new_members;
            for (auto* member : members_to_clone) {
                auto member_sym = ctx.Clone(member->symbol);
                auto* member_type = ctx.Clone(member->type);
                auto member_attrs = ctx.Clone(member->attributes);
                new_members.push_back(
                    ctx.dst->Member(member_sym, member_type, std::move(member_attrs)));
            }
            auto* new_struct = ctx.dst->Structure(ctx.dst->Sym(), new_members);

            // Create a new function parameter with this struct.
            auto* new_param = ctx.dst->Param(ctx.dst->Sym(), ctx.dst->ty.Of(new_struct));
            new_function_parameters.push_back(new_param);

            // Copy values from the new parameter to the function-scope variable.
            for (auto* member : members_to_clone) {
                auto member_name = ctx.Clone(member->symbol);
                ctx.InsertFront(func->body->statements,
                                ctx.dst->Assign(ctx.dst->MemberAccessor(func_var, member_name),
                                                ctx.dst->MemberAccessor(new_param, member_name)));
            }
        }
    }

    /// Process an entry point function.
    /// @param func the entry point function
    void Process(const ast::Function* func) {
        if (func->body->Empty()) {
            return;
        }

        // Process entry point parameters.
        for (auto* param : func->params) {
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
                if (layout.step_mode == VertexStepMode::kVertex) {
                    auto name = ctx.dst->Symbols().New("tint_pulling_vertex_index");
                    new_function_parameters.push_back(ctx.dst->Param(
                        name, ctx.dst->ty.u32(), {ctx.dst->Builtin(ast::Builtin::kVertexIndex)}));
                    vertex_index_expr = [this, name]() { return ctx.dst->Expr(name); };
                    break;
                }
            }
        }
        if (!instance_index_expr) {
            for (const VertexBufferLayoutDescriptor& layout : cfg.vertex_state) {
                if (layout.step_mode == VertexStepMode::kInstance) {
                    auto name = ctx.dst->Symbols().New("tint_pulling_instance_index");
                    new_function_parameters.push_back(ctx.dst->Param(
                        name, ctx.dst->ty.u32(), {ctx.dst->Builtin(ast::Builtin::kInstanceIndex)}));
                    instance_index_expr = [this, name]() { return ctx.dst->Expr(name); };
                    break;
                }
            }
        }

        // Generate vertex pulling preamble.
        if (auto* block = CreateVertexPullingPreamble()) {
            ctx.InsertFront(func->body->statements, block);
        }

        // Rewrite the function header with the new parameters.
        auto func_sym = ctx.Clone(func->symbol);
        auto* ret_type = ctx.Clone(func->return_type);
        auto* body = ctx.Clone(func->body);
        auto attrs = ctx.Clone(func->attributes);
        auto ret_attrs = ctx.Clone(func->return_type_attributes);
        auto* new_func =
            ctx.dst->create<ast::Function>(func->source, func_sym, new_function_parameters,
                                           ret_type, body, std::move(attrs), std::move(ret_attrs));
        ctx.Replace(func, new_func);
    }
};

}  // namespace

VertexPulling::VertexPulling() = default;
VertexPulling::~VertexPulling() = default;

void VertexPulling::Run(CloneContext& ctx, const DataMap& inputs, DataMap&) const {
    auto cfg = cfg_;
    if (auto* cfg_data = inputs.Get<Config>()) {
        cfg = *cfg_data;
    }

    // Find entry point
    auto* func = ctx.src->AST().Functions().Find(ctx.src->Symbols().Get(cfg.entry_point_name),
                                                 ast::PipelineStage::kVertex);
    if (func == nullptr) {
        ctx.dst->Diagnostics().add_error(diag::System::Transform,
                                         "Vertex stage entry point not found");
        return;
    }

    // TODO(idanr): Need to check shader locations in descriptor cover all
    // attributes

    // TODO(idanr): Make sure we covered all error cases, to guarantee the
    // following stages will pass

    State state{ctx, cfg};
    state.AddVertexStorageBuffers();
    state.Process(func);

    ctx.Clone();
}

VertexPulling::Config::Config() = default;
VertexPulling::Config::Config(const Config&) = default;
VertexPulling::Config::~Config() = default;
VertexPulling::Config& VertexPulling::Config::operator=(const Config&) = default;

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor() = default;

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor(
    uint32_t in_array_stride,
    VertexStepMode in_step_mode,
    std::vector<VertexAttributeDescriptor> in_attributes)
    : array_stride(in_array_stride),
      step_mode(in_step_mode),
      attributes(std::move(in_attributes)) {}

VertexBufferLayoutDescriptor::VertexBufferLayoutDescriptor(
    const VertexBufferLayoutDescriptor& other) = default;

VertexBufferLayoutDescriptor& VertexBufferLayoutDescriptor::operator=(
    const VertexBufferLayoutDescriptor& other) = default;

VertexBufferLayoutDescriptor::~VertexBufferLayoutDescriptor() = default;

}  // namespace tint::transform
