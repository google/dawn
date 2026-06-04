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
  float2 v_8 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_7.zw) : (v_7.xy)));
  uint v_9 = (8u + start_byte_offset);
  uint4 v_10 = ub[(v_9 / 16u)];
  float2 v_11 = asfloat((((((v_9 & 15u) >> 2u) == 2u)) ? (v_10.zw) : (v_10.xy)));
  uint v_12 = (16u + start_byte_offset);
  uint4 v_13 = ub[(v_12 / 16u)];
  float2 v_14 = asfloat((((((v_12 & 15u) >> 2u) == 2u)) ? (v_13.zw) : (v_13.xy)));
  uint v_15 = (24u + start_byte_offset);
  uint4 v_16 = ub[(v_15 / 16u)];
  return float4x2(v_8, v_11, v_14, asfloat((((((v_15 & 15u) >> 2u) == 2u)) ? (v_16.zw) : (v_16.xy))));
}

float3x4 v_17(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_18(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_19(uint start_byte_offset) {
  uint4 v_20 = ub[(start_byte_offset / 16u)];
  float2 v_21 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_20.zw) : (v_20.xy)));
  uint v_22 = (8u + start_byte_offset);
  uint4 v_23 = ub[(v_22 / 16u)];
  float2 v_24 = asfloat((((((v_22 & 15u) >> 2u) == 2u)) ? (v_23.zw) : (v_23.xy)));
  uint v_25 = (16u + start_byte_offset);
  uint4 v_26 = ub[(v_25 / 16u)];
  return float3x2(v_21, v_24, asfloat((((((v_25 & 15u) >> 2u) == 2u)) ? (v_26.zw) : (v_26.xy))));
}

float2x4 v_27(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_28(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_29(uint start_byte_offset) {
  uint4 v_30 = ub[(start_byte_offset / 16u)];
  float2 v_31 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_30.zw) : (v_30.xy)));
  uint v_32 = (8u + start_byte_offset);
  uint4 v_33 = ub[(v_32 / 16u)];
  return float2x2(v_31, asfloat((((((v_32 & 15u) >> 2u) == 2u)) ? (v_33.zw) : (v_33.xy))));
}

void main_inner(uint idx) {
  uint v_34 = (idx * 544u);
  float scalar_f32 = asfloat(ub[(v_34 / 16u)][((v_34 & 15u) >> 2u)]);
  uint v_35 = (4u + (idx * 544u));
  int scalar_i32 = asint(ub[(v_35 / 16u)][((v_35 & 15u) >> 2u)]);
  uint v_36 = (8u + (idx * 544u));
  uint scalar_u32 = ub[(v_36 / 16u)][((v_36 & 15u) >> 2u)];
  uint v_37 = (16u + (idx * 544u));
  uint4 v_38 = ub[(v_37 / 16u)];
  float2 vec2_f32 = asfloat((((((v_37 & 15u) >> 2u) == 2u)) ? (v_38.zw) : (v_38.xy)));
  uint v_39 = (24u + (idx * 544u));
  uint4 v_40 = ub[(v_39 / 16u)];
  int2 vec2_i32 = asint((((((v_39 & 15u) >> 2u) == 2u)) ? (v_40.zw) : (v_40.xy)));
  uint v_41 = (32u + (idx * 544u));
  uint4 v_42 = ub[(v_41 / 16u)];
  uint2 vec2_u32 = (((((v_41 & 15u) >> 2u) == 2u)) ? (v_42.zw) : (v_42.xy));
  float3 vec3_f32 = asfloat(ub[((48u + (idx * 544u)) / 16u)].xyz);
  int3 vec3_i32 = asint(ub[((64u + (idx * 544u)) / 16u)].xyz);
  uint3 vec3_u32 = ub[((80u + (idx * 544u)) / 16u)].xyz;
  float4 vec4_f32 = asfloat(ub[((96u + (idx * 544u)) / 16u)]);
  int4 vec4_i32 = asint(ub[((112u + (idx * 544u)) / 16u)]);
  uint4 vec4_u32 = ub[((128u + (idx * 544u)) / 16u)];
  float2x2 mat2x2_f32 = v_29((144u + (idx * 544u)));
  float2x3 mat2x3_f32 = v_28((160u + (idx * 544u)));
  float2x4 mat2x4_f32 = v_27((192u + (idx * 544u)));
  float3x2 mat3x2_f32 = v_19((224u + (idx * 544u)));
  float3x3 mat3x3_f32 = v_18((256u + (idx * 544u)));
  float3x4 mat3x4_f32 = v_17((304u + (idx * 544u)));
  float4x2 mat4x2_f32 = v_6((352u + (idx * 544u)));
  float4x3 mat4x3_f32 = v_5((384u + (idx * 544u)));
  float4x4 mat4x4_f32 = v_4((448u + (idx * 544u)));
  float3 arr2_vec3_f32[2] = v((512u + (idx * 544u)));
  int v_43 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_44 = asint((asuint(v_43) + asuint(int(scalar_u32))));
  int v_45 = asint((asuint(asint((asuint(v_44) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_46 = asint((asuint(v_45) + asuint(int(vec2_u32.x))));
  int v_47 = asint((asuint(asint((asuint(v_46) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_48 = asint((asuint(v_47) + asuint(int(vec3_u32.y))));
  int v_49 = asint((asuint(asint((asuint(v_48) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_50 = asint((asuint(v_49) + asuint(int(vec4_u32.z))));
  int v_51 = asint((asuint(v_50) + asuint(tint_f32_to_i32(mat2x2_f32[0u].x))));
  int v_52 = asint((asuint(v_51) + asuint(tint_f32_to_i32(mat2x3_f32[0u].x))));
  int v_53 = asint((asuint(v_52) + asuint(tint_f32_to_i32(mat2x4_f32[0u].x))));
  int v_54 = asint((asuint(v_53) + asuint(tint_f32_to_i32(mat3x2_f32[0u].x))));
  int v_55 = asint((asuint(v_54) + asuint(tint_f32_to_i32(mat3x3_f32[0u].x))));
  int v_56 = asint((asuint(v_55) + asuint(tint_f32_to_i32(mat3x4_f32[0u].x))));
  int v_57 = asint((asuint(v_56) + asuint(tint_f32_to_i32(mat4x2_f32[0u].x))));
  int v_58 = asint((asuint(v_57) + asuint(tint_f32_to_i32(mat4x3_f32[0u].x))));
  int v_59 = asint((asuint(v_58) + asuint(tint_f32_to_i32(mat4x4_f32[0u].x))));
  s.Store(0u, asuint(asint((asuint(v_59) + asuint(tint_f32_to_i32(arr2_vec3_f32[0u].x))))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

