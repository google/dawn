struct main_inputs {
  uint idx : SV_GroupIndex;
};


ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

typedef float3 ary_ret[2];
ary_ret v(uint offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 2u)) {
        break;
      }
      a[v_2] = asfloat(sb.Load3((offset + (v_2 * 16u))));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
  float3 v_3[2] = a;
  return v_3;
}

float4x4 v_4(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

float4x3 v_5(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x2 v_6(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float3x4 v_7(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float3x3 v_8(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x2 v_9(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float2x4 v_10(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float2x3 v_11(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x2 v_12(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
}

void main_inner(uint idx) {
  float scalar_f32 = asfloat(sb.Load((0u + (uint(idx) * 544u))));
  int scalar_i32 = asint(sb.Load((4u + (uint(idx) * 544u))));
  uint scalar_u32 = sb.Load((8u + (uint(idx) * 544u)));
  float2 vec2_f32 = asfloat(sb.Load2((16u + (uint(idx) * 544u))));
  int2 vec2_i32 = asint(sb.Load2((24u + (uint(idx) * 544u))));
  uint2 vec2_u32 = sb.Load2((32u + (uint(idx) * 544u)));
  float3 vec3_f32 = asfloat(sb.Load3((48u + (uint(idx) * 544u))));
  int3 vec3_i32 = asint(sb.Load3((64u + (uint(idx) * 544u))));
  uint3 vec3_u32 = sb.Load3((80u + (uint(idx) * 544u)));
  float4 vec4_f32 = asfloat(sb.Load4((96u + (uint(idx) * 544u))));
  int4 vec4_i32 = asint(sb.Load4((112u + (uint(idx) * 544u))));
  uint4 vec4_u32 = sb.Load4((128u + (uint(idx) * 544u)));
  float2x2 mat2x2_f32 = v_12((144u + (uint(idx) * 544u)));
  float2x3 mat2x3_f32 = v_11((160u + (uint(idx) * 544u)));
  float2x4 mat2x4_f32 = v_10((192u + (uint(idx) * 544u)));
  float3x2 mat3x2_f32 = v_9((224u + (uint(idx) * 544u)));
  float3x3 mat3x3_f32 = v_8((256u + (uint(idx) * 544u)));
  float3x4 mat3x4_f32 = v_7((304u + (uint(idx) * 544u)));
  float4x2 mat4x2_f32 = v_6((352u + (uint(idx) * 544u)));
  float4x3 mat4x3_f32 = v_5((384u + (uint(idx) * 544u)));
  float4x4 mat4x4_f32 = v_4((448u + (uint(idx) * 544u)));
  float3 arr2_vec3_f32[2] = v((512u + (uint(idx) * 544u)));
  int v_13 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_14 = (v_13 + int(scalar_u32));
  int v_15 = ((v_14 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_16 = (v_15 + int(vec2_u32.x));
  int v_17 = ((v_16 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_18 = (v_17 + int(vec3_u32.y));
  int v_19 = ((v_18 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_20 = (v_19 + int(vec4_u32.z));
  int v_21 = (v_20 + tint_f32_to_i32(mat2x2_f32[int(0)].x));
  int v_22 = (v_21 + tint_f32_to_i32(mat2x3_f32[int(0)].x));
  int v_23 = (v_22 + tint_f32_to_i32(mat2x4_f32[int(0)].x));
  int v_24 = (v_23 + tint_f32_to_i32(mat3x2_f32[int(0)].x));
  int v_25 = (v_24 + tint_f32_to_i32(mat3x3_f32[int(0)].x));
  int v_26 = (v_25 + tint_f32_to_i32(mat3x4_f32[int(0)].x));
  int v_27 = (v_26 + tint_f32_to_i32(mat4x2_f32[int(0)].x));
  int v_28 = (v_27 + tint_f32_to_i32(mat4x3_f32[int(0)].x));
  int v_29 = (v_28 + tint_f32_to_i32(mat4x4_f32[int(0)].x));
  s.Store(0u, asuint((v_29 + tint_f32_to_i32(arr2_vec3_f32[int(0)].x))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

