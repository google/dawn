struct Inner {
  int scalar_i32;
  float scalar_f32;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[44];
};
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

Inner v(uint start_byte_offset) {
  uint v_1 = (16u + start_byte_offset);
  Inner v_2 = {asint(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]), asfloat(ub[(v_1 / 16u)][((v_1 & 15u) >> 2u)])};
  return v_2;
}

typedef Inner ary_ret[4];
ary_ret v_3(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_4 = idx;
      if ((v_4 >= 4u)) {
        break;
      }
      Inner v_5 = v((start_byte_offset + (v_4 * 32u)));
      a[v_4] = v_5;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_6[4] = a;
  return v_6;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_7(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_8 = idx;
      if ((v_8 >= 2u)) {
        break;
      }
      a[v_8] = asfloat(ub[((start_byte_offset + (v_8 * 16u)) / 16u)].xyz);
      {
        idx = (idx + 1u);
      }
    }
  }
  float3 v_9[2] = a;
  return v_9;
}

float4x4 v_10(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_11(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_12(uint start_byte_offset) {
  uint4 v_13 = ub[(start_byte_offset / 16u)];
  uint v_14 = (8u + start_byte_offset);
  uint4 v_15 = ub[(v_14 / 16u)];
  uint v_16 = (16u + start_byte_offset);
  uint4 v_17 = ub[(v_16 / 16u)];
  uint v_18 = (24u + start_byte_offset);
  uint4 v_19 = ub[(v_18 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_13.zw, v_13.xy)), asfloat(select((((v_14 & 15u) >> 2u) == 2u), v_15.zw, v_15.xy)), asfloat(select((((v_16 & 15u) >> 2u) == 2u), v_17.zw, v_17.xy)), asfloat(select((((v_18 & 15u) >> 2u) == 2u), v_19.zw, v_19.xy)));
}

float3x4 v_20(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_21(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_22(uint start_byte_offset) {
  uint4 v_23 = ub[(start_byte_offset / 16u)];
  uint v_24 = (8u + start_byte_offset);
  uint4 v_25 = ub[(v_24 / 16u)];
  uint v_26 = (16u + start_byte_offset);
  uint4 v_27 = ub[(v_26 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_23.zw, v_23.xy)), asfloat(select((((v_24 & 15u) >> 2u) == 2u), v_25.zw, v_25.xy)), asfloat(select((((v_26 & 15u) >> 2u) == 2u), v_27.zw, v_27.xy)));
}

float2x4 v_28(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_29(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_30(uint start_byte_offset) {
  uint4 v_31 = ub[(start_byte_offset / 16u)];
  uint v_32 = (8u + start_byte_offset);
  uint4 v_33 = ub[(v_32 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_31.zw, v_31.xy)), asfloat(select((((v_32 & 15u) >> 2u) == 2u), v_33.zw, v_33.xy)));
}

[numthreads(1, 1, 1)]
void main() {
  float scalar_f32 = asfloat(ub[0u].x);
  int scalar_i32 = asint(ub[0u].y);
  uint scalar_u32 = ub[0u].z;
  float2 vec2_f32 = asfloat(ub[1u].xy);
  int2 vec2_i32 = asint(ub[1u].zw);
  uint2 vec2_u32 = ub[2u].xy;
  float3 vec3_f32 = asfloat(ub[3u].xyz);
  int3 vec3_i32 = asint(ub[4u].xyz);
  uint3 vec3_u32 = ub[5u].xyz;
  float4 vec4_f32 = asfloat(ub[6u]);
  int4 vec4_i32 = asint(ub[7u]);
  uint4 vec4_u32 = ub[8u];
  float2x2 mat2x2_f32 = v_30(144u);
  float2x3 mat2x3_f32 = v_29(160u);
  float2x4 mat2x4_f32 = v_28(192u);
  float3x2 mat3x2_f32 = v_22(224u);
  float3x3 mat3x3_f32 = v_21(256u);
  float3x4 mat3x4_f32 = v_20(304u);
  float4x2 mat4x2_f32 = v_12(352u);
  float4x3 mat4x3_f32 = v_11(384u);
  float4x4 mat4x4_f32 = v_10(448u);
  float3 arr2_vec3_f32[2] = v_7(512u);
  Inner struct_inner = v(544u);
  Inner array_struct_inner[4] = v_3(576u);
  int v_34 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_35 = asint((asuint(v_34) + asuint(int(scalar_u32))));
  int v_36 = asint((asuint(asint((asuint(v_35) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_37 = asint((asuint(v_36) + asuint(int(vec2_u32.x))));
  int v_38 = asint((asuint(asint((asuint(v_37) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_39 = asint((asuint(v_38) + asuint(int(vec3_u32.y))));
  int v_40 = asint((asuint(asint((asuint(v_39) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_41 = asint((asuint(v_40) + asuint(int(vec4_u32.z))));
  int v_42 = asint((asuint(v_41) + asuint(tint_f32_to_i32(mat2x2_f32[int(0)].x))));
  int v_43 = asint((asuint(v_42) + asuint(tint_f32_to_i32(mat2x3_f32[int(0)].x))));
  int v_44 = asint((asuint(v_43) + asuint(tint_f32_to_i32(mat2x4_f32[int(0)].x))));
  int v_45 = asint((asuint(v_44) + asuint(tint_f32_to_i32(mat3x2_f32[int(0)].x))));
  int v_46 = asint((asuint(v_45) + asuint(tint_f32_to_i32(mat3x3_f32[int(0)].x))));
  int v_47 = asint((asuint(v_46) + asuint(tint_f32_to_i32(mat3x4_f32[int(0)].x))));
  int v_48 = asint((asuint(v_47) + asuint(tint_f32_to_i32(mat4x2_f32[int(0)].x))));
  int v_49 = asint((asuint(v_48) + asuint(tint_f32_to_i32(mat4x3_f32[int(0)].x))));
  int v_50 = asint((asuint(v_49) + asuint(tint_f32_to_i32(mat4x4_f32[int(0)].x))));
  s.Store(0u, asuint(asint((asuint(asint((asuint(asint((asuint(v_50) + asuint(tint_f32_to_i32(arr2_vec3_f32[int(0)].x))))) + asuint(struct_inner.scalar_i32)))) + asuint(array_struct_inner[int(0)].scalar_i32)))));
}

