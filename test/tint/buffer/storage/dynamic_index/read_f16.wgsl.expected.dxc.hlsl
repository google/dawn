struct main_inputs {
  uint idx : SV_GroupIndex;
};


ByteAddressBuffer sb : register(t0);
RWByteAddressBuffer s : register(u1);
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

int tint_f16_to_i32(float16_t value) {
  return int(clamp(value, float16_t(-65504.0h), float16_t(65504.0h)));
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
  uint v_26 = 0u;
  sb.GetDimensions(v_26);
  float scalar_f32 = asfloat(sb.Load((0u + (min(idx, ((v_26 / 800u) - 1u)) * 800u))));
  uint v_27 = 0u;
  sb.GetDimensions(v_27);
  int scalar_i32 = asint(sb.Load((4u + (min(idx, ((v_27 / 800u) - 1u)) * 800u))));
  uint v_28 = 0u;
  sb.GetDimensions(v_28);
  uint scalar_u32 = sb.Load((8u + (min(idx, ((v_28 / 800u) - 1u)) * 800u)));
  uint v_29 = 0u;
  sb.GetDimensions(v_29);
  float16_t scalar_f16 = sb.Load<float16_t>((12u + (min(idx, ((v_29 / 800u) - 1u)) * 800u)));
  uint v_30 = 0u;
  sb.GetDimensions(v_30);
  float2 vec2_f32 = asfloat(sb.Load2((16u + (min(idx, ((v_30 / 800u) - 1u)) * 800u))));
  uint v_31 = 0u;
  sb.GetDimensions(v_31);
  int2 vec2_i32 = asint(sb.Load2((24u + (min(idx, ((v_31 / 800u) - 1u)) * 800u))));
  uint v_32 = 0u;
  sb.GetDimensions(v_32);
  uint2 vec2_u32 = sb.Load2((32u + (min(idx, ((v_32 / 800u) - 1u)) * 800u)));
  uint v_33 = 0u;
  sb.GetDimensions(v_33);
  vector<float16_t, 2> vec2_f16 = sb.Load<vector<float16_t, 2> >((40u + (min(idx, ((v_33 / 800u) - 1u)) * 800u)));
  uint v_34 = 0u;
  sb.GetDimensions(v_34);
  float3 vec3_f32 = asfloat(sb.Load3((48u + (min(idx, ((v_34 / 800u) - 1u)) * 800u))));
  uint v_35 = 0u;
  sb.GetDimensions(v_35);
  int3 vec3_i32 = asint(sb.Load3((64u + (min(idx, ((v_35 / 800u) - 1u)) * 800u))));
  uint v_36 = 0u;
  sb.GetDimensions(v_36);
  uint3 vec3_u32 = sb.Load3((80u + (min(idx, ((v_36 / 800u) - 1u)) * 800u)));
  uint v_37 = 0u;
  sb.GetDimensions(v_37);
  vector<float16_t, 3> vec3_f16 = sb.Load<vector<float16_t, 3> >((96u + (min(idx, ((v_37 / 800u) - 1u)) * 800u)));
  uint v_38 = 0u;
  sb.GetDimensions(v_38);
  float4 vec4_f32 = asfloat(sb.Load4((112u + (min(idx, ((v_38 / 800u) - 1u)) * 800u))));
  uint v_39 = 0u;
  sb.GetDimensions(v_39);
  int4 vec4_i32 = asint(sb.Load4((128u + (min(idx, ((v_39 / 800u) - 1u)) * 800u))));
  uint v_40 = 0u;
  sb.GetDimensions(v_40);
  uint4 vec4_u32 = sb.Load4((144u + (min(idx, ((v_40 / 800u) - 1u)) * 800u)));
  uint v_41 = 0u;
  sb.GetDimensions(v_41);
  vector<float16_t, 4> vec4_f16 = sb.Load<vector<float16_t, 4> >((160u + (min(idx, ((v_41 / 800u) - 1u)) * 800u)));
  uint v_42 = 0u;
  sb.GetDimensions(v_42);
  float2x2 mat2x2_f32 = v_25((168u + (min(idx, ((v_42 / 800u) - 1u)) * 800u)));
  uint v_43 = 0u;
  sb.GetDimensions(v_43);
  float2x3 mat2x3_f32 = v_24((192u + (min(idx, ((v_43 / 800u) - 1u)) * 800u)));
  uint v_44 = 0u;
  sb.GetDimensions(v_44);
  float2x4 mat2x4_f32 = v_23((224u + (min(idx, ((v_44 / 800u) - 1u)) * 800u)));
  uint v_45 = 0u;
  sb.GetDimensions(v_45);
  float3x2 mat3x2_f32 = v_22((256u + (min(idx, ((v_45 / 800u) - 1u)) * 800u)));
  uint v_46 = 0u;
  sb.GetDimensions(v_46);
  float3x3 mat3x3_f32 = v_21((288u + (min(idx, ((v_46 / 800u) - 1u)) * 800u)));
  uint v_47 = 0u;
  sb.GetDimensions(v_47);
  float3x4 mat3x4_f32 = v_20((336u + (min(idx, ((v_47 / 800u) - 1u)) * 800u)));
  uint v_48 = 0u;
  sb.GetDimensions(v_48);
  float4x2 mat4x2_f32 = v_19((384u + (min(idx, ((v_48 / 800u) - 1u)) * 800u)));
  uint v_49 = 0u;
  sb.GetDimensions(v_49);
  float4x3 mat4x3_f32 = v_18((416u + (min(idx, ((v_49 / 800u) - 1u)) * 800u)));
  uint v_50 = 0u;
  sb.GetDimensions(v_50);
  float4x4 mat4x4_f32 = v_17((480u + (min(idx, ((v_50 / 800u) - 1u)) * 800u)));
  uint v_51 = 0u;
  sb.GetDimensions(v_51);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_16((544u + (min(idx, ((v_51 / 800u) - 1u)) * 800u)));
  uint v_52 = 0u;
  sb.GetDimensions(v_52);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_15((552u + (min(idx, ((v_52 / 800u) - 1u)) * 800u)));
  uint v_53 = 0u;
  sb.GetDimensions(v_53);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_14((568u + (min(idx, ((v_53 / 800u) - 1u)) * 800u)));
  uint v_54 = 0u;
  sb.GetDimensions(v_54);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_13((584u + (min(idx, ((v_54 / 800u) - 1u)) * 800u)));
  uint v_55 = 0u;
  sb.GetDimensions(v_55);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_12((600u + (min(idx, ((v_55 / 800u) - 1u)) * 800u)));
  uint v_56 = 0u;
  sb.GetDimensions(v_56);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_11((624u + (min(idx, ((v_56 / 800u) - 1u)) * 800u)));
  uint v_57 = 0u;
  sb.GetDimensions(v_57);
  matrix<float16_t, 4, 2> mat4x2_f16 = v((648u + (min(idx, ((v_57 / 800u) - 1u)) * 800u)));
  uint v_58 = 0u;
  sb.GetDimensions(v_58);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_10((664u + (min(idx, ((v_58 / 800u) - 1u)) * 800u)));
  uint v_59 = 0u;
  sb.GetDimensions(v_59);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_9((696u + (min(idx, ((v_59 / 800u) - 1u)) * 800u)));
  uint v_60 = 0u;
  sb.GetDimensions(v_60);
  float3 arr2_vec3_f32[2] = v_5((736u + (min(idx, ((v_60 / 800u) - 1u)) * 800u)));
  uint v_61 = 0u;
  sb.GetDimensions(v_61);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_1((768u + (min(idx, ((v_61 / 800u) - 1u)) * 800u)));
  int v_62 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_63 = asint((asuint(v_62) + asuint(int(scalar_u32))));
  int v_64 = asint((asuint(v_63) + asuint(tint_f16_to_i32(scalar_f16))));
  int v_65 = asint((asuint(asint((asuint(v_64) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_66 = asint((asuint(v_65) + asuint(int(vec2_u32.x))));
  int v_67 = asint((asuint(v_66) + asuint(tint_f16_to_i32(vec2_f16.x))));
  int v_68 = asint((asuint(asint((asuint(v_67) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_69 = asint((asuint(v_68) + asuint(int(vec3_u32.y))));
  int v_70 = asint((asuint(v_69) + asuint(tint_f16_to_i32(vec3_f16.y))));
  int v_71 = asint((asuint(asint((asuint(v_70) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_72 = asint((asuint(v_71) + asuint(int(vec4_u32.z))));
  int v_73 = asint((asuint(v_72) + asuint(tint_f16_to_i32(vec4_f16.z))));
  int v_74 = asint((asuint(v_73) + asuint(tint_f32_to_i32(mat2x2_f32[0u].x))));
  int v_75 = asint((asuint(v_74) + asuint(tint_f32_to_i32(mat2x3_f32[0u].x))));
  int v_76 = asint((asuint(v_75) + asuint(tint_f32_to_i32(mat2x4_f32[0u].x))));
  int v_77 = asint((asuint(v_76) + asuint(tint_f32_to_i32(mat3x2_f32[0u].x))));
  int v_78 = asint((asuint(v_77) + asuint(tint_f32_to_i32(mat3x3_f32[0u].x))));
  int v_79 = asint((asuint(v_78) + asuint(tint_f32_to_i32(mat3x4_f32[0u].x))));
  int v_80 = asint((asuint(v_79) + asuint(tint_f32_to_i32(mat4x2_f32[0u].x))));
  int v_81 = asint((asuint(v_80) + asuint(tint_f32_to_i32(mat4x3_f32[0u].x))));
  int v_82 = asint((asuint(v_81) + asuint(tint_f32_to_i32(mat4x4_f32[0u].x))));
  int v_83 = asint((asuint(v_82) + asuint(tint_f16_to_i32(mat2x2_f16[0u].x))));
  int v_84 = asint((asuint(v_83) + asuint(tint_f16_to_i32(mat2x3_f16[0u].x))));
  int v_85 = asint((asuint(v_84) + asuint(tint_f16_to_i32(mat2x4_f16[0u].x))));
  int v_86 = asint((asuint(v_85) + asuint(tint_f16_to_i32(mat3x2_f16[0u].x))));
  int v_87 = asint((asuint(v_86) + asuint(tint_f16_to_i32(mat3x3_f16[0u].x))));
  int v_88 = asint((asuint(v_87) + asuint(tint_f16_to_i32(mat3x4_f16[0u].x))));
  int v_89 = asint((asuint(v_88) + asuint(tint_f16_to_i32(mat4x2_f16[0u].x))));
  int v_90 = asint((asuint(v_89) + asuint(tint_f16_to_i32(mat4x3_f16[0u].x))));
  int v_91 = asint((asuint(v_90) + asuint(tint_f16_to_i32(mat4x4_f16[0u].x))));
  int v_92 = asint((asuint(v_91) + asuint(tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x))));
  s.Store(0u, asuint(asint((asuint(v_92) + asuint(tint_f32_to_i32(arr2_vec3_f32[0u].x))))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

