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
  uint v_8 = (8u + start_byte_offset);
  uint4 v_9 = ub[(v_8 / 16u)];
  uint v_10 = (16u + start_byte_offset);
  uint4 v_11 = ub[(v_10 / 16u)];
  uint v_12 = (24u + start_byte_offset);
  uint4 v_13 = ub[(v_12 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_7.zw, v_7.xy)), asfloat(select((((v_8 & 15u) >> 2u) == 2u), v_9.zw, v_9.xy)), asfloat(select((((v_10 & 15u) >> 2u) == 2u), v_11.zw, v_11.xy)), asfloat(select((((v_12 & 15u) >> 2u) == 2u), v_13.zw, v_13.xy)));
}

float3x4 v_14(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_15(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_16(uint start_byte_offset) {
  uint4 v_17 = ub[(start_byte_offset / 16u)];
  uint v_18 = (8u + start_byte_offset);
  uint4 v_19 = ub[(v_18 / 16u)];
  uint v_20 = (16u + start_byte_offset);
  uint4 v_21 = ub[(v_20 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_17.zw, v_17.xy)), asfloat(select((((v_18 & 15u) >> 2u) == 2u), v_19.zw, v_19.xy)), asfloat(select((((v_20 & 15u) >> 2u) == 2u), v_21.zw, v_21.xy)));
}

float2x4 v_22(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_23(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_24(uint start_byte_offset) {
  uint4 v_25 = ub[(start_byte_offset / 16u)];
  uint v_26 = (8u + start_byte_offset);
  uint4 v_27 = ub[(v_26 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_25.zw, v_25.xy)), asfloat(select((((v_26 & 15u) >> 2u) == 2u), v_27.zw, v_27.xy)));
}

void main_inner(uint idx) {
  uint v_28 = (idx * 544u);
  float scalar_f32 = asfloat(ub[(v_28 / 16u)][((v_28 & 15u) >> 2u)]);
  uint v_29 = (4u + (idx * 544u));
  int scalar_i32 = asint(ub[(v_29 / 16u)][((v_29 & 15u) >> 2u)]);
  uint v_30 = (8u + (idx * 544u));
  uint scalar_u32 = ub[(v_30 / 16u)][((v_30 & 15u) >> 2u)];
  uint v_31 = (16u + (idx * 544u));
  uint4 v_32 = ub[(v_31 / 16u)];
  float2 vec2_f32 = asfloat(select((((v_31 & 15u) >> 2u) == 2u), v_32.zw, v_32.xy));
  uint v_33 = (24u + (idx * 544u));
  uint4 v_34 = ub[(v_33 / 16u)];
  int2 vec2_i32 = asint(select((((v_33 & 15u) >> 2u) == 2u), v_34.zw, v_34.xy));
  uint v_35 = (32u + (idx * 544u));
  uint4 v_36 = ub[(v_35 / 16u)];
  uint2 vec2_u32 = select((((v_35 & 15u) >> 2u) == 2u), v_36.zw, v_36.xy);
  float3 vec3_f32 = asfloat(ub[((48u + (idx * 544u)) / 16u)].xyz);
  int3 vec3_i32 = asint(ub[((64u + (idx * 544u)) / 16u)].xyz);
  uint3 vec3_u32 = ub[((80u + (idx * 544u)) / 16u)].xyz;
  float4 vec4_f32 = asfloat(ub[((96u + (idx * 544u)) / 16u)]);
  int4 vec4_i32 = asint(ub[((112u + (idx * 544u)) / 16u)]);
  uint4 vec4_u32 = ub[((128u + (idx * 544u)) / 16u)];
  float2x2 mat2x2_f32 = v_24((144u + (idx * 544u)));
  float2x3 mat2x3_f32 = v_23((160u + (idx * 544u)));
  float2x4 mat2x4_f32 = v_22((192u + (idx * 544u)));
  float3x2 mat3x2_f32 = v_16((224u + (idx * 544u)));
  float3x3 mat3x3_f32 = v_15((256u + (idx * 544u)));
  float3x4 mat3x4_f32 = v_14((304u + (idx * 544u)));
  float4x2 mat4x2_f32 = v_6((352u + (idx * 544u)));
  float4x3 mat4x3_f32 = v_5((384u + (idx * 544u)));
  float4x4 mat4x4_f32 = v_4((448u + (idx * 544u)));
  float3 arr2_vec3_f32[2] = v((512u + (idx * 544u)));
  int v_37 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_38 = asint((asuint(v_37) + asuint(int(scalar_u32))));
  int v_39 = asint((asuint(asint((asuint(v_38) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_40 = asint((asuint(v_39) + asuint(int(vec2_u32.x))));
  int v_41 = asint((asuint(asint((asuint(v_40) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_42 = asint((asuint(v_41) + asuint(int(vec3_u32.y))));
  int v_43 = asint((asuint(asint((asuint(v_42) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_44 = asint((asuint(v_43) + asuint(int(vec4_u32.z))));
  int v_45 = asint((asuint(v_44) + asuint(tint_f32_to_i32(mat2x2_f32[0u].x))));
  int v_46 = asint((asuint(v_45) + asuint(tint_f32_to_i32(mat2x3_f32[0u].x))));
  int v_47 = asint((asuint(v_46) + asuint(tint_f32_to_i32(mat2x4_f32[0u].x))));
  int v_48 = asint((asuint(v_47) + asuint(tint_f32_to_i32(mat3x2_f32[0u].x))));
  int v_49 = asint((asuint(v_48) + asuint(tint_f32_to_i32(mat3x3_f32[0u].x))));
  int v_50 = asint((asuint(v_49) + asuint(tint_f32_to_i32(mat3x4_f32[0u].x))));
  int v_51 = asint((asuint(v_50) + asuint(tint_f32_to_i32(mat4x2_f32[0u].x))));
  int v_52 = asint((asuint(v_51) + asuint(tint_f32_to_i32(mat4x3_f32[0u].x))));
  int v_53 = asint((asuint(v_52) + asuint(tint_f32_to_i32(mat4x4_f32[0u].x))));
  s.Store(0u, asuint(asint((asuint(v_53) + asuint(tint_f32_to_i32(arr2_vec3_f32[0u].x))))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

