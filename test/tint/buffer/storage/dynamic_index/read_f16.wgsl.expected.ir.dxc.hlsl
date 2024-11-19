struct main_inputs {
  uint idx : SV_GroupIndex;
};


ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

int tint_f16_to_i32(float16_t value) {
  return (((value <= float16_t(65504.0h))) ? ((((value >= float16_t(-65504.0h))) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

matrix<float16_t, 4, 2> v(uint offset) {
  return matrix<float16_t, 4, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)), sb.Load<vector<float16_t, 2> >((offset + 12u)));
}

typedef matrix<float16_t, 4, 2> ary_ret[2];
ary_ret v_1(uint offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 2u)) {
        break;
      }
      a[v_3] = v((offset + (v_3 * 16u)));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_4[2] = a;
  return v_4;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_5(uint offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 2u)) {
        break;
      }
      a[v_7] = asfloat(sb.Load3((offset + (v_7 * 16u))));
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
  float3 v_8[2] = a;
  return v_8;
}

matrix<float16_t, 4, 4> v_9(uint offset) {
  return matrix<float16_t, 4, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)), sb.Load<vector<float16_t, 4> >((offset + 24u)));
}

matrix<float16_t, 4, 3> v_10(uint offset) {
  return matrix<float16_t, 4, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)), sb.Load<vector<float16_t, 3> >((offset + 24u)));
}

matrix<float16_t, 3, 4> v_11(uint offset) {
  return matrix<float16_t, 3, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)), sb.Load<vector<float16_t, 4> >((offset + 16u)));
}

matrix<float16_t, 3, 3> v_12(uint offset) {
  return matrix<float16_t, 3, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)), sb.Load<vector<float16_t, 3> >((offset + 16u)));
}

matrix<float16_t, 3, 2> v_13(uint offset) {
  return matrix<float16_t, 3, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)), sb.Load<vector<float16_t, 2> >((offset + 8u)));
}

matrix<float16_t, 2, 4> v_14(uint offset) {
  return matrix<float16_t, 2, 4>(sb.Load<vector<float16_t, 4> >((offset + 0u)), sb.Load<vector<float16_t, 4> >((offset + 8u)));
}

matrix<float16_t, 2, 3> v_15(uint offset) {
  return matrix<float16_t, 2, 3>(sb.Load<vector<float16_t, 3> >((offset + 0u)), sb.Load<vector<float16_t, 3> >((offset + 8u)));
}

matrix<float16_t, 2, 2> v_16(uint offset) {
  return matrix<float16_t, 2, 2>(sb.Load<vector<float16_t, 2> >((offset + 0u)), sb.Load<vector<float16_t, 2> >((offset + 4u)));
}

float4x4 v_17(uint offset) {
  return float4x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))), asfloat(sb.Load4((offset + 48u))));
}

float4x3 v_18(uint offset) {
  return float4x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))), asfloat(sb.Load3((offset + 48u))));
}

float4x2 v_19(uint offset) {
  return float4x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))), asfloat(sb.Load2((offset + 24u))));
}

float3x4 v_20(uint offset) {
  return float3x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))), asfloat(sb.Load4((offset + 32u))));
}

float3x3 v_21(uint offset) {
  return float3x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))), asfloat(sb.Load3((offset + 32u))));
}

float3x2 v_22(uint offset) {
  return float3x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))), asfloat(sb.Load2((offset + 16u))));
}

float2x4 v_23(uint offset) {
  return float2x4(asfloat(sb.Load4((offset + 0u))), asfloat(sb.Load4((offset + 16u))));
}

float2x3 v_24(uint offset) {
  return float2x3(asfloat(sb.Load3((offset + 0u))), asfloat(sb.Load3((offset + 16u))));
}

float2x2 v_25(uint offset) {
  return float2x2(asfloat(sb.Load2((offset + 0u))), asfloat(sb.Load2((offset + 8u))));
}

void main_inner(uint idx) {
  float scalar_f32 = asfloat(sb.Load((0u + (uint(idx) * 800u))));
  int scalar_i32 = asint(sb.Load((4u + (uint(idx) * 800u))));
  uint scalar_u32 = sb.Load((8u + (uint(idx) * 800u)));
  float16_t scalar_f16 = sb.Load<float16_t>((12u + (uint(idx) * 800u)));
  float2 vec2_f32 = asfloat(sb.Load2((16u + (uint(idx) * 800u))));
  int2 vec2_i32 = asint(sb.Load2((24u + (uint(idx) * 800u))));
  uint2 vec2_u32 = sb.Load2((32u + (uint(idx) * 800u)));
  vector<float16_t, 2> vec2_f16 = sb.Load<vector<float16_t, 2> >((40u + (uint(idx) * 800u)));
  float3 vec3_f32 = asfloat(sb.Load3((48u + (uint(idx) * 800u))));
  int3 vec3_i32 = asint(sb.Load3((64u + (uint(idx) * 800u))));
  uint3 vec3_u32 = sb.Load3((80u + (uint(idx) * 800u)));
  vector<float16_t, 3> vec3_f16 = sb.Load<vector<float16_t, 3> >((96u + (uint(idx) * 800u)));
  float4 vec4_f32 = asfloat(sb.Load4((112u + (uint(idx) * 800u))));
  int4 vec4_i32 = asint(sb.Load4((128u + (uint(idx) * 800u))));
  uint4 vec4_u32 = sb.Load4((144u + (uint(idx) * 800u)));
  vector<float16_t, 4> vec4_f16 = sb.Load<vector<float16_t, 4> >((160u + (uint(idx) * 800u)));
  float2x2 mat2x2_f32 = v_25((168u + (uint(idx) * 800u)));
  float2x3 mat2x3_f32 = v_24((192u + (uint(idx) * 800u)));
  float2x4 mat2x4_f32 = v_23((224u + (uint(idx) * 800u)));
  float3x2 mat3x2_f32 = v_22((256u + (uint(idx) * 800u)));
  float3x3 mat3x3_f32 = v_21((288u + (uint(idx) * 800u)));
  float3x4 mat3x4_f32 = v_20((336u + (uint(idx) * 800u)));
  float4x2 mat4x2_f32 = v_19((384u + (uint(idx) * 800u)));
  float4x3 mat4x3_f32 = v_18((416u + (uint(idx) * 800u)));
  float4x4 mat4x4_f32 = v_17((480u + (uint(idx) * 800u)));
  matrix<float16_t, 2, 2> mat2x2_f16 = v_16((544u + (uint(idx) * 800u)));
  matrix<float16_t, 2, 3> mat2x3_f16 = v_15((552u + (uint(idx) * 800u)));
  matrix<float16_t, 2, 4> mat2x4_f16 = v_14((568u + (uint(idx) * 800u)));
  matrix<float16_t, 3, 2> mat3x2_f16 = v_13((584u + (uint(idx) * 800u)));
  matrix<float16_t, 3, 3> mat3x3_f16 = v_12((600u + (uint(idx) * 800u)));
  matrix<float16_t, 3, 4> mat3x4_f16 = v_11((624u + (uint(idx) * 800u)));
  matrix<float16_t, 4, 2> mat4x2_f16 = v((648u + (uint(idx) * 800u)));
  matrix<float16_t, 4, 3> mat4x3_f16 = v_10((664u + (uint(idx) * 800u)));
  matrix<float16_t, 4, 4> mat4x4_f16 = v_9((696u + (uint(idx) * 800u)));
  float3 arr2_vec3_f32[2] = v_5((736u + (uint(idx) * 800u)));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_1((768u + (uint(idx) * 800u)));
  int v_26 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_27 = (v_26 + int(scalar_u32));
  int v_28 = (v_27 + tint_f16_to_i32(scalar_f16));
  int v_29 = ((v_28 + tint_f32_to_i32(vec2_f32.x)) + vec2_i32.x);
  int v_30 = (v_29 + int(vec2_u32.x));
  int v_31 = (v_30 + tint_f16_to_i32(vec2_f16.x));
  int v_32 = ((v_31 + tint_f32_to_i32(vec3_f32.y)) + vec3_i32.y);
  int v_33 = (v_32 + int(vec3_u32.y));
  int v_34 = (v_33 + tint_f16_to_i32(vec3_f16.y));
  int v_35 = ((v_34 + tint_f32_to_i32(vec4_f32.z)) + vec4_i32.z);
  int v_36 = (v_35 + int(vec4_u32.z));
  int v_37 = (v_36 + tint_f16_to_i32(vec4_f16.z));
  int v_38 = (v_37 + tint_f32_to_i32(mat2x2_f32[int(0)].x));
  int v_39 = (v_38 + tint_f32_to_i32(mat2x3_f32[int(0)].x));
  int v_40 = (v_39 + tint_f32_to_i32(mat2x4_f32[int(0)].x));
  int v_41 = (v_40 + tint_f32_to_i32(mat3x2_f32[int(0)].x));
  int v_42 = (v_41 + tint_f32_to_i32(mat3x3_f32[int(0)].x));
  int v_43 = (v_42 + tint_f32_to_i32(mat3x4_f32[int(0)].x));
  int v_44 = (v_43 + tint_f32_to_i32(mat4x2_f32[int(0)].x));
  int v_45 = (v_44 + tint_f32_to_i32(mat4x3_f32[int(0)].x));
  int v_46 = (v_45 + tint_f32_to_i32(mat4x4_f32[int(0)].x));
  int v_47 = (v_46 + tint_f16_to_i32(mat2x2_f16[int(0)].x));
  int v_48 = (v_47 + tint_f16_to_i32(mat2x3_f16[int(0)].x));
  int v_49 = (v_48 + tint_f16_to_i32(mat2x4_f16[int(0)].x));
  int v_50 = (v_49 + tint_f16_to_i32(mat3x2_f16[int(0)].x));
  int v_51 = (v_50 + tint_f16_to_i32(mat3x3_f16[int(0)].x));
  int v_52 = (v_51 + tint_f16_to_i32(mat3x4_f16[int(0)].x));
  int v_53 = (v_52 + tint_f16_to_i32(mat4x2_f16[int(0)].x));
  int v_54 = (v_53 + tint_f16_to_i32(mat4x3_f16[int(0)].x));
  int v_55 = (v_54 + tint_f16_to_i32(mat4x4_f16[int(0)].x));
  int v_56 = (v_55 + tint_f16_to_i32(arr2_mat4x2_f16[int(0)][int(0)].x));
  s.Store(0u, asuint((v_56 + tint_f32_to_i32(arr2_vec3_f32[int(0)].x))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

