struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};

ByteAddressBuffer sb : register(t0, space0);

float2x2 tint_symbol_16(ByteAddressBuffer buffer, uint offset) {
  return float2x2(asfloat(buffer.Load2((offset + 0u))), asfloat(buffer.Load2((offset + 8u))));
}

float2x3 tint_symbol_17(ByteAddressBuffer buffer, uint offset) {
  return float2x3(asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load3((offset + 16u))));
}

float2x4 tint_symbol_18(ByteAddressBuffer buffer, uint offset) {
  return float2x4(asfloat(buffer.Load4((offset + 0u))), asfloat(buffer.Load4((offset + 16u))));
}

float3x2 tint_symbol_19(ByteAddressBuffer buffer, uint offset) {
  return float3x2(asfloat(buffer.Load2((offset + 0u))), asfloat(buffer.Load2((offset + 8u))), asfloat(buffer.Load2((offset + 16u))));
}

float3x3 tint_symbol_20(ByteAddressBuffer buffer, uint offset) {
  return float3x3(asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load3((offset + 16u))), asfloat(buffer.Load3((offset + 32u))));
}

float3x4 tint_symbol_21(ByteAddressBuffer buffer, uint offset) {
  return float3x4(asfloat(buffer.Load4((offset + 0u))), asfloat(buffer.Load4((offset + 16u))), asfloat(buffer.Load4((offset + 32u))));
}

float4x2 tint_symbol_22(ByteAddressBuffer buffer, uint offset) {
  return float4x2(asfloat(buffer.Load2((offset + 0u))), asfloat(buffer.Load2((offset + 8u))), asfloat(buffer.Load2((offset + 16u))), asfloat(buffer.Load2((offset + 24u))));
}

float4x3 tint_symbol_23(ByteAddressBuffer buffer, uint offset) {
  return float4x3(asfloat(buffer.Load3((offset + 0u))), asfloat(buffer.Load3((offset + 16u))), asfloat(buffer.Load3((offset + 32u))), asfloat(buffer.Load3((offset + 48u))));
}

float4x4 tint_symbol_24(ByteAddressBuffer buffer, uint offset) {
  return float4x4(asfloat(buffer.Load4((offset + 0u))), asfloat(buffer.Load4((offset + 16u))), asfloat(buffer.Load4((offset + 32u))), asfloat(buffer.Load4((offset + 48u))));
}

matrix<float16_t, 2, 2> tint_symbol_25(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 2, 2>(buffer.Load<vector<float16_t, 2> >((offset + 0u)), buffer.Load<vector<float16_t, 2> >((offset + 4u)));
}

matrix<float16_t, 2, 3> tint_symbol_26(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 2, 3>(buffer.Load<vector<float16_t, 3> >((offset + 0u)), buffer.Load<vector<float16_t, 3> >((offset + 8u)));
}

matrix<float16_t, 2, 4> tint_symbol_27(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 2, 4>(buffer.Load<vector<float16_t, 4> >((offset + 0u)), buffer.Load<vector<float16_t, 4> >((offset + 8u)));
}

matrix<float16_t, 3, 2> tint_symbol_28(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 3, 2>(buffer.Load<vector<float16_t, 2> >((offset + 0u)), buffer.Load<vector<float16_t, 2> >((offset + 4u)), buffer.Load<vector<float16_t, 2> >((offset + 8u)));
}

matrix<float16_t, 3, 3> tint_symbol_29(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 3, 3>(buffer.Load<vector<float16_t, 3> >((offset + 0u)), buffer.Load<vector<float16_t, 3> >((offset + 8u)), buffer.Load<vector<float16_t, 3> >((offset + 16u)));
}

matrix<float16_t, 3, 4> tint_symbol_30(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 3, 4>(buffer.Load<vector<float16_t, 4> >((offset + 0u)), buffer.Load<vector<float16_t, 4> >((offset + 8u)), buffer.Load<vector<float16_t, 4> >((offset + 16u)));
}

matrix<float16_t, 4, 2> tint_symbol_31(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 4, 2>(buffer.Load<vector<float16_t, 2> >((offset + 0u)), buffer.Load<vector<float16_t, 2> >((offset + 4u)), buffer.Load<vector<float16_t, 2> >((offset + 8u)), buffer.Load<vector<float16_t, 2> >((offset + 12u)));
}

matrix<float16_t, 4, 3> tint_symbol_32(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 4, 3>(buffer.Load<vector<float16_t, 3> >((offset + 0u)), buffer.Load<vector<float16_t, 3> >((offset + 8u)), buffer.Load<vector<float16_t, 3> >((offset + 16u)), buffer.Load<vector<float16_t, 3> >((offset + 24u)));
}

matrix<float16_t, 4, 4> tint_symbol_33(ByteAddressBuffer buffer, uint offset) {
  return matrix<float16_t, 4, 4>(buffer.Load<vector<float16_t, 4> >((offset + 0u)), buffer.Load<vector<float16_t, 4> >((offset + 8u)), buffer.Load<vector<float16_t, 4> >((offset + 16u)), buffer.Load<vector<float16_t, 4> >((offset + 24u)));
}

typedef float3 tint_symbol_34_ret[2];
tint_symbol_34_ret tint_symbol_34(ByteAddressBuffer buffer, uint offset) {
  float3 arr[2] = (float3[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr[i] = asfloat(buffer.Load3((offset + (i * 16u))));
    }
  }
  return arr;
}

typedef matrix<float16_t, 4, 2> tint_symbol_35_ret[2];
tint_symbol_35_ret tint_symbol_35(ByteAddressBuffer buffer, uint offset) {
  matrix<float16_t, 4, 2> arr_1[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = tint_symbol_31(buffer, (offset + (i_1 * 16u)));
    }
  }
  return arr_1;
}

Inner tint_symbol_36(ByteAddressBuffer buffer, uint offset) {
  const Inner tint_symbol_38 = {asint(buffer.Load((offset + 0u))), asfloat(buffer.Load((offset + 4u))), buffer.Load<float16_t>((offset + 8u))};
  return tint_symbol_38;
}

typedef Inner tint_symbol_37_ret[4];
tint_symbol_37_ret tint_symbol_37(ByteAddressBuffer buffer, uint offset) {
  Inner arr_2[4] = (Inner[4])0;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      arr_2[i_2] = tint_symbol_36(buffer, (offset + (i_2 * 12u)));
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
  const float2x2 mat2x2_f32 = tint_symbol_16(sb, 168u);
  const float2x3 mat2x3_f32 = tint_symbol_17(sb, 192u);
  const float2x4 mat2x4_f32 = tint_symbol_18(sb, 224u);
  const float3x2 mat3x2_f32 = tint_symbol_19(sb, 256u);
  const float3x3 mat3x3_f32 = tint_symbol_20(sb, 288u);
  const float3x4 mat3x4_f32 = tint_symbol_21(sb, 336u);
  const float4x2 mat4x2_f32 = tint_symbol_22(sb, 384u);
  const float4x3 mat4x3_f32 = tint_symbol_23(sb, 416u);
  const float4x4 mat4x4_f32 = tint_symbol_24(sb, 480u);
  const matrix<float16_t, 2, 2> mat2x2_f16 = tint_symbol_25(sb, 544u);
  const matrix<float16_t, 2, 3> mat2x3_f16 = tint_symbol_26(sb, 552u);
  const matrix<float16_t, 2, 4> mat2x4_f16 = tint_symbol_27(sb, 568u);
  const matrix<float16_t, 3, 2> mat3x2_f16 = tint_symbol_28(sb, 584u);
  const matrix<float16_t, 3, 3> mat3x3_f16 = tint_symbol_29(sb, 600u);
  const matrix<float16_t, 3, 4> mat3x4_f16 = tint_symbol_30(sb, 624u);
  const matrix<float16_t, 4, 2> mat4x2_f16 = tint_symbol_31(sb, 648u);
  const matrix<float16_t, 4, 3> mat4x3_f16 = tint_symbol_32(sb, 664u);
  const matrix<float16_t, 4, 4> mat4x4_f16 = tint_symbol_33(sb, 696u);
  const float3 arr2_vec3_f32[2] = tint_symbol_34(sb, 736u);
  const matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = tint_symbol_35(sb, 768u);
  const Inner struct_inner = tint_symbol_36(sb, 800u);
  const Inner array_struct_inner[4] = tint_symbol_37(sb, 812u);
  return;
}
