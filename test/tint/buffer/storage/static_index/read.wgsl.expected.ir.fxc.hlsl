struct Inner {
  int scalar_i32;
  float scalar_f32;
};


ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

Inner v(uint offset) {
  int v_1 = asint(sb.Load((offset + 0u)));
  Inner v_2 = {v_1, asfloat(sb.Load((offset + 4u)))};
  return v_2;
}

typedef Inner ary_ret[4];
ary_ret v_3(uint offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      Inner v_6 = v((offset + (v_5 * 8u)));
      a[v_5] = v_6;
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  Inner v_7[4] = a;
  return v_7;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_8(uint offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 2u)) {
        break;
      }
      a[v_10] = asfloat(sb.Load3((offset + (v_10 * 16u))));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  float3 v_11[2] = a;
  return v_11;
}

float4x4 v_12(uint offset) {
  float4 v_13 = asfloat(sb.Load4((offset + 0u)));
  float4 v_14 = asfloat(sb.Load4((offset + 16u)));
  float4 v_15 = asfloat(sb.Load4((offset + 32u)));
  return float4x4(v_13, v_14, v_15, asfloat(sb.Load4((offset + 48u))));
}

float4x3 v_16(uint offset) {
  float3 v_17 = asfloat(sb.Load3((offset + 0u)));
  float3 v_18 = asfloat(sb.Load3((offset + 16u)));
  float3 v_19 = asfloat(sb.Load3((offset + 32u)));
  return float4x3(v_17, v_18, v_19, asfloat(sb.Load3((offset + 48u))));
}

float4x2 v_20(uint offset) {
  float2 v_21 = asfloat(sb.Load2((offset + 0u)));
  float2 v_22 = asfloat(sb.Load2((offset + 8u)));
  float2 v_23 = asfloat(sb.Load2((offset + 16u)));
  return float4x2(v_21, v_22, v_23, asfloat(sb.Load2((offset + 24u))));
}

float3x4 v_24(uint offset) {
  float4 v_25 = asfloat(sb.Load4((offset + 0u)));
  float4 v_26 = asfloat(sb.Load4((offset + 16u)));
  return float3x4(v_25, v_26, asfloat(sb.Load4((offset + 32u))));
}

float3x3 v_27(uint offset) {
  float3 v_28 = asfloat(sb.Load3((offset + 0u)));
  float3 v_29 = asfloat(sb.Load3((offset + 16u)));
  return float3x3(v_28, v_29, asfloat(sb.Load3((offset + 32u))));
}

float3x2 v_30(uint offset) {
  float2 v_31 = asfloat(sb.Load2((offset + 0u)));
  float2 v_32 = asfloat(sb.Load2((offset + 8u)));
  return float3x2(v_31, v_32, asfloat(sb.Load2((offset + 16u))));
}

float2x4 v_33(uint offset) {
  float4 v_34 = asfloat(sb.Load4((offset + 0u)));
  return float2x4(v_34, asfloat(sb.Load4((offset + 16u))));
}

float2x3 v_35(uint offset) {
  float3 v_36 = asfloat(sb.Load3((offset + 0u)));
  return float2x3(v_36, asfloat(sb.Load3((offset + 16u))));
}

float2x2 v_37(uint offset) {
  float2 v_38 = asfloat(sb.Load2((offset + 0u)));
  return float2x2(v_38, asfloat(sb.Load2((offset + 8u))));
}

[numthreads(1, 1, 1)]
void main() {
  float scalar_f32 = asfloat(sb.Load(0u));
  int scalar_i32 = asint(sb.Load(4u));
  uint scalar_u32 = sb.Load(8u);
  float2 vec2_f32 = asfloat(sb.Load2(16u));
  int2 vec2_i32 = asint(sb.Load2(24u));
  uint2 vec2_u32 = sb.Load2(32u);
  float3 vec3_f32 = asfloat(sb.Load3(48u));
  int3 vec3_i32 = asint(sb.Load3(64u));
  uint3 vec3_u32 = sb.Load3(80u);
  float4 vec4_f32 = asfloat(sb.Load4(96u));
  int4 vec4_i32 = asint(sb.Load4(112u));
  uint4 vec4_u32 = sb.Load4(128u);
  float2x2 mat2x2_f32 = v_37(144u);
  float2x3 mat2x3_f32 = v_35(160u);
  float2x4 mat2x4_f32 = v_33(192u);
  float3x2 mat3x2_f32 = v_30(224u);
  float3x3 mat3x3_f32 = v_27(256u);
  float3x4 mat3x4_f32 = v_24(304u);
  float4x2 mat4x2_f32 = v_20(352u);
  float4x3 mat4x3_f32 = v_16(384u);
  float4x4 mat4x4_f32 = v_12(448u);
  float3 arr2_vec3_f32[2] = v_8(512u);
  Inner struct_inner = v(544u);
  Inner array_struct_inner[4] = v_3(552u);
  int v_39 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_40 = (v_39 + int(scalar_u32));
  int v_41 = ((v_40 + tint_f32_to_i32(vec2_f32[0u])) + vec2_i32[0u]);
  int v_42 = (v_41 + int(vec2_u32[0u]));
  int v_43 = ((v_42 + tint_f32_to_i32(vec3_f32[1u])) + vec3_i32[1u]);
  int v_44 = (v_43 + int(vec3_u32[1u]));
  int v_45 = ((v_44 + tint_f32_to_i32(vec4_f32[2u])) + vec4_i32[2u]);
  int v_46 = (v_45 + int(vec4_u32[2u]));
  int v_47 = (v_46 + tint_f32_to_i32(mat2x2_f32[int(0)][0u]));
  int v_48 = (v_47 + tint_f32_to_i32(mat2x3_f32[int(0)][0u]));
  int v_49 = (v_48 + tint_f32_to_i32(mat2x4_f32[int(0)][0u]));
  int v_50 = (v_49 + tint_f32_to_i32(mat3x2_f32[int(0)][0u]));
  int v_51 = (v_50 + tint_f32_to_i32(mat3x3_f32[int(0)][0u]));
  int v_52 = (v_51 + tint_f32_to_i32(mat3x4_f32[int(0)][0u]));
  int v_53 = (v_52 + tint_f32_to_i32(mat4x2_f32[int(0)][0u]));
  int v_54 = (v_53 + tint_f32_to_i32(mat4x3_f32[int(0)][0u]));
  int v_55 = (v_54 + tint_f32_to_i32(mat4x4_f32[int(0)][0u]));
  s.Store(0u, asuint((((v_55 + tint_f32_to_i32(arr2_vec3_f32[int(0)][0u])) + struct_inner.scalar_i32) + array_struct_inner[int(0)].scalar_i32)));
}

