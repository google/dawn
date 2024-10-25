struct main_inputs {
  uint idx : SV_GroupIndex;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[272];
};
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

typedef float3 ary_ret[2];
ary_ret v(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 2u)) {
        break;
      }
      a[v_2] = asfloat(ub[((start_byte_offset + (v_2 * 16u)) / 16u)].xyz);
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  float3 v_3[2] = a;
  return v_3;
}

float4x4 v_4(uint start_byte_offset) {
  float4 v_5 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_6 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  float4 v_7 = asfloat(ub[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_5, v_6, v_7, asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_8(uint start_byte_offset) {
  float3 v_9 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_10 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_11 = asfloat(ub[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_9, v_10, v_11, asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_12(uint start_byte_offset) {
  uint4 v_13 = ub[(start_byte_offset / 16u)];
  float2 v_14 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_13.zw) : (v_13.xy)));
  uint4 v_15 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_16 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_15.zw) : (v_15.xy)));
  uint4 v_17 = ub[((16u + start_byte_offset) / 16u)];
  float2 v_18 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_17.zw) : (v_17.xy)));
  uint4 v_19 = ub[((24u + start_byte_offset) / 16u)];
  return float4x2(v_14, v_16, v_18, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_19.zw) : (v_19.xy))));
}

float3x4 v_20(uint start_byte_offset) {
  float4 v_21 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_22 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_21, v_22, asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_23(uint start_byte_offset) {
  float3 v_24 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_25 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_24, v_25, asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_26(uint start_byte_offset) {
  uint4 v_27 = ub[(start_byte_offset / 16u)];
  float2 v_28 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_27.zw) : (v_27.xy)));
  uint4 v_29 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_30 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_29.zw) : (v_29.xy)));
  uint4 v_31 = ub[((16u + start_byte_offset) / 16u)];
  return float3x2(v_28, v_30, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_31.zw) : (v_31.xy))));
}

float2x4 v_32(uint start_byte_offset) {
  float4 v_33 = asfloat(ub[(start_byte_offset / 16u)]);
  return float2x4(v_33, asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_34(uint start_byte_offset) {
  float3 v_35 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  return float2x3(v_35, asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_36(uint start_byte_offset) {
  uint4 v_37 = ub[(start_byte_offset / 16u)];
  float2 v_38 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_37.zw) : (v_37.xy)));
  uint4 v_39 = ub[((8u + start_byte_offset) / 16u)];
  return float2x2(v_38, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_39.zw) : (v_39.xy))));
}

void main_inner(uint idx) {
  uint v_40 = (544u * uint(idx));
  float scalar_f32 = asfloat(ub[(v_40 / 16u)][((v_40 % 16u) / 4u)]);
  uint v_41 = (4u + (544u * uint(idx)));
  int scalar_i32 = asint(ub[(v_41 / 16u)][((v_41 % 16u) / 4u)]);
  uint v_42 = (8u + (544u * uint(idx)));
  uint scalar_u32 = ub[(v_42 / 16u)][((v_42 % 16u) / 4u)];
  uint v_43 = (16u + (544u * uint(idx)));
  uint4 v_44 = ub[(v_43 / 16u)];
  float2 vec2_f32 = asfloat((((((v_43 % 16u) / 4u) == 2u)) ? (v_44.zw) : (v_44.xy)));
  uint v_45 = (24u + (544u * uint(idx)));
  uint4 v_46 = ub[(v_45 / 16u)];
  int2 vec2_i32 = asint((((((v_45 % 16u) / 4u) == 2u)) ? (v_46.zw) : (v_46.xy)));
  uint v_47 = (32u + (544u * uint(idx)));
  uint4 v_48 = ub[(v_47 / 16u)];
  uint2 vec2_u32 = (((((v_47 % 16u) / 4u) == 2u)) ? (v_48.zw) : (v_48.xy));
  uint v_49 = ((48u + (544u * uint(idx))) / 16u);
  float3 vec3_f32 = asfloat(ub[v_49].xyz);
  uint v_50 = ((64u + (544u * uint(idx))) / 16u);
  int3 vec3_i32 = asint(ub[v_50].xyz);
  uint v_51 = ((80u + (544u * uint(idx))) / 16u);
  uint3 vec3_u32 = ub[v_51].xyz;
  uint v_52 = ((96u + (544u * uint(idx))) / 16u);
  float4 vec4_f32 = asfloat(ub[v_52]);
  uint v_53 = ((112u + (544u * uint(idx))) / 16u);
  int4 vec4_i32 = asint(ub[v_53]);
  uint v_54 = ((128u + (544u * uint(idx))) / 16u);
  uint4 vec4_u32 = ub[v_54];
  float2x2 mat2x2_f32 = v_36((144u + (544u * uint(idx))));
  float2x3 mat2x3_f32 = v_34((160u + (544u * uint(idx))));
  float2x4 mat2x4_f32 = v_32((192u + (544u * uint(idx))));
  float3x2 mat3x2_f32 = v_26((224u + (544u * uint(idx))));
  float3x3 mat3x3_f32 = v_23((256u + (544u * uint(idx))));
  float3x4 mat3x4_f32 = v_20((304u + (544u * uint(idx))));
  float4x2 mat4x2_f32 = v_12((352u + (544u * uint(idx))));
  float4x3 mat4x3_f32 = v_8((384u + (544u * uint(idx))));
  float4x4 mat4x4_f32 = v_4((448u + (544u * uint(idx))));
  float3 arr2_vec3_f32[2] = v((512u + (544u * uint(idx))));
  int v_55 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_56 = (v_55 + int(scalar_u32));
  int v_57 = ((v_56 + tint_f32_to_i32(vec2_f32[0u])) + vec2_i32[0u]);
  int v_58 = (v_57 + int(vec2_u32[0u]));
  int v_59 = ((v_58 + tint_f32_to_i32(vec3_f32[1u])) + vec3_i32[1u]);
  int v_60 = (v_59 + int(vec3_u32[1u]));
  int v_61 = ((v_60 + tint_f32_to_i32(vec4_f32[2u])) + vec4_i32[2u]);
  int v_62 = (v_61 + int(vec4_u32[2u]));
  int v_63 = (v_62 + tint_f32_to_i32(mat2x2_f32[int(0)][0u]));
  int v_64 = (v_63 + tint_f32_to_i32(mat2x3_f32[int(0)][0u]));
  int v_65 = (v_64 + tint_f32_to_i32(mat2x4_f32[int(0)][0u]));
  int v_66 = (v_65 + tint_f32_to_i32(mat3x2_f32[int(0)][0u]));
  int v_67 = (v_66 + tint_f32_to_i32(mat3x3_f32[int(0)][0u]));
  int v_68 = (v_67 + tint_f32_to_i32(mat3x4_f32[int(0)][0u]));
  int v_69 = (v_68 + tint_f32_to_i32(mat4x2_f32[int(0)][0u]));
  int v_70 = (v_69 + tint_f32_to_i32(mat4x3_f32[int(0)][0u]));
  int v_71 = (v_70 + tint_f32_to_i32(mat4x4_f32[int(0)][0u]));
  s.Store(0u, asuint((v_71 + tint_f32_to_i32(arr2_vec3_f32[int(0)][0u]))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

