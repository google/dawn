// Copyright 2021 The Tint Authors.
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

#include "src/transform/hlsl.h"

#include <utility>

#include "src/ast/stage_decoration.h"
#include "src/ast/variable_decl_statement.h"
#include "src/program_builder.h"
#include "src/semantic/expression.h"
#include "src/semantic/statement.h"
#include "src/semantic/variable.h"
#include "src/transform/calculate_array_length.h"
#include "src/transform/canonicalize_entry_point_io.h"
#include "src/transform/decompose_storage_access.h"
#include "src/transform/manager.h"

namespace tint {
namespace transform {

namespace {

// This list is used for a binary search and must be kept in sorted order.
const char* kReservedKeywords[] = {"AddressU",
                                   "AddressV",
                                   "AddressW",
                                   "AllMemoryBarrier",
                                   "AllMemoryBarrierWithGroupSync",
                                   "AppendStructuredBuffer",
                                   "BINORMAL",
                                   "BLENDINDICES",
                                   "BLENDWEIGHT",
                                   "BlendState",
                                   "BorderColor",
                                   "Buffer",
                                   "ByteAddressBuffer",
                                   "COLOR",
                                   "CheckAccessFullyMapped",
                                   "ComparisonFunc",
                                   "CompileShader",
                                   "ComputeShader",
                                   "ConsumeStructuredBuffer",
                                   "D3DCOLORtoUBYTE4",
                                   "DEPTH",
                                   "DepthStencilState",
                                   "DepthStencilView",
                                   "DeviceMemoryBarrier",
                                   "DeviceMemroyBarrierWithGroupSync",
                                   "DomainShader",
                                   "EvaluateAttributeAtCentroid",
                                   "EvaluateAttributeAtSample",
                                   "EvaluateAttributeSnapped",
                                   "FOG",
                                   "Filter",
                                   "GeometryShader",
                                   "GetRenderTargetSampleCount",
                                   "GetRenderTargetSamplePosition",
                                   "GroupMemoryBarrier",
                                   "GroupMemroyBarrierWithGroupSync",
                                   "Hullshader",
                                   "InputPatch",
                                   "InterlockedAdd",
                                   "InterlockedAnd",
                                   "InterlockedCompareExchange",
                                   "InterlockedCompareStore",
                                   "InterlockedExchange",
                                   "InterlockedMax",
                                   "InterlockedMin",
                                   "InterlockedOr",
                                   "InterlockedXor",
                                   "LineStream",
                                   "MaxAnisotropy",
                                   "MaxLOD",
                                   "MinLOD",
                                   "MipLODBias",
                                   "NORMAL",
                                   "NULL",
                                   "Normal",
                                   "OutputPatch",
                                   "POSITION",
                                   "POSITIONT",
                                   "PSIZE",
                                   "PixelShader",
                                   "PointStream",
                                   "Process2DQuadTessFactorsAvg",
                                   "Process2DQuadTessFactorsMax",
                                   "Process2DQuadTessFactorsMin",
                                   "ProcessIsolineTessFactors",
                                   "ProcessQuadTessFactorsAvg",
                                   "ProcessQuadTessFactorsMax",
                                   "ProcessQuadTessFactorsMin",
                                   "ProcessTriTessFactorsAvg",
                                   "ProcessTriTessFactorsMax",
                                   "ProcessTriTessFactorsMin",
                                   "RWBuffer",
                                   "RWByteAddressBuffer",
                                   "RWStructuredBuffer",
                                   "RWTexture1D",
                                   "RWTexture2D",
                                   "RWTexture2DArray",
                                   "RWTexture3D",
                                   "RasterizerState",
                                   "RenderTargetView",
                                   "SV_ClipDistance",
                                   "SV_Coverage",
                                   "SV_CullDistance",
                                   "SV_Depth",
                                   "SV_DepthGreaterEqual",
                                   "SV_DepthLessEqual",
                                   "SV_DispatchThreadID",
                                   "SV_DomainLocation",
                                   "SV_GSInstanceID",
                                   "SV_GroupID",
                                   "SV_GroupIndex",
                                   "SV_GroupThreadID",
                                   "SV_InnerCoverage",
                                   "SV_InsideTessFactor",
                                   "SV_InstanceID",
                                   "SV_IsFrontFace",
                                   "SV_OutputControlPointID",
                                   "SV_Position",
                                   "SV_PrimitiveID",
                                   "SV_RenderTargetArrayIndex",
                                   "SV_SampleIndex",
                                   "SV_StencilRef",
                                   "SV_Target",
                                   "SV_TessFactor",
                                   "SV_VertexArrayIndex",
                                   "SV_VertexID",
                                   "Sampler",
                                   "Sampler1D",
                                   "Sampler2D",
                                   "Sampler3D",
                                   "SamplerCUBE",
                                   "StructuredBuffer",
                                   "TANGENT",
                                   "TESSFACTOR",
                                   "TEXCOORD",
                                   "Texcoord",
                                   "Texture",
                                   "Texture1D",
                                   "Texture2D",
                                   "Texture2DArray",
                                   "Texture2DMS",
                                   "Texture2DMSArray",
                                   "Texture3D",
                                   "TextureCube",
                                   "TextureCubeArray",
                                   "TriangleStream",
                                   "VFACE",
                                   "VPOS",
                                   "VertexShader",
                                   "abort",
                                   "allow_uav_condition",
                                   "asdouble",
                                   "asfloat",
                                   "asint",
                                   "asm",
                                   "asm_fragment",
                                   "asuint",
                                   "auto",
                                   "bool",
                                   "bool1",
                                   "bool1x1",
                                   "bool1x2",
                                   "bool1x3",
                                   "bool1x4",
                                   "bool2",
                                   "bool2x1",
                                   "bool2x2",
                                   "bool2x3",
                                   "bool2x4",
                                   "bool3",
                                   "bool3x1",
                                   "bool3x2",
                                   "bool3x3",
                                   "bool3x4",
                                   "bool4",
                                   "bool4x1",
                                   "bool4x2",
                                   "bool4x3",
                                   "bool4x4",
                                   "branch",
                                   "break",
                                   "call",
                                   "case",
                                   "catch",
                                   "cbuffer",
                                   "centroid",
                                   "char",
                                   "class",
                                   "clip",
                                   "column_major",
                                   "compile_fragment",
                                   "const",
                                   "const_cast",
                                   "continue",
                                   "countbits",
                                   "ddx",
                                   "ddx_coarse",
                                   "ddx_fine",
                                   "ddy",
                                   "ddy_coarse",
                                   "ddy_fine",
                                   "degrees",
                                   "delete",
                                   "discard",
                                   "do",
                                   "double",
                                   "double1",
                                   "double1x1",
                                   "double1x2",
                                   "double1x3",
                                   "double1x4",
                                   "double2",
                                   "double2x1",
                                   "double2x2",
                                   "double2x3",
                                   "double2x4",
                                   "double3",
                                   "double3x1",
                                   "double3x2",
                                   "double3x3",
                                   "double3x4",
                                   "double4",
                                   "double4x1",
                                   "double4x2",
                                   "double4x3",
                                   "double4x4",
                                   "dst",
                                   "dword",
                                   "dword1",
                                   "dword1x1",
                                   "dword1x2",
                                   "dword1x3",
                                   "dword1x4",
                                   "dword2",
                                   "dword2x1",
                                   "dword2x2",
                                   "dword2x3",
                                   "dword2x4",
                                   "dword3",
                                   "dword3x1",
                                   "dword3x2",
                                   "dword3x3",
                                   "dword3x4",
                                   "dword4",
                                   "dword4x1",
                                   "dword4x2",
                                   "dword4x3",
                                   "dword4x4",
                                   "dynamic_cast",
                                   "else",
                                   "enum",
                                   "errorf",
                                   "explicit",
                                   "export",
                                   "extern",
                                   "f16to32",
                                   "f32tof16",
                                   "false",
                                   "fastopt",
                                   "firstbithigh",
                                   "firstbitlow",
                                   "flatten",
                                   "float",
                                   "float1",
                                   "float1x1",
                                   "float1x2",
                                   "float1x3",
                                   "float1x4",
                                   "float2",
                                   "float2x1",
                                   "float2x2",
                                   "float2x3",
                                   "float2x4",
                                   "float3",
                                   "float3x1",
                                   "float3x2",
                                   "float3x3",
                                   "float3x4",
                                   "float4",
                                   "float4x1",
                                   "float4x2",
                                   "float4x3",
                                   "float4x4",
                                   "fmod",
                                   "for",
                                   "forcecase",
                                   "frac",
                                   "friend",
                                   "fxgroup",
                                   "goto",
                                   "groupshared",
                                   "half",
                                   "half1",
                                   "half1x1",
                                   "half1x2",
                                   "half1x3",
                                   "half1x4",
                                   "half2",
                                   "half2x1",
                                   "half2x2",
                                   "half2x3",
                                   "half2x4",
                                   "half3",
                                   "half3x1",
                                   "half3x2",
                                   "half3x3",
                                   "half3x4",
                                   "half4",
                                   "half4x1",
                                   "half4x2",
                                   "half4x3",
                                   "half4x4",
                                   "if",
                                   "in",
                                   "inline",
                                   "inout",
                                   "int",
                                   "int1",
                                   "int1x1",
                                   "int1x2",
                                   "int1x3",
                                   "int1x4",
                                   "int2",
                                   "int2x1",
                                   "int2x2",
                                   "int2x3",
                                   "int2x4",
                                   "int3",
                                   "int3x1",
                                   "int3x2",
                                   "int3x3",
                                   "int3x4",
                                   "int4",
                                   "int4x1",
                                   "int4x2",
                                   "int4x3",
                                   "int4x4",
                                   "interface",
                                   "isfinite",
                                   "isinf",
                                   "isnan",
                                   "lerp",
                                   "lineadj",
                                   "linear",
                                   "lit",
                                   "log10",
                                   "long",
                                   "loop",
                                   "mad",
                                   "matrix",
                                   "min10float",
                                   "min10float1",
                                   "min10float1x1",
                                   "min10float1x2",
                                   "min10float1x3",
                                   "min10float1x4",
                                   "min10float2",
                                   "min10float2x1",
                                   "min10float2x2",
                                   "min10float2x3",
                                   "min10float2x4",
                                   "min10float3",
                                   "min10float3x1",
                                   "min10float3x2",
                                   "min10float3x3",
                                   "min10float3x4",
                                   "min10float4",
                                   "min10float4x1",
                                   "min10float4x2",
                                   "min10float4x3",
                                   "min10float4x4",
                                   "min12int",
                                   "min12int1",
                                   "min12int1x1",
                                   "min12int1x2",
                                   "min12int1x3",
                                   "min12int1x4",
                                   "min12int2",
                                   "min12int2x1",
                                   "min12int2x2",
                                   "min12int2x3",
                                   "min12int2x4",
                                   "min12int3",
                                   "min12int3x1",
                                   "min12int3x2",
                                   "min12int3x3",
                                   "min12int3x4",
                                   "min12int4",
                                   "min12int4x1",
                                   "min12int4x2",
                                   "min12int4x3",
                                   "min12int4x4",
                                   "min16float",
                                   "min16float1",
                                   "min16float1x1",
                                   "min16float1x2",
                                   "min16float1x3",
                                   "min16float1x4",
                                   "min16float2",
                                   "min16float2x1",
                                   "min16float2x2",
                                   "min16float2x3",
                                   "min16float2x4",
                                   "min16float3",
                                   "min16float3x1",
                                   "min16float3x2",
                                   "min16float3x3",
                                   "min16float3x4",
                                   "min16float4",
                                   "min16float4x1",
                                   "min16float4x2",
                                   "min16float4x3",
                                   "min16float4x4",
                                   "min16int",
                                   "min16int1",
                                   "min16int1x1",
                                   "min16int1x2",
                                   "min16int1x3",
                                   "min16int1x4",
                                   "min16int2",
                                   "min16int2x1",
                                   "min16int2x2",
                                   "min16int2x3",
                                   "min16int2x4",
                                   "min16int3",
                                   "min16int3x1",
                                   "min16int3x2",
                                   "min16int3x3",
                                   "min16int3x4",
                                   "min16int4",
                                   "min16int4x1",
                                   "min16int4x2",
                                   "min16int4x3",
                                   "min16int4x4",
                                   "min16uint",
                                   "min16uint1",
                                   "min16uint1x1",
                                   "min16uint1x2",
                                   "min16uint1x3",
                                   "min16uint1x4",
                                   "min16uint2",
                                   "min16uint2x1",
                                   "min16uint2x2",
                                   "min16uint2x3",
                                   "min16uint2x4",
                                   "min16uint3",
                                   "min16uint3x1",
                                   "min16uint3x2",
                                   "min16uint3x3",
                                   "min16uint3x4",
                                   "min16uint4",
                                   "min16uint4x1",
                                   "min16uint4x2",
                                   "min16uint4x3",
                                   "min16uint4x4",
                                   "msad4",
                                   "mul",
                                   "mutable",
                                   "namespace",
                                   "new",
                                   "nointerpolation",
                                   "noise",
                                   "noperspective",
                                   "numthreads",
                                   "operator",
                                   "out",
                                   "packoffset",
                                   "pass",
                                   "pixelfragment",
                                   "pixelshader",
                                   "point",
                                   "precise",
                                   "printf",
                                   "private",
                                   "protected",
                                   "public",
                                   "radians",
                                   "rcp",
                                   "refract",
                                   "register",
                                   "reinterpret_cast",
                                   "return",
                                   "row_major",
                                   "rsqrt",
                                   "sample",
                                   "sampler",
                                   "sampler1D",
                                   "sampler2D",
                                   "sampler3D",
                                   "samplerCUBE",
                                   "sampler_state",
                                   "saturate",
                                   "shared",
                                   "short",
                                   "signed",
                                   "sincos",
                                   "sizeof",
                                   "snorm",
                                   "stateblock",
                                   "stateblock_state",
                                   "static",
                                   "static_cast",
                                   "string",
                                   "struct",
                                   "switch",
                                   "tbuffer",
                                   "technique",
                                   "technique10",
                                   "technique11",
                                   "template",
                                   "tex1D",
                                   "tex1Dbias",
                                   "tex1Dgrad",
                                   "tex1Dlod",
                                   "tex1Dproj",
                                   "tex2D",
                                   "tex2Dbias",
                                   "tex2Dgrad",
                                   "tex2Dlod",
                                   "tex2Dproj",
                                   "tex3D",
                                   "tex3Dbias",
                                   "tex3Dgrad",
                                   "tex3Dlod",
                                   "tex3Dproj",
                                   "texCUBE",
                                   "texCUBEbias",
                                   "texCUBEgrad",
                                   "texCUBElod",
                                   "texCUBEproj",
                                   "texture",
                                   "texture1D",
                                   "texture1DArray",
                                   "texture2D",
                                   "texture2DArray",
                                   "texture2DMS",
                                   "texture2DMSArray",
                                   "texture3D",
                                   "textureCube",
                                   "textureCubeArray",
                                   "this",
                                   "throw",
                                   "transpose",
                                   "triangle",
                                   "triangleadj",
                                   "true",
                                   "try",
                                   "typedef",
                                   "typename",
                                   "uint",
                                   "uint1",
                                   "uint1x1",
                                   "uint1x2",
                                   "uint1x3",
                                   "uint1x4",
                                   "uint2",
                                   "uint2x1",
                                   "uint2x2",
                                   "uint2x3",
                                   "uint2x4",
                                   "uint3",
                                   "uint3x1",
                                   "uint3x2",
                                   "uint3x3",
                                   "uint3x4",
                                   "uint4",
                                   "uint4x1",
                                   "uint4x2",
                                   "uint4x3",
                                   "uint4x4",
                                   "uniform",
                                   "union",
                                   "unorm",
                                   "unroll",
                                   "unsigned",
                                   "using",
                                   "vector",
                                   "vertexfragment",
                                   "vertexshader",
                                   "virtual",
                                   "void",
                                   "volatile",
                                   "while"};

}  // namespace

Hlsl::Hlsl() = default;
Hlsl::~Hlsl() = default;

Transform::Output Hlsl::Run(const Program* in, const DataMap& data) {
  Manager manager;
  manager.Add<CanonicalizeEntryPointIO>();
  manager.Add<DecomposeStorageAccess>();
  manager.Add<CalculateArrayLength>();
  auto out = manager.Run(in, data);
  if (!out.program.IsValid()) {
    return out;
  }

  ProgramBuilder builder;
  CloneContext ctx(&builder, &out.program);
  PromoteInitializersToConstVar(ctx);
  AddEmptyEntryPoint(ctx);
  RenameReservedKeywords(&ctx, kReservedKeywords);
  ctx.Clone();
  return Output{Program(std::move(builder))};
}

void Hlsl::PromoteInitializersToConstVar(CloneContext& ctx) const {
  // Scan the AST nodes for array and structure initializers which
  // need to be promoted to their own constant declaration.

  // Note: Correct handling of nested expressions is guaranteed due to the
  // depth-first traversal of the ast::Node::Clone() methods:
  //
  // The inner-most initializers are traversed first, and they are hoisted
  // to const variables declared just above the statement of use. The outer
  // initializer will then be hoisted, inserting themselves between the
  // inner declaration and the statement of use. This pattern applies correctly
  // to any nested depth.
  //
  // Depth-first traversal of the AST is guaranteed because AST nodes are fully
  // immutable and require their children to be constructed first so their
  // pointer can be passed to the parent's constructor.

  for (auto* src_node : ctx.src->ASTNodes().Objects()) {
    if (auto* src_init = src_node->As<ast::TypeConstructorExpression>()) {
      auto* src_sem_expr = ctx.src->Sem().Get(src_init);
      if (!src_sem_expr) {
        TINT_ICE(ctx.dst->Diagnostics())
            << "ast::TypeConstructorExpression has no semantic expression node";
        continue;
      }
      auto* src_sem_stmt = src_sem_expr->Stmt();
      if (!src_sem_stmt) {
        // Expression is outside of a statement. This usually means the
        // expression is part of a global (module-scope) constant declaration.
        // These must be constexpr, and so cannot contain the type of
        // expressions that must be sanitized.
        continue;
      }
      auto* src_stmt = src_sem_stmt->Declaration();

      if (auto* src_var_decl = src_stmt->As<ast::VariableDeclStatement>()) {
        if (src_var_decl->variable()->constructor() == src_init) {
          // This statement is just a variable declaration with the initializer
          // as the constructor value. This is what we're attempting to
          // transform to, and so ignore.
          continue;
        }
      }

      auto* src_ty = src_sem_expr->Type();
      if (src_ty->IsAnyOf<type::Array, type::Struct>()) {
        // Create a new symbol for the constant
        auto dst_symbol = ctx.dst->Symbols().New();
        // Clone the type
        auto* dst_ty = ctx.Clone(src_ty);
        // Clone the initializer
        auto* dst_init = ctx.Clone(src_init);
        // Construct the constant that holds the hoisted initializer
        auto* dst_var = ctx.dst->Const(dst_symbol, dst_ty, dst_init);
        // Construct the variable declaration statement
        auto* dst_var_decl =
            ctx.dst->create<ast::VariableDeclStatement>(dst_var);
        // Construct the identifier for referencing the constant
        auto* dst_ident = ctx.dst->Expr(dst_symbol);

        // Insert the constant before the usage
        ctx.InsertBefore(src_sem_stmt->Block()->statements(), src_stmt,
                         dst_var_decl);
        // Replace the inlined initializer with a reference to the constant
        ctx.Replace(src_init, dst_ident);
      }
    }
  }
}

void Hlsl::AddEmptyEntryPoint(CloneContext& ctx) const {
  for (auto* func : ctx.src->AST().Functions()) {
    if (func->IsEntryPoint()) {
      return;
    }
  }
  ctx.dst->Func(
      "_tint_unused_entry_point", {}, ctx.dst->ty.void_(), {},
      {ctx.dst->create<ast::StageDecoration>(ast::PipelineStage::kVertex)});
}

}  // namespace transform
}  // namespace tint
