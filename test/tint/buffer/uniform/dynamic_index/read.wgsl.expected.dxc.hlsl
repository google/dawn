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
  uint v_7 = (8u + start_byte_offset);
  uint4 v_8 = ub[(v_7 / 16u)];
  uint v_9 = (16u + start_byte_offset);
  uint4 v_10 = ub[(v_9 / 16u)];
  uint v_11 = (24u + start_byte_offset);
  uint4 v_12 = ub[(v_11 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_6.zw, v_6.xy)), asfloat(select((((v_7 & 15u) >> 2u) == 2u), v_8.zw, v_8.xy)), asfloat(select((((v_9 & 15u) >> 2u) == 2u), v_10.zw, v_10.xy)), asfloat(select((((v_11 & 15u) >> 2u) == 2u), v_12.zw, v_12.xy)));
}

float3x4 v_13(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_14(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_15(uint start_byte_offset) {
  uint4 v_16 = ub[(start_byte_offset / 16u)];
  uint v_17 = (8u + start_byte_offset);
  uint4 v_18 = ub[(v_17 / 16u)];
  uint v_19 = (16u + start_byte_offset);
  uint4 v_20 = ub[(v_19 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_16.zw, v_16.xy)), asfloat(select((((v_17 & 15u) >> 2u) == 2u), v_18.zw, v_18.xy)), asfloat(select((((v_19 & 15u) >> 2u) == 2u), v_20.zw, v_20.xy)));
}

float2x4 v_21(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_22(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_23(uint start_byte_offset) {
  uint4 v_24 = ub[(start_byte_offset / 16u)];
  uint v_25 = (8u + start_byte_offset);
  uint4 v_26 = ub[(v_25 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_24.zw, v_24.xy)), asfloat(select((((v_25 & 15u) >> 2u) == 2u), v_26.zw, v_26.xy)));
}

void main_inner(uint idx) {
  uint v_27 = (idx * 544u);
  float scalar_f32 = asfloat(ub[(v_27 / 16u)][((v_27 & 15u) >> 2u)]);
  uint v_28 = (4u + (idx * 544u));
  int scalar_i32 = asint(ub[(v_28 / 16u)][((v_28 & 15u) >> 2u)]);
  uint v_29 = (8u + (idx * 544u));
  uint scalar_u32 = ub[(v_29 / 16u)][((v_29 & 15u) >> 2u)];
  uint v_30 = (16u + (idx * 544u));
  uint4 v_31 = ub[(v_30 / 16u)];
  float2 vec2_f32 = asfloat(select((((v_30 & 15u) >> 2u) == 2u), v_31.zw, v_31.xy));
  uint v_32 = (24u + (idx * 544u));
  uint4 v_33 = ub[(v_32 / 16u)];
  int2 vec2_i32 = asint(select((((v_32 & 15u) >> 2u) == 2u), v_33.zw, v_33.xy));
  uint v_34 = (32u + (idx * 544u));
  uint4 v_35 = ub[(v_34 / 16u)];
  uint2 vec2_u32 = select((((v_34 & 15u) >> 2u) == 2u), v_35.zw, v_35.xy);
  float3 vec3_f32 = asfloat(ub[((48u + (idx * 544u)) / 16u)].xyz);
  int3 vec3_i32 = asint(ub[((64u + (idx * 544u)) / 16u)].xyz);
  uint3 vec3_u32 = ub[((80u + (idx * 544u)) / 16u)].xyz;
  float4 vec4_f32 = asfloat(ub[((96u + (idx * 544u)) / 16u)]);
  int4 vec4_i32 = asint(ub[((112u + (idx * 544u)) / 16u)]);
  uint4 vec4_u32 = ub[((128u + (idx * 544u)) / 16u)];
  float2x2 mat2x2_f32 = v_23((144u + (idx * 544u)));
  float2x3 mat2x3_f32 = v_22((160u + (idx * 544u)));
  float2x4 mat2x4_f32 = v_21((192u + (idx * 544u)));
  float3x2 mat3x2_f32 = v_15((224u + (idx * 544u)));
  float3x3 mat3x3_f32 = v_14((256u + (idx * 544u)));
  float3x4 mat3x4_f32 = v_13((304u + (idx * 544u)));
  float4x2 mat4x2_f32 = v_5((352u + (idx * 544u)));
  float4x3 mat4x3_f32 = v_4((384u + (idx * 544u)));
  float4x4 mat4x4_f32 = v_3((448u + (idx * 544u)));
  float3 arr2_vec3_f32[2] = v((512u + (idx * 544u)));
  int v_36 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_37 = asint((asuint(v_36) + asuint(int(scalar_u32))));
  int v_38 = asint((asuint(asint((asuint(v_37) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_39 = asint((asuint(v_38) + asuint(int(vec2_u32.x))));
  int v_40 = asint((asuint(asint((asuint(v_39) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_41 = asint((asuint(v_40) + asuint(int(vec3_u32.y))));
  int v_42 = asint((asuint(asint((asuint(v_41) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_43 = asint((asuint(v_42) + asuint(int(vec4_u32.z))));
  int v_44 = asint((asuint(v_43) + asuint(tint_f32_to_i32(mat2x2_f32[int(0)].x))));
  int v_45 = asint((asuint(v_44) + asuint(tint_f32_to_i32(mat2x3_f32[int(0)].x))));
  int v_46 = asint((asuint(v_45) + asuint(tint_f32_to_i32(mat2x4_f32[int(0)].x))));
  int v_47 = asint((asuint(v_46) + asuint(tint_f32_to_i32(mat3x2_f32[int(0)].x))));
  int v_48 = asint((asuint(v_47) + asuint(tint_f32_to_i32(mat3x3_f32[int(0)].x))));
  int v_49 = asint((asuint(v_48) + asuint(tint_f32_to_i32(mat3x4_f32[int(0)].x))));
  int v_50 = asint((asuint(v_49) + asuint(tint_f32_to_i32(mat4x2_f32[int(0)].x))));
  int v_51 = asint((asuint(v_50) + asuint(tint_f32_to_i32(mat4x3_f32[int(0)].x))));
  int v_52 = asint((asuint(v_51) + asuint(tint_f32_to_i32(mat4x4_f32[int(0)].x))));
  s.Store(0u, asuint(asint((asuint(v_52) + asuint(tint_f32_to_i32(arr2_vec3_f32[int(0)].x))))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

