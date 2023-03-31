struct Inner {
  int scalar_i32;
  float scalar_f32;
};

ByteAddressBuffer sb : register(t0);

float2x2 sb_load_12(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
}

float2x3 sb_load_13(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x4 sb_load_14(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float3x2 sb_load_15(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float3x3 sb_load_16(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x4 sb_load_17(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float4x2 sb_load_18(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float4x3 sb_load_19(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x4 sb_load_20(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

typedef float3 sb_load_21_ret[2];
sb_load_21_ret sb_load_21(uint offset) {
  float3 arr[2] = (float3[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr[i] = asfloat(sb.Load3((offset + (i * 16u))));
    }
  }
  return arr;
}

Inner sb_load_22(uint offset) {
  const Inner tint_symbol = {asint(sb.Load((offset + 0u))), asfloat(sb.Load((offset + 4u)))};
  return tint_symbol;
}

typedef Inner sb_load_23_ret[4];
sb_load_23_ret sb_load_23(uint offset) {
  Inner arr_1[4] = (Inner[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = sb_load_22((offset + (i_1 * 8u)));
    }
  }
  return arr_1;
}

[numthreads(1, 1, 1)]
void main() {
  const float scalar_f32 = asfloat(sb.Load(0u));
  const int scalar_i32 = asint(sb.Load(4u));
  const uint scalar_u32 = sb.Load(8u);
  const float2 vec2_f32 = asfloat(sb.Load2(16u));
  const int2 vec2_i32 = asint(sb.Load2(24u));
  const uint2 vec2_u32 = sb.Load2(32u);
  const float3 vec3_f32 = asfloat(sb.Load3(48u));
  const int3 vec3_i32 = asint(sb.Load3(64u));
  const uint3 vec3_u32 = sb.Load3(80u);
  const float4 vec4_f32 = asfloat(sb.Load4(96u));
  const int4 vec4_i32 = asint(sb.Load4(112u));
  const uint4 vec4_u32 = sb.Load4(128u);
  const float2x2 mat2x2_f32 = sb_load_12(144u);
  const float2x3 mat2x3_f32 = sb_load_13(160u);
  const float2x4 mat2x4_f32 = sb_load_14(192u);
  const float3x2 mat3x2_f32 = sb_load_15(224u);
  const float3x3 mat3x3_f32 = sb_load_16(256u);
  const float3x4 mat3x4_f32 = sb_load_17(304u);
  const float4x2 mat4x2_f32 = sb_load_18(352u);
  const float4x3 mat4x3_f32 = sb_load_19(384u);
  const float4x4 mat4x4_f32 = sb_load_20(448u);
  const float3 arr2_vec3_f32[2] = sb_load_21(512u);
  const Inner struct_inner = sb_load_22(544u);
  const Inner array_struct_inner[4] = sb_load_23(552u);
  return;
}
