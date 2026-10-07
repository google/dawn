struct main_inputs {
  uint idx : SV_GroupIndex;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[400];
};
RWByteAddressBuffer s : register(u1);
int tint_f16_to_i32(float16_t value) {
  return int(clamp(value, float16_t(-65504.0h), float16_t(65504.0h)));
}

int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((uint2(v, v) >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 2> v_1(uint start_byte_offset) {
  vector<float16_t, 2> v_2 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(ub[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16(ub[(v_5 / 16u)][((v_5 & 15u) >> 2u)]);
  uint v_7 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_2, v_4, v_6, tint_bitcast_to_f16(ub[(v_7 / 16u)][((v_7 & 15u) >> 2u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret[2];
ary_ret v_8(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_9 = idx;
      if ((v_9 >= 2u)) {
        break;
      }
      a[v_9] = v_1((start_byte_offset + (v_9 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_10[2] = a;
  return v_10;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_11(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 2u)) {
        break;
      }
      a[v_12] = asfloat(ub[((start_byte_offset + (v_12 * 16u)) / 16u)].xyz);
      {
        idx = (idx + 1u);
      }
    }
  }
  float3 v_13[2] = a;
  return v_13;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 4> v_14(uint start_byte_offset) {
  uint4 v_15 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_16 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_15.zw, v_15.xy));
  uint v_17 = (8u + start_byte_offset);
  uint4 v_18 = ub[(v_17 / 16u)];
  vector<float16_t, 4> v_19 = tint_bitcast_to_f16_1(select((((v_17 & 15u) >> 2u) == 2u), v_18.zw, v_18.xy));
  uint v_20 = (16u + start_byte_offset);
  uint4 v_21 = ub[(v_20 / 16u)];
  vector<float16_t, 4> v_22 = tint_bitcast_to_f16_1(select((((v_20 & 15u) >> 2u) == 2u), v_21.zw, v_21.xy));
  uint v_23 = (24u + start_byte_offset);
  uint4 v_24 = ub[(v_23 / 16u)];
  return matrix<float16_t, 4, 4>(v_16, v_19, v_22, tint_bitcast_to_f16_1(select((((v_23 & 15u) >> 2u) == 2u), v_24.zw, v_24.xy)));
}

matrix<float16_t, 4, 3> v_25(uint start_byte_offset) {
  uint4 v_26 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_27 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_26.zw, v_26.xy)).xyz;
  uint v_28 = (8u + start_byte_offset);
  uint4 v_29 = ub[(v_28 / 16u)];
  vector<float16_t, 3> v_30 = tint_bitcast_to_f16_1(select((((v_28 & 15u) >> 2u) == 2u), v_29.zw, v_29.xy)).xyz;
  uint v_31 = (16u + start_byte_offset);
  uint4 v_32 = ub[(v_31 / 16u)];
  vector<float16_t, 3> v_33 = tint_bitcast_to_f16_1(select((((v_31 & 15u) >> 2u) == 2u), v_32.zw, v_32.xy)).xyz;
  uint v_34 = (24u + start_byte_offset);
  uint4 v_35 = ub[(v_34 / 16u)];
  return matrix<float16_t, 4, 3>(v_27, v_30, v_33, tint_bitcast_to_f16_1(select((((v_34 & 15u) >> 2u) == 2u), v_35.zw, v_35.xy)).xyz);
}

matrix<float16_t, 3, 4> v_36(uint start_byte_offset) {
  uint4 v_37 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_38 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_37.zw, v_37.xy));
  uint v_39 = (8u + start_byte_offset);
  uint4 v_40 = ub[(v_39 / 16u)];
  vector<float16_t, 4> v_41 = tint_bitcast_to_f16_1(select((((v_39 & 15u) >> 2u) == 2u), v_40.zw, v_40.xy));
  uint v_42 = (16u + start_byte_offset);
  uint4 v_43 = ub[(v_42 / 16u)];
  return matrix<float16_t, 3, 4>(v_38, v_41, tint_bitcast_to_f16_1(select((((v_42 & 15u) >> 2u) == 2u), v_43.zw, v_43.xy)));
}

matrix<float16_t, 3, 3> v_44(uint start_byte_offset) {
  uint4 v_45 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_46 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_45.zw, v_45.xy)).xyz;
  uint v_47 = (8u + start_byte_offset);
  uint4 v_48 = ub[(v_47 / 16u)];
  vector<float16_t, 3> v_49 = tint_bitcast_to_f16_1(select((((v_47 & 15u) >> 2u) == 2u), v_48.zw, v_48.xy)).xyz;
  uint v_50 = (16u + start_byte_offset);
  uint4 v_51 = ub[(v_50 / 16u)];
  return matrix<float16_t, 3, 3>(v_46, v_49, tint_bitcast_to_f16_1(select((((v_50 & 15u) >> 2u) == 2u), v_51.zw, v_51.xy)).xyz);
}

matrix<float16_t, 3, 2> v_52(uint start_byte_offset) {
  vector<float16_t, 2> v_53 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_54 = (4u + start_byte_offset);
  vector<float16_t, 2> v_55 = tint_bitcast_to_f16(ub[(v_54 / 16u)][((v_54 & 15u) >> 2u)]);
  uint v_56 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_53, v_55, tint_bitcast_to_f16(ub[(v_56 / 16u)][((v_56 & 15u) >> 2u)]));
}

matrix<float16_t, 2, 4> v_57(uint start_byte_offset) {
  uint4 v_58 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_59 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_58.zw, v_58.xy));
  uint v_60 = (8u + start_byte_offset);
  uint4 v_61 = ub[(v_60 / 16u)];
  return matrix<float16_t, 2, 4>(v_59, tint_bitcast_to_f16_1(select((((v_60 & 15u) >> 2u) == 2u), v_61.zw, v_61.xy)));
}

matrix<float16_t, 2, 3> v_62(uint start_byte_offset) {
  uint4 v_63 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_64 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_63.zw, v_63.xy)).xyz;
  uint v_65 = (8u + start_byte_offset);
  uint4 v_66 = ub[(v_65 / 16u)];
  return matrix<float16_t, 2, 3>(v_64, tint_bitcast_to_f16_1(select((((v_65 & 15u) >> 2u) == 2u), v_66.zw, v_66.xy)).xyz);
}

matrix<float16_t, 2, 2> v_67(uint start_byte_offset) {
  vector<float16_t, 2> v_68 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_69 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_68, tint_bitcast_to_f16(ub[(v_69 / 16u)][((v_69 & 15u) >> 2u)]));
}

float4x4 v_70(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_71(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_72(uint start_byte_offset) {
  uint4 v_73 = ub[(start_byte_offset / 16u)];
  uint v_74 = (8u + start_byte_offset);
  uint4 v_75 = ub[(v_74 / 16u)];
  uint v_76 = (16u + start_byte_offset);
  uint4 v_77 = ub[(v_76 / 16u)];
  uint v_78 = (24u + start_byte_offset);
  uint4 v_79 = ub[(v_78 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_73.zw, v_73.xy)), asfloat(select((((v_74 & 15u) >> 2u) == 2u), v_75.zw, v_75.xy)), asfloat(select((((v_76 & 15u) >> 2u) == 2u), v_77.zw, v_77.xy)), asfloat(select((((v_78 & 15u) >> 2u) == 2u), v_79.zw, v_79.xy)));
}

float3x4 v_80(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_81(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_82(uint start_byte_offset) {
  uint4 v_83 = ub[(start_byte_offset / 16u)];
  uint v_84 = (8u + start_byte_offset);
  uint4 v_85 = ub[(v_84 / 16u)];
  uint v_86 = (16u + start_byte_offset);
  uint4 v_87 = ub[(v_86 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_83.zw, v_83.xy)), asfloat(select((((v_84 & 15u) >> 2u) == 2u), v_85.zw, v_85.xy)), asfloat(select((((v_86 & 15u) >> 2u) == 2u), v_87.zw, v_87.xy)));
}

float2x4 v_88(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_89(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_90(uint start_byte_offset) {
  uint4 v_91 = ub[(start_byte_offset / 16u)];
  uint v_92 = (8u + start_byte_offset);
  uint4 v_93 = ub[(v_92 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_91.zw, v_91.xy)), asfloat(select((((v_92 & 15u) >> 2u) == 2u), v_93.zw, v_93.xy)));
}

void main_inner(uint idx) {
  uint v_94 = (idx * 800u);
  float scalar_f32 = asfloat(ub[(v_94 / 16u)][((v_94 & 15u) >> 2u)]);
  uint v_95 = (4u + (idx * 800u));
  int scalar_i32 = asint(ub[(v_95 / 16u)][((v_95 & 15u) >> 2u)]);
  uint v_96 = (8u + (idx * 800u));
  uint scalar_u32 = ub[(v_96 / 16u)][((v_96 & 15u) >> 2u)];
  uint v_97 = (12u + (idx * 800u));
  float16_t scalar_f16 = tint_bitcast_to_f16(ub[(v_97 / 16u)][((v_97 & 15u) >> 2u)])[select(((v_97 % 4u) == 0u), 0u, 1u)];
  uint v_98 = (16u + (idx * 800u));
  uint4 v_99 = ub[(v_98 / 16u)];
  float2 vec2_f32 = asfloat(select((((v_98 & 15u) >> 2u) == 2u), v_99.zw, v_99.xy));
  uint v_100 = (24u + (idx * 800u));
  uint4 v_101 = ub[(v_100 / 16u)];
  int2 vec2_i32 = asint(select((((v_100 & 15u) >> 2u) == 2u), v_101.zw, v_101.xy));
  uint v_102 = (32u + (idx * 800u));
  uint4 v_103 = ub[(v_102 / 16u)];
  uint2 vec2_u32 = select((((v_102 & 15u) >> 2u) == 2u), v_103.zw, v_103.xy);
  uint v_104 = (40u + (idx * 800u));
  vector<float16_t, 2> vec2_f16 = tint_bitcast_to_f16(ub[(v_104 / 16u)][((v_104 & 15u) >> 2u)]);
  float3 vec3_f32 = asfloat(ub[((48u + (idx * 800u)) / 16u)].xyz);
  int3 vec3_i32 = asint(ub[((64u + (idx * 800u)) / 16u)].xyz);
  uint3 vec3_u32 = ub[((80u + (idx * 800u)) / 16u)].xyz;
  uint v_105 = (96u + (idx * 800u));
  uint4 v_106 = ub[(v_105 / 16u)];
  vector<float16_t, 3> vec3_f16 = tint_bitcast_to_f16_1(select((((v_105 & 15u) >> 2u) == 2u), v_106.zw, v_106.xy)).xyz;
  float4 vec4_f32 = asfloat(ub[((112u + (idx * 800u)) / 16u)]);
  int4 vec4_i32 = asint(ub[((128u + (idx * 800u)) / 16u)]);
  uint4 vec4_u32 = ub[((144u + (idx * 800u)) / 16u)];
  uint v_107 = (160u + (idx * 800u));
  uint4 v_108 = ub[(v_107 / 16u)];
  vector<float16_t, 4> vec4_f16 = tint_bitcast_to_f16_1(select((((v_107 & 15u) >> 2u) == 2u), v_108.zw, v_108.xy));
  float2x2 mat2x2_f32 = v_90((168u + (idx * 800u)));
  float2x3 mat2x3_f32 = v_89((192u + (idx * 800u)));
  float2x4 mat2x4_f32 = v_88((224u + (idx * 800u)));
  float3x2 mat3x2_f32 = v_82((256u + (idx * 800u)));
  float3x3 mat3x3_f32 = v_81((288u + (idx * 800u)));
  float3x4 mat3x4_f32 = v_80((336u + (idx * 800u)));
  float4x2 mat4x2_f32 = v_72((384u + (idx * 800u)));
  float4x3 mat4x3_f32 = v_71((416u + (idx * 800u)));
  float4x4 mat4x4_f32 = v_70((480u + (idx * 800u)));
  matrix<float16_t, 2, 2> mat2x2_f16 = v_67((544u + (idx * 800u)));
  matrix<float16_t, 2, 3> mat2x3_f16 = v_62((552u + (idx * 800u)));
  matrix<float16_t, 2, 4> mat2x4_f16 = v_57((568u + (idx * 800u)));
  matrix<float16_t, 3, 2> mat3x2_f16 = v_52((584u + (idx * 800u)));
  matrix<float16_t, 3, 3> mat3x3_f16 = v_44((600u + (idx * 800u)));
  matrix<float16_t, 3, 4> mat3x4_f16 = v_36((624u + (idx * 800u)));
  matrix<float16_t, 4, 2> mat4x2_f16 = v_1((648u + (idx * 800u)));
  matrix<float16_t, 4, 3> mat4x3_f16 = v_25((664u + (idx * 800u)));
  matrix<float16_t, 4, 4> mat4x4_f16 = v_14((696u + (idx * 800u)));
  float3 arr2_vec3_f32[2] = v_11((736u + (idx * 800u)));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_8((768u + (idx * 800u)));
  int v_109 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_110 = asint((asuint(v_109) + asuint(int(scalar_u32))));
  int v_111 = asint((asuint(v_110) + asuint(tint_f16_to_i32(scalar_f16))));
  int v_112 = asint((asuint(asint((asuint(v_111) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_113 = asint((asuint(v_112) + asuint(int(vec2_u32.x))));
  int v_114 = asint((asuint(v_113) + asuint(tint_f16_to_i32(vec2_f16.x))));
  int v_115 = asint((asuint(asint((asuint(v_114) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_116 = asint((asuint(v_115) + asuint(int(vec3_u32.y))));
  int v_117 = asint((asuint(v_116) + asuint(tint_f16_to_i32(vec3_f16.y))));
  int v_118 = asint((asuint(asint((asuint(v_117) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_119 = asint((asuint(v_118) + asuint(int(vec4_u32.z))));
  int v_120 = asint((asuint(v_119) + asuint(tint_f16_to_i32(vec4_f16.z))));
  int v_121 = asint((asuint(v_120) + asuint(tint_f32_to_i32(mat2x2_f32[int(0)].x))));
  int v_122 = asint((asuint(v_121) + asuint(tint_f32_to_i32(mat2x3_f32[int(0)].x))));
  int v_123 = asint((asuint(v_122) + asuint(tint_f32_to_i32(mat2x4_f32[int(0)].x))));
  int v_124 = asint((asuint(v_123) + asuint(tint_f32_to_i32(mat3x2_f32[int(0)].x))));
  int v_125 = asint((asuint(v_124) + asuint(tint_f32_to_i32(mat3x3_f32[int(0)].x))));
  int v_126 = asint((asuint(v_125) + asuint(tint_f32_to_i32(mat3x4_f32[int(0)].x))));
  int v_127 = asint((asuint(v_126) + asuint(tint_f32_to_i32(mat4x2_f32[int(0)].x))));
  int v_128 = asint((asuint(v_127) + asuint(tint_f32_to_i32(mat4x3_f32[int(0)].x))));
  int v_129 = asint((asuint(v_128) + asuint(tint_f32_to_i32(mat4x4_f32[int(0)].x))));
  int v_130 = asint((asuint(v_129) + asuint(tint_f16_to_i32(mat2x2_f16[int(0)].x))));
  int v_131 = asint((asuint(v_130) + asuint(tint_f16_to_i32(mat2x3_f16[int(0)].x))));
  int v_132 = asint((asuint(v_131) + asuint(tint_f16_to_i32(mat2x4_f16[int(0)].x))));
  int v_133 = asint((asuint(v_132) + asuint(tint_f16_to_i32(mat3x2_f16[int(0)].x))));
  int v_134 = asint((asuint(v_133) + asuint(tint_f16_to_i32(mat3x3_f16[int(0)].x))));
  int v_135 = asint((asuint(v_134) + asuint(tint_f16_to_i32(mat3x4_f16[int(0)].x))));
  int v_136 = asint((asuint(v_135) + asuint(tint_f16_to_i32(mat4x2_f16[int(0)].x))));
  int v_137 = asint((asuint(v_136) + asuint(tint_f16_to_i32(mat4x3_f16[int(0)].x))));
  int v_138 = asint((asuint(v_137) + asuint(tint_f16_to_i32(mat4x4_f16[int(0)].x))));
  int v_139 = asint((asuint(v_138) + asuint(tint_f32_to_i32(arr2_vec3_f32[int(0)].x))));
  s.Store(0u, asuint(asint((asuint(v_139) + asuint(tint_f16_to_i32(arr2_mat4x2_f16[int(0)][int(0)].x))))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

