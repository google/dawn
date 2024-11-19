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
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_5(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_6(uint start_byte_offset) {
  uint4 v_7 = ub[(start_byte_offset / 16u)];
  float2 v_8 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint4 v_9 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_10 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy)));
  uint4 v_11 = ub[((16u + start_byte_offset) / 16u)];
  float2 v_12 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_11.zw) : (v_11.xy)));
  uint4 v_13 = ub[((24u + start_byte_offset) / 16u)];
  return float4x2(v_8, v_10, v_12, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_13.zw) : (v_13.xy))));
}

float3x4 v_14(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_15(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_16(uint start_byte_offset) {
  uint4 v_17 = ub[(start_byte_offset / 16u)];
  float2 v_18 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_17.zw) : (v_17.xy)));
  uint4 v_19 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_20 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_19.zw) : (v_19.xy)));
  uint4 v_21 = ub[((16u + start_byte_offset) / 16u)];
  return float3x2(v_18, v_20, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_21.zw) : (v_21.xy))));
}

float2x4 v_22(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_23(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_24(uint start_byte_offset) {
  uint4 v_25 = ub[(start_byte_offset / 16u)];
  float2 v_26 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_25.zw) : (v_25.xy)));
  uint4 v_27 = ub[((8u + start_byte_offset) / 16u)];
  return float2x2(v_26, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_27.zw) : (v_27.xy))));
}

void main_inner(uint idx) {
  uint v_28 = (544u * uint(idx));
  float scalar_f32 = asfloat(ub[(v_28 / 16u)][((v_28 % 16u) / 4u)]);
  uint v_29 = (4u + (544u * uint(idx)));
  int scalar_i32 = asint(ub[(v_29 / 16u)][((v_29 % 16u) / 4u)]);
  uint v_30 = (8u + (544u * uint(idx)));
  uint scalar_u32 = ub[(v_30 / 16u)][((v_30 % 16u) / 4u)];
  uint v_31 = (16u + (544u * uint(idx)));
  uint4 v_32 = ub[(v_31 / 16u)];
  float2 vec2_f32 = asfloat((((((v_31 % 16u) / 4u) == 2u)) ? (v_32.zw) : (v_32.xy)));
  uint v_33 = (24u + (544u * uint(idx)));
  uint4 v_34 = ub[(v_33 / 16u)];
  int2 vec2_i32 = asint((((((v_33 % 16u) / 4u) == 2u)) ? (v_34.zw) : (v_34.xy)));
  uint v_35 = (32u + (544u * uint(idx)));
  uint4 v_36 = ub[(v_35 / 16u)];
  uint2 vec2_u32 = (((((v_35 % 16u) / 4u) == 2u)) ? (v_36.zw) : (v_36.xy));
  uint v_37 = ((48u + (544u * uint(idx))) / 16u);
  float3 vec3_f32 = asfloat(ub[v_37].xyz);
  uint v_38 = ((64u + (544u * uint(idx))) / 16u);
  int3 vec3_i32 = asint(ub[v_38].xyz);
  uint v_39 = ((80u + (544u * uint(idx))) / 16u);
  uint3 vec3_u32 = ub[v_39].xyz;
  uint v_40 = ((96u + (544u * uint(idx))) / 16u);
  float4 vec4_f32 = asfloat(ub[v_40]);
  uint v_41 = ((112u + (544u * uint(idx))) / 16u);
  int4 vec4_i32 = asint(ub[v_41]);
  uint v_42 = ((128u + (544u * uint(idx))) / 16u);
  uint4 vec4_u32 = ub[v_42];
  float2x2 mat2x2_f32 = v_24((144u + (544u * uint(idx))));
  float2x3 mat2x3_f32 = v_23((160u + (544u * uint(idx))));
  float2x4 mat2x4_f32 = v_22((192u + (544u * uint(idx))));
  float3x2 mat3x2_f32 = v_16((224u + (544u * uint(idx))));
  float3x3 mat3x3_f32 = v_15((256u + (544u * uint(idx))));
  float3x4 mat3x4_f32 = v_14((304u + (544u * uint(idx))));
  float4x2 mat4x2_f32 = v_6((352u + (544u * uint(idx))));
  float4x3 mat4x3_f32 = v_5((384u + (544u * uint(idx))));
  float4x4 mat4x4_f32 = v_4((448u + (544u * uint(idx))));
  float3 arr2_vec3_f32[2] = v((512u + (544u * uint(idx))));
  int v_43 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_44 = (v_43 + int(scalar_u32));
  int v_45 = ((v_44 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_46 = (v_45 + int(vec2_u32.x));
  int v_47 = ((v_46 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_48 = (v_47 + int(vec3_u32.y));
  int v_49 = ((v_48 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_50 = (v_49 + int(vec4_u32.z));
  int v_51 = (v_50 + tint_f32_to_i32(mat2x2_f32[int(0)].x));
  int v_52 = (v_51 + tint_f32_to_i32(mat2x3_f32[int(0)].x));
  int v_53 = (v_52 + tint_f32_to_i32(mat2x4_f32[int(0)].x));
  int v_54 = (v_53 + tint_f32_to_i32(mat3x2_f32[int(0)].x));
  int v_55 = (v_54 + tint_f32_to_i32(mat3x3_f32[int(0)].x));
  int v_56 = (v_55 + tint_f32_to_i32(mat3x4_f32[int(0)].x));
  int v_57 = (v_56 + tint_f32_to_i32(mat4x2_f32[int(0)].x));
  int v_58 = (v_57 + tint_f32_to_i32(mat4x3_f32[int(0)].x));
  int v_59 = (v_58 + tint_f32_to_i32(mat4x4_f32[int(0)].x));
  s.Store(0u, asuint((v_59 + tint_f32_to_i32(arr2_vec3_f32[int(0)].x))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

