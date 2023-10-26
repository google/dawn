struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};

ByteAddressBuffer sb : register(t0);

float2x2 sb_load_16(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
}

float2x3 sb_load_17(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x4 sb_load_18(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float3x2 sb_load_19(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float3x3 sb_load_20(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x4 sb_load_21(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float4x2 sb_load_22(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float4x3 sb_load_23(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x4 sb_load_24(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

matrix<float16_t, 2, 2> sb_load_25(uint offset) {
  return matrix<float16_t, 2, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)));
}

matrix<float16_t, 2, 3> sb_load_26(uint offset) {
  return matrix<float16_t, 2, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)));
}

matrix<float16_t, 2, 4> sb_load_27(uint offset) {
  return matrix<float16_t, 2, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)));
}

matrix<float16_t, 3, 2> sb_load_28(uint offset) {
  return matrix<float16_t, 3, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)));
}

matrix<float16_t, 3, 3> sb_load_29(uint offset) {
  return matrix<float16_t, 3, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)));
}

matrix<float16_t, 3, 4> sb_load_30(uint offset) {
  return matrix<float16_t, 3, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)));
}

matrix<float16_t, 4, 2> sb_load_31(uint offset) {
  return matrix<float16_t, 4, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)), sb.Load<vector<float16_t, 2> >((offset + 12u)));
}

matrix<float16_t, 4, 3> sb_load_32(uint offset) {
  return matrix<float16_t, 4, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)), sb.Load<vector<float16_t, 3> >((offset + 24u)));
}

matrix<float16_t, 4, 4> sb_load_33(uint offset) {
  return matrix<float16_t, 4, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)), sb.Load<vector<float16_t, 4> >((offset + 24u)));
}

typedef float3 sb_load_34_ret[2];
sb_load_34_ret sb_load_34(uint offset) {
  float3 arr[2] = (float3[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr[i] = asfloat(sb.Load3((offset + (i * 16u))));
    }
  }
  return arr;
}

typedef matrix<float16_t, 4, 2> sb_load_35_ret[2];
sb_load_35_ret sb_load_35(uint offset) {
  matrix<float16_t, 4, 2> arr_1[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = sb_load_31((offset + (i_1 * 16u)));
    }
  }
  return arr_1;
}

Inner sb_load_36(uint offset) {
  const Inner tint_symbol = {asint(sb.Load((offset + 0u))), asfloat(sb.Load((offset + 4u))), sb.Load<float16_t>((offset + 8u))};
  return tint_symbol;
}

typedef Inner sb_load_37_ret[4];
sb_load_37_ret sb_load_37(uint offset) {
  Inner arr_2[4] = (Inner[4])0;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      arr_2[i_2] = sb_load_36((offset + (i_2 * 12u)));
    }
  }
  return arr_2;
}

[numthreads(1, 1, 1)]
void main() {
  const float scalar_f32 = asfloat(sb.Load(0u));
  const int scalar_i32 = asint(sb.Load(4u));
  const uint scalar_u32 = sb.Load(8u);
  const float16_t scalar_f16 = sb.Load<float16_t>(12u);
  const float2 vec2_f32 = asfloat(sb.Load2(16u));
  const int2 vec2_i32 = asint(sb.Load2(24u));
  const uint2 vec2_u32 = sb.Load2(32u);
  const vector<float16_t, 2> vec2_f16 = sb.Load<vector<float16_t, 2> >(40u);
  const float3 vec3_f32 = asfloat(sb.Load3(48u));
  const int3 vec3_i32 = asint(sb.Load3(64u));
  const uint3 vec3_u32 = sb.Load3(80u);
  const vector<float16_t, 3> vec3_f16 = sb.Load<vector<float16_t, 3> >(96u);
  const float4 vec4_f32 = asfloat(sb.Load4(112u));
  const int4 vec4_i32 = asint(sb.Load4(128u));
  const uint4 vec4_u32 = sb.Load4(144u);
  const vector<float16_t, 4> vec4_f16 = sb.Load<vector<float16_t, 4> >(160u);
  const float2x2 mat2x2_f32 = sb_load_16(168u);
  const float2x3 mat2x3_f32 = sb_load_17(192u);
  const float2x4 mat2x4_f32 = sb_load_18(224u);
  const float3x2 mat3x2_f32 = sb_load_19(256u);
  const float3x3 mat3x3_f32 = sb_load_20(288u);
  const float3x4 mat3x4_f32 = sb_load_21(336u);
  const float4x2 mat4x2_f32 = sb_load_22(384u);
  const float4x3 mat4x3_f32 = sb_load_23(416u);
  const float4x4 mat4x4_f32 = sb_load_24(480u);
  const matrix<float16_t, 2, 2> mat2x2_f16 = sb_load_25(544u);
  const matrix<float16_t, 2, 3> mat2x3_f16 = sb_load_26(552u);
  const matrix<float16_t, 2, 4> mat2x4_f16 = sb_load_27(568u);
  const matrix<float16_t, 3, 2> mat3x2_f16 = sb_load_28(584u);
  const matrix<float16_t, 3, 3> mat3x3_f16 = sb_load_29(600u);
  const matrix<float16_t, 3, 4> mat3x4_f16 = sb_load_30(624u);
  const matrix<float16_t, 4, 2> mat4x2_f16 = sb_load_31(648u);
  const matrix<float16_t, 4, 3> mat4x3_f16 = sb_load_32(664u);
  const matrix<float16_t, 4, 4> mat4x4_f16 = sb_load_33(696u);
  const float3 arr2_vec3_f32[2] = sb_load_34(736u);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = sb_load_35(768u);
  const Inner struct_inner = sb_load_36(800u);
  const Inner array_struct_inner[4] = sb_load_37(812u);
  return;
}
