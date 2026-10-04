struct main_inputs {
  uint idx : SV_GroupIndex;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[272];
};
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

typedef float3 ary_ret[2];
ary_ret v(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 2u)) {
        break;
      }
      a[v_1] = asfloat(ub[((start_byte_offset + (v_1 * 16u)) / 16u)].xyz);
      {
        idx = (idx + 1u);
      }
    }
  }
  float3 v_2[2] = a;
  return v_2;
}

float4x4 v_3(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_4(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_5(uint start_byte_offset) {
  uint4 v_6 = ub[(start_byte_offset / 16u)];
  float2 v_7 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_6.zw) : (v_6.xy)));
  uint v_8 = (8u + start_byte_offset);
  uint4 v_9 = ub[(v_8 / 16u)];
  float2 v_10 = asfloat((((((v_8 & 15u) >> 2u) == 2u)) ? (v_9.zw) : (v_9.xy)));
  uint v_11 = (16u + start_byte_offset);
  uint4 v_12 = ub[(v_11 / 16u)];
  float2 v_13 = asfloat((((((v_11 & 15u) >> 2u) == 2u)) ? (v_12.zw) : (v_12.xy)));
  uint v_14 = (24u + start_byte_offset);
  uint4 v_15 = ub[(v_14 / 16u)];
  return float4x2(v_7, v_10, v_13, asfloat((((((v_14 & 15u) >> 2u) == 2u)) ? (v_15.zw) : (v_15.xy))));
}

float3x4 v_16(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_17(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_18(uint start_byte_offset) {
  uint4 v_19 = ub[(start_byte_offset / 16u)];
  float2 v_20 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_19.zw) : (v_19.xy)));
  uint v_21 = (8u + start_byte_offset);
  uint4 v_22 = ub[(v_21 / 16u)];
  float2 v_23 = asfloat((((((v_21 & 15u) >> 2u) == 2u)) ? (v_22.zw) : (v_22.xy)));
  uint v_24 = (16u + start_byte_offset);
  uint4 v_25 = ub[(v_24 / 16u)];
  return float3x2(v_20, v_23, asfloat((((((v_24 & 15u) >> 2u) == 2u)) ? (v_25.zw) : (v_25.xy))));
}

float2x4 v_26(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_27(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_28(uint start_byte_offset) {
  uint4 v_29 = ub[(start_byte_offset / 16u)];
  float2 v_30 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_29.zw) : (v_29.xy)));
  uint v_31 = (8u + start_byte_offset);
  uint4 v_32 = ub[(v_31 / 16u)];
  return float2x2(v_30, asfloat((((((v_31 & 15u) >> 2u) == 2u)) ? (v_32.zw) : (v_32.xy))));
}

void main_inner(uint idx) {
  uint v_33 = (idx * 544u);
  float scalar_f32 = asfloat(ub[(v_33 / 16u)][((v_33 & 15u) >> 2u)]);
  uint v_34 = (4u + (idx * 544u));
  int scalar_i32 = asint(ub[(v_34 / 16u)][((v_34 & 15u) >> 2u)]);
  uint v_35 = (8u + (idx * 544u));
  uint scalar_u32 = ub[(v_35 / 16u)][((v_35 & 15u) >> 2u)];
  uint v_36 = (16u + (idx * 544u));
  uint4 v_37 = ub[(v_36 / 16u)];
  float2 vec2_f32 = asfloat((((((v_36 & 15u) >> 2u) == 2u)) ? (v_37.zw) : (v_37.xy)));
  uint v_38 = (24u + (idx * 544u));
  uint4 v_39 = ub[(v_38 / 16u)];
  int2 vec2_i32 = asint((((((v_38 & 15u) >> 2u) == 2u)) ? (v_39.zw) : (v_39.xy)));
  uint v_40 = (32u + (idx * 544u));
  uint4 v_41 = ub[(v_40 / 16u)];
  uint2 vec2_u32 = (((((v_40 & 15u) >> 2u) == 2u)) ? (v_41.zw) : (v_41.xy));
  float3 vec3_f32 = asfloat(ub[((48u + (idx * 544u)) / 16u)].xyz);
  int3 vec3_i32 = asint(ub[((64u + (idx * 544u)) / 16u)].xyz);
  uint3 vec3_u32 = ub[((80u + (idx * 544u)) / 16u)].xyz;
  float4 vec4_f32 = asfloat(ub[((96u + (idx * 544u)) / 16u)]);
  int4 vec4_i32 = asint(ub[((112u + (idx * 544u)) / 16u)]);
  uint4 vec4_u32 = ub[((128u + (idx * 544u)) / 16u)];
  float2x2 mat2x2_f32 = v_28((144u + (idx * 544u)));
  float2x3 mat2x3_f32 = v_27((160u + (idx * 544u)));
  float2x4 mat2x4_f32 = v_26((192u + (idx * 544u)));
  float3x2 mat3x2_f32 = v_18((224u + (idx * 544u)));
  float3x3 mat3x3_f32 = v_17((256u + (idx * 544u)));
  float3x4 mat3x4_f32 = v_16((304u + (idx * 544u)));
  float4x2 mat4x2_f32 = v_5((352u + (idx * 544u)));
  float4x3 mat4x3_f32 = v_4((384u + (idx * 544u)));
  float4x4 mat4x4_f32 = v_3((448u + (idx * 544u)));
  float3 arr2_vec3_f32[2] = v((512u + (idx * 544u)));
  int v_42 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_43 = asint((asuint(v_42) + asuint(int(scalar_u32))));
  int v_44 = asint((asuint(asint((asuint(v_43) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_45 = asint((asuint(v_44) + asuint(int(vec2_u32.x))));
  int v_46 = asint((asuint(asint((asuint(v_45) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_47 = asint((asuint(v_46) + asuint(int(vec3_u32.y))));
  int v_48 = asint((asuint(asint((asuint(v_47) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_49 = asint((asuint(v_48) + asuint(int(vec4_u32.z))));
  int v_50 = asint((asuint(v_49) + asuint(tint_f32_to_i32(mat2x2_f32[int(0)].x))));
  int v_51 = asint((asuint(v_50) + asuint(tint_f32_to_i32(mat2x3_f32[int(0)].x))));
  int v_52 = asint((asuint(v_51) + asuint(tint_f32_to_i32(mat2x4_f32[int(0)].x))));
  int v_53 = asint((asuint(v_52) + asuint(tint_f32_to_i32(mat3x2_f32[int(0)].x))));
  int v_54 = asint((asuint(v_53) + asuint(tint_f32_to_i32(mat3x3_f32[int(0)].x))));
  int v_55 = asint((asuint(v_54) + asuint(tint_f32_to_i32(mat3x4_f32[int(0)].x))));
  int v_56 = asint((asuint(v_55) + asuint(tint_f32_to_i32(mat4x2_f32[int(0)].x))));
  int v_57 = asint((asuint(v_56) + asuint(tint_f32_to_i32(mat4x3_f32[int(0)].x))));
  int v_58 = asint((asuint(v_57) + asuint(tint_f32_to_i32(mat4x4_f32[int(0)].x))));
  s.Store(0u, asuint(asint((asuint(v_58) + asuint(tint_f32_to_i32(arr2_vec3_f32[int(0)].x))))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

