struct Inner {
  int scalar_i32;
  float scalar_f32;
};


ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

Inner v(uint offset) {
  Inner v_1 = {asint(sb.Load((offset + 0u))), asfloat(sb.Load((offset + 4u)))};
  return v_1;
}

typedef Inner ary_ret[4];
ary_ret v_2(uint offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 4u)) {
        break;
      }
      Inner v_4 = v((offset + (v_3 * 8u)));
      a[v_3] = v_4;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_5[4] = a;
  return v_5;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_6(uint offset) {
  float3 a[2] = (float3[2])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_7 = idx;
      if ((v_7 >= 2u)) {
        break;
      }
      a[v_7] = asfloat(sb.Load3((offset + (v_7 * 16u))));
      {
        idx = (idx + 1u);
      }
    }
  }
  float3 v_8[2] = a;
  return v_8;
}

float4x4 v_9(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

float4x3 v_10(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x2 v_11(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float3x4 v_12(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float3x3 v_13(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x2 v_14(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float2x4 v_15(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float2x3 v_16(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x2 v_17(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
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
  float2x2 mat2x2_f32 = v_17(144u);
  float2x3 mat2x3_f32 = v_16(160u);
  float2x4 mat2x4_f32 = v_15(192u);
  float3x2 mat3x2_f32 = v_14(224u);
  float3x3 mat3x3_f32 = v_13(256u);
  float3x4 mat3x4_f32 = v_12(304u);
  float4x2 mat4x2_f32 = v_11(352u);
  float4x3 mat4x3_f32 = v_10(384u);
  float4x4 mat4x4_f32 = v_9(448u);
  float3 arr2_vec3_f32[2] = v_6(512u);
  Inner struct_inner = v(544u);
  Inner array_struct_inner[4] = v_2(552u);
  int v_18 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_19 = asint((asuint(v_18) + asuint(int(scalar_u32))));
  int v_20 = asint((asuint(asint((asuint(v_19) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_21 = asint((asuint(v_20) + asuint(int(vec2_u32.x))));
  int v_22 = asint((asuint(asint((asuint(v_21) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_23 = asint((asuint(v_22) + asuint(int(vec3_u32.y))));
  int v_24 = asint((asuint(asint((asuint(v_23) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_25 = asint((asuint(v_24) + asuint(int(vec4_u32.z))));
  int v_26 = asint((asuint(v_25) + asuint(tint_f32_to_i32(mat2x2_f32[int(0)].x))));
  int v_27 = asint((asuint(v_26) + asuint(tint_f32_to_i32(mat2x3_f32[int(0)].x))));
  int v_28 = asint((asuint(v_27) + asuint(tint_f32_to_i32(mat2x4_f32[int(0)].x))));
  int v_29 = asint((asuint(v_28) + asuint(tint_f32_to_i32(mat3x2_f32[int(0)].x))));
  int v_30 = asint((asuint(v_29) + asuint(tint_f32_to_i32(mat3x3_f32[int(0)].x))));
  int v_31 = asint((asuint(v_30) + asuint(tint_f32_to_i32(mat3x4_f32[int(0)].x))));
  int v_32 = asint((asuint(v_31) + asuint(tint_f32_to_i32(mat4x2_f32[int(0)].x))));
  int v_33 = asint((asuint(v_32) + asuint(tint_f32_to_i32(mat4x3_f32[int(0)].x))));
  int v_34 = asint((asuint(v_33) + asuint(tint_f32_to_i32(mat4x4_f32[int(0)].x))));
  s.Store(0u, asuint(asint((asuint(asint((asuint(asint((asuint(v_34) + asuint(tint_f32_to_i32(arr2_vec3_f32[int(0)].x))))) + asuint(struct_inner.scalar_i32)))) + asuint(array_struct_inner[int(0)].scalar_i32)))));
}

