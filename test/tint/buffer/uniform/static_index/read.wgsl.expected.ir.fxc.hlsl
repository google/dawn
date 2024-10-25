struct Inner {
  int scalar_i32;
  float scalar_f32;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[44];
};
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

Inner v(uint start_byte_offset) {
  int v_1 = asint(ub[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  Inner v_2 = {v_1, asfloat(ub[((16u + start_byte_offset) / 16u)][(((16u + start_byte_offset) % 16u) / 4u)])};
  return v_2;
}

typedef Inner ary_ret[4];
ary_ret v_3(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      Inner v_6 = v((start_byte_offset + (v_5 * 32u)));
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
ary_ret_1 v_8(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 2u)) {
        break;
      }
      a[v_10] = asfloat(ub[((start_byte_offset + (v_10 * 16u)) / 16u)].xyz);
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  float3 v_11[2] = a;
  return v_11;
}

float4x4 v_12(uint start_byte_offset) {
  float4 v_13 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_14 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  float4 v_15 = asfloat(ub[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_13, v_14, v_15, asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_16(uint start_byte_offset) {
  float3 v_17 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_18 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_19 = asfloat(ub[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_17, v_18, v_19, asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_20(uint start_byte_offset) {
  uint4 v_21 = ub[(start_byte_offset / 16u)];
  float2 v_22 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_21.zw) : (v_21.xy)));
  uint4 v_23 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_24 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_23.zw) : (v_23.xy)));
  uint4 v_25 = ub[((16u + start_byte_offset) / 16u)];
  float2 v_26 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint4 v_27 = ub[((24u + start_byte_offset) / 16u)];
  return float4x2(v_22, v_24, v_26, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_27.zw) : (v_27.xy))));
}

float3x4 v_28(uint start_byte_offset) {
  float4 v_29 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_30 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_29, v_30, asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_31(uint start_byte_offset) {
  float3 v_32 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_33 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_32, v_33, asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_34(uint start_byte_offset) {
  uint4 v_35 = ub[(start_byte_offset / 16u)];
  float2 v_36 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_35.zw) : (v_35.xy)));
  uint4 v_37 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_38 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_37.zw) : (v_37.xy)));
  uint4 v_39 = ub[((16u + start_byte_offset) / 16u)];
  return float3x2(v_36, v_38, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_39.zw) : (v_39.xy))));
}

float2x4 v_40(uint start_byte_offset) {
  float4 v_41 = asfloat(ub[(start_byte_offset / 16u)]);
  return float2x4(v_41, asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_42(uint start_byte_offset) {
  float3 v_43 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  return float2x3(v_43, asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_44(uint start_byte_offset) {
  uint4 v_45 = ub[(start_byte_offset / 16u)];
  float2 v_46 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_45.zw) : (v_45.xy)));
  uint4 v_47 = ub[((8u + start_byte_offset) / 16u)];
  return float2x2(v_46, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_47.zw) : (v_47.xy))));
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
  float2x2 mat2x2_f32 = v_44(144u);
  float2x3 mat2x3_f32 = v_42(160u);
  float2x4 mat2x4_f32 = v_40(192u);
  float3x2 mat3x2_f32 = v_34(224u);
  float3x3 mat3x3_f32 = v_31(256u);
  float3x4 mat3x4_f32 = v_28(304u);
  float4x2 mat4x2_f32 = v_20(352u);
  float4x3 mat4x3_f32 = v_16(384u);
  float4x4 mat4x4_f32 = v_12(448u);
  float3 arr2_vec3_f32[2] = v_8(512u);
  Inner struct_inner = v(544u);
  Inner array_struct_inner[4] = v_3(576u);
  int v_48 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_49 = (v_48 + int(scalar_u32));
  int v_50 = ((v_49 + tint_f32_to_i32(vec2_f32[0u])) + vec2_i32[0u]);
  int v_51 = (v_50 + int(vec2_u32[0u]));
  int v_52 = ((v_51 + tint_f32_to_i32(vec3_f32[1u])) + vec3_i32[1u]);
  int v_53 = (v_52 + int(vec3_u32[1u]));
  int v_54 = ((v_53 + tint_f32_to_i32(vec4_f32[2u])) + vec4_i32[2u]);
  int v_55 = (v_54 + int(vec4_u32[2u]));
  int v_56 = (v_55 + tint_f32_to_i32(mat2x2_f32[int(0)][0u]));
  int v_57 = (v_56 + tint_f32_to_i32(mat2x3_f32[int(0)][0u]));
  int v_58 = (v_57 + tint_f32_to_i32(mat2x4_f32[int(0)][0u]));
  int v_59 = (v_58 + tint_f32_to_i32(mat3x2_f32[int(0)][0u]));
  int v_60 = (v_59 + tint_f32_to_i32(mat3x3_f32[int(0)][0u]));
  int v_61 = (v_60 + tint_f32_to_i32(mat3x4_f32[int(0)][0u]));
  int v_62 = (v_61 + tint_f32_to_i32(mat4x2_f32[int(0)][0u]));
  int v_63 = (v_62 + tint_f32_to_i32(mat4x3_f32[int(0)][0u]));
  int v_64 = (v_63 + tint_f32_to_i32(mat4x4_f32[int(0)][0u]));
  s.Store(0u, asuint((((v_64 + tint_f32_to_i32(arr2_vec3_f32[int(0)][0u])) + struct_inner.scalar_i32) + array_struct_inner[int(0)].scalar_i32)));
}

