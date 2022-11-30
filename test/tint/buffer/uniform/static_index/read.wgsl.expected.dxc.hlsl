struct Inner {
  int scalar_i32;
  float scalar_f32;
};

cbuffer cbuffer_ub : register(b0, space0) {
  uint4 ub[44];
};

float2x2 tint_symbol_12(uint4 buffer[44], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = buffer[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = buffer[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

float2x3 tint_symbol_13(uint4 buffer[44], uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 16u)) / 4;
  return float2x3(asfloat(buffer[scalar_offset_2 / 4].xyz), asfloat(buffer[scalar_offset_3 / 4].xyz));
}

float2x4 tint_symbol_14(uint4 buffer[44], uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 16u)) / 4;
  return float2x4(asfloat(buffer[scalar_offset_4 / 4]), asfloat(buffer[scalar_offset_5 / 4]));
}

float3x2 tint_symbol_15(uint4 buffer[44], uint offset) {
  const uint scalar_offset_6 = ((offset + 0u)) / 4;
  uint4 ubo_load_2 = buffer[scalar_offset_6 / 4];
  const uint scalar_offset_7 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = buffer[scalar_offset_7 / 4];
  const uint scalar_offset_8 = ((offset + 16u)) / 4;
  uint4 ubo_load_4 = buffer[scalar_offset_8 / 4];
  return float3x2(asfloat(((scalar_offset_6 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_7 & 2) ? ubo_load_3.zw : ubo_load_3.xy)), asfloat(((scalar_offset_8 & 2) ? ubo_load_4.zw : ubo_load_4.xy)));
}

float3x3 tint_symbol_16(uint4 buffer[44], uint offset) {
  const uint scalar_offset_9 = ((offset + 0u)) / 4;
  const uint scalar_offset_10 = ((offset + 16u)) / 4;
  const uint scalar_offset_11 = ((offset + 32u)) / 4;
  return float3x3(asfloat(buffer[scalar_offset_9 / 4].xyz), asfloat(buffer[scalar_offset_10 / 4].xyz), asfloat(buffer[scalar_offset_11 / 4].xyz));
}

float3x4 tint_symbol_17(uint4 buffer[44], uint offset) {
  const uint scalar_offset_12 = ((offset + 0u)) / 4;
  const uint scalar_offset_13 = ((offset + 16u)) / 4;
  const uint scalar_offset_14 = ((offset + 32u)) / 4;
  return float3x4(asfloat(buffer[scalar_offset_12 / 4]), asfloat(buffer[scalar_offset_13 / 4]), asfloat(buffer[scalar_offset_14 / 4]));
}

float4x2 tint_symbol_18(uint4 buffer[44], uint offset) {
  const uint scalar_offset_15 = ((offset + 0u)) / 4;
  uint4 ubo_load_5 = buffer[scalar_offset_15 / 4];
  const uint scalar_offset_16 = ((offset + 8u)) / 4;
  uint4 ubo_load_6 = buffer[scalar_offset_16 / 4];
  const uint scalar_offset_17 = ((offset + 16u)) / 4;
  uint4 ubo_load_7 = buffer[scalar_offset_17 / 4];
  const uint scalar_offset_18 = ((offset + 24u)) / 4;
  uint4 ubo_load_8 = buffer[scalar_offset_18 / 4];
  return float4x2(asfloat(((scalar_offset_15 & 2) ? ubo_load_5.zw : ubo_load_5.xy)), asfloat(((scalar_offset_16 & 2) ? ubo_load_6.zw : ubo_load_6.xy)), asfloat(((scalar_offset_17 & 2) ? ubo_load_7.zw : ubo_load_7.xy)), asfloat(((scalar_offset_18 & 2) ? ubo_load_8.zw : ubo_load_8.xy)));
}

float4x3 tint_symbol_19(uint4 buffer[44], uint offset) {
  const uint scalar_offset_19 = ((offset + 0u)) / 4;
  const uint scalar_offset_20 = ((offset + 16u)) / 4;
  const uint scalar_offset_21 = ((offset + 32u)) / 4;
  const uint scalar_offset_22 = ((offset + 48u)) / 4;
  return float4x3(asfloat(buffer[scalar_offset_19 / 4].xyz), asfloat(buffer[scalar_offset_20 / 4].xyz), asfloat(buffer[scalar_offset_21 / 4].xyz), asfloat(buffer[scalar_offset_22 / 4].xyz));
}

float4x4 tint_symbol_20(uint4 buffer[44], uint offset) {
  const uint scalar_offset_23 = ((offset + 0u)) / 4;
  const uint scalar_offset_24 = ((offset + 16u)) / 4;
  const uint scalar_offset_25 = ((offset + 32u)) / 4;
  const uint scalar_offset_26 = ((offset + 48u)) / 4;
  return float4x4(asfloat(buffer[scalar_offset_23 / 4]), asfloat(buffer[scalar_offset_24 / 4]), asfloat(buffer[scalar_offset_25 / 4]), asfloat(buffer[scalar_offset_26 / 4]));
}

typedef float3 tint_symbol_21_ret[2];
tint_symbol_21_ret tint_symbol_21(uint4 buffer[44], uint offset) {
  float3 arr[2] = (float3[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      const uint scalar_offset_27 = ((offset + (i * 16u))) / 4;
      arr[i] = asfloat(buffer[scalar_offset_27 / 4].xyz);
    }
  }
  return arr;
}

Inner tint_symbol_22(uint4 buffer[44], uint offset) {
  const uint scalar_offset_28 = ((offset + 0u)) / 4;
  const uint scalar_offset_29 = ((offset + 16u)) / 4;
  const Inner tint_symbol_24 = {asint(buffer[scalar_offset_28 / 4][scalar_offset_28 % 4]), asfloat(buffer[scalar_offset_29 / 4][scalar_offset_29 % 4])};
  return tint_symbol_24;
}

typedef Inner tint_symbol_23_ret[4];
tint_symbol_23_ret tint_symbol_23(uint4 buffer[44], uint offset) {
  Inner arr_1[4] = (Inner[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = tint_symbol_22(buffer, (offset + (i_1 * 32u)));
    }
  }
  return arr_1;
}

[numthreads(1, 1, 1)]
void main() {
  const float scalar_f32 = asfloat(ub[0].x);
  const int scalar_i32 = asint(ub[0].y);
  const uint scalar_u32 = ub[0].z;
  const float2 vec2_f32 = asfloat(ub[1].xy);
  const int2 vec2_i32 = asint(ub[1].zw);
  const uint2 vec2_u32 = ub[2].xy;
  const float3 vec3_f32 = asfloat(ub[3].xyz);
  const int3 vec3_i32 = asint(ub[4].xyz);
  const uint3 vec3_u32 = ub[5].xyz;
  const float4 vec4_f32 = asfloat(ub[6]);
  const int4 vec4_i32 = asint(ub[7]);
  const uint4 vec4_u32 = ub[8];
  const float2x2 mat2x2_f32 = tint_symbol_12(ub, 144u);
  const float2x3 mat2x3_f32 = tint_symbol_13(ub, 160u);
  const float2x4 mat2x4_f32 = tint_symbol_14(ub, 192u);
  const float3x2 mat3x2_f32 = tint_symbol_15(ub, 224u);
  const float3x3 mat3x3_f32 = tint_symbol_16(ub, 256u);
  const float3x4 mat3x4_f32 = tint_symbol_17(ub, 304u);
  const float4x2 mat4x2_f32 = tint_symbol_18(ub, 352u);
  const float4x3 mat4x3_f32 = tint_symbol_19(ub, 384u);
  const float4x4 mat4x4_f32 = tint_symbol_20(ub, 448u);
  const float3 arr2_vec3_f32[2] = tint_symbol_21(ub, 512u);
  const Inner struct_inner = tint_symbol_22(ub, 544u);
  const Inner array_struct_inner[4] = tint_symbol_23(ub, 576u);
  return;
}
