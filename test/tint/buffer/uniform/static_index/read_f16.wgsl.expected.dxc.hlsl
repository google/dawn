struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[55];
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

Inner v_1(uint start_byte_offset) {
  int v_2 = asint(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_3 = (4u + start_byte_offset);
  float v_4 = asfloat(ub[(v_3 / 16u)][((v_3 & 15u) >> 2u)]);
  uint v_5 = (8u + start_byte_offset);
  Inner v_6 = {v_2, v_4, tint_bitcast_to_f16(ub[(v_5 / 16u)][((v_5 & 15u) >> 2u)])[select(((v_5 % 4u) == 0u), 0u, 1u)]};
  return v_6;
}

typedef Inner ary_ret[4];
ary_ret v_7(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_8 = idx;
      if ((v_8 >= 4u)) {
        break;
      }
      Inner v_9 = v_1((start_byte_offset + (v_8 * 16u)));
      a[v_8] = v_9;
      {
        idx = (idx + 1u);
      }
    }
  }
  Inner v_10[4] = a;
  return v_10;
}

matrix<float16_t, 4, 2> v_11(uint start_byte_offset) {
  vector<float16_t, 2> v_12 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_13 = (4u + start_byte_offset);
  vector<float16_t, 2> v_14 = tint_bitcast_to_f16(ub[(v_13 / 16u)][((v_13 & 15u) >> 2u)]);
  uint v_15 = (8u + start_byte_offset);
  vector<float16_t, 2> v_16 = tint_bitcast_to_f16(ub[(v_15 / 16u)][((v_15 & 15u) >> 2u)]);
  uint v_17 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_12, v_14, v_16, tint_bitcast_to_f16(ub[(v_17 / 16u)][((v_17 & 15u) >> 2u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret_1[2];
ary_ret_1 v_18(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_19 = idx;
      if ((v_19 >= 2u)) {
        break;
      }
      a[v_19] = v_11((start_byte_offset + (v_19 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_20[2] = a;
  return v_20;
}

typedef float3 ary_ret_2[2];
ary_ret_2 v_21(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_22 = idx;
      if ((v_22 >= 2u)) {
        break;
      }
      a[v_22] = asfloat(ub[((start_byte_offset + (v_22 * 16u)) / 16u)].xyz);
      {
        idx = (idx + 1u);
      }
    }
  }
  float3 v_23[2] = a;
  return v_23;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 4> v_24(uint start_byte_offset) {
  uint4 v_25 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_26 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_25.zw, v_25.xy));
  uint v_27 = (8u + start_byte_offset);
  uint4 v_28 = ub[(v_27 / 16u)];
  vector<float16_t, 4> v_29 = tint_bitcast_to_f16_1(select((((v_27 & 15u) >> 2u) == 2u), v_28.zw, v_28.xy));
  uint v_30 = (16u + start_byte_offset);
  uint4 v_31 = ub[(v_30 / 16u)];
  vector<float16_t, 4> v_32 = tint_bitcast_to_f16_1(select((((v_30 & 15u) >> 2u) == 2u), v_31.zw, v_31.xy));
  uint v_33 = (24u + start_byte_offset);
  uint4 v_34 = ub[(v_33 / 16u)];
  return matrix<float16_t, 4, 4>(v_26, v_29, v_32, tint_bitcast_to_f16_1(select((((v_33 & 15u) >> 2u) == 2u), v_34.zw, v_34.xy)));
}

matrix<float16_t, 4, 3> v_35(uint start_byte_offset) {
  uint4 v_36 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_37 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_36.zw, v_36.xy)).xyz;
  uint v_38 = (8u + start_byte_offset);
  uint4 v_39 = ub[(v_38 / 16u)];
  vector<float16_t, 3> v_40 = tint_bitcast_to_f16_1(select((((v_38 & 15u) >> 2u) == 2u), v_39.zw, v_39.xy)).xyz;
  uint v_41 = (16u + start_byte_offset);
  uint4 v_42 = ub[(v_41 / 16u)];
  vector<float16_t, 3> v_43 = tint_bitcast_to_f16_1(select((((v_41 & 15u) >> 2u) == 2u), v_42.zw, v_42.xy)).xyz;
  uint v_44 = (24u + start_byte_offset);
  uint4 v_45 = ub[(v_44 / 16u)];
  return matrix<float16_t, 4, 3>(v_37, v_40, v_43, tint_bitcast_to_f16_1(select((((v_44 & 15u) >> 2u) == 2u), v_45.zw, v_45.xy)).xyz);
}

matrix<float16_t, 3, 4> v_46(uint start_byte_offset) {
  uint4 v_47 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_48 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_47.zw, v_47.xy));
  uint v_49 = (8u + start_byte_offset);
  uint4 v_50 = ub[(v_49 / 16u)];
  vector<float16_t, 4> v_51 = tint_bitcast_to_f16_1(select((((v_49 & 15u) >> 2u) == 2u), v_50.zw, v_50.xy));
  uint v_52 = (16u + start_byte_offset);
  uint4 v_53 = ub[(v_52 / 16u)];
  return matrix<float16_t, 3, 4>(v_48, v_51, tint_bitcast_to_f16_1(select((((v_52 & 15u) >> 2u) == 2u), v_53.zw, v_53.xy)));
}

matrix<float16_t, 3, 3> v_54(uint start_byte_offset) {
  uint4 v_55 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_56 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_55.zw, v_55.xy)).xyz;
  uint v_57 = (8u + start_byte_offset);
  uint4 v_58 = ub[(v_57 / 16u)];
  vector<float16_t, 3> v_59 = tint_bitcast_to_f16_1(select((((v_57 & 15u) >> 2u) == 2u), v_58.zw, v_58.xy)).xyz;
  uint v_60 = (16u + start_byte_offset);
  uint4 v_61 = ub[(v_60 / 16u)];
  return matrix<float16_t, 3, 3>(v_56, v_59, tint_bitcast_to_f16_1(select((((v_60 & 15u) >> 2u) == 2u), v_61.zw, v_61.xy)).xyz);
}

matrix<float16_t, 3, 2> v_62(uint start_byte_offset) {
  vector<float16_t, 2> v_63 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_64 = (4u + start_byte_offset);
  vector<float16_t, 2> v_65 = tint_bitcast_to_f16(ub[(v_64 / 16u)][((v_64 & 15u) >> 2u)]);
  uint v_66 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_63, v_65, tint_bitcast_to_f16(ub[(v_66 / 16u)][((v_66 & 15u) >> 2u)]));
}

matrix<float16_t, 2, 4> v_67(uint start_byte_offset) {
  uint4 v_68 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_69 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_68.zw, v_68.xy));
  uint v_70 = (8u + start_byte_offset);
  uint4 v_71 = ub[(v_70 / 16u)];
  return matrix<float16_t, 2, 4>(v_69, tint_bitcast_to_f16_1(select((((v_70 & 15u) >> 2u) == 2u), v_71.zw, v_71.xy)));
}

matrix<float16_t, 2, 3> v_72(uint start_byte_offset) {
  uint4 v_73 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_74 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_73.zw, v_73.xy)).xyz;
  uint v_75 = (8u + start_byte_offset);
  uint4 v_76 = ub[(v_75 / 16u)];
  return matrix<float16_t, 2, 3>(v_74, tint_bitcast_to_f16_1(select((((v_75 & 15u) >> 2u) == 2u), v_76.zw, v_76.xy)).xyz);
}

matrix<float16_t, 2, 2> v_77(uint start_byte_offset) {
  vector<float16_t, 2> v_78 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_79 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_78, tint_bitcast_to_f16(ub[(v_79 / 16u)][((v_79 & 15u) >> 2u)]));
}

float4x4 v_80(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_81(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_82(uint start_byte_offset) {
  uint4 v_83 = ub[(start_byte_offset / 16u)];
  uint v_84 = (8u + start_byte_offset);
  uint4 v_85 = ub[(v_84 / 16u)];
  uint v_86 = (16u + start_byte_offset);
  uint4 v_87 = ub[(v_86 / 16u)];
  uint v_88 = (24u + start_byte_offset);
  uint4 v_89 = ub[(v_88 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_83.zw, v_83.xy)), asfloat(select((((v_84 & 15u) >> 2u) == 2u), v_85.zw, v_85.xy)), asfloat(select((((v_86 & 15u) >> 2u) == 2u), v_87.zw, v_87.xy)), asfloat(select((((v_88 & 15u) >> 2u) == 2u), v_89.zw, v_89.xy)));
}

float3x4 v_90(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_91(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_92(uint start_byte_offset) {
  uint4 v_93 = ub[(start_byte_offset / 16u)];
  uint v_94 = (8u + start_byte_offset);
  uint4 v_95 = ub[(v_94 / 16u)];
  uint v_96 = (16u + start_byte_offset);
  uint4 v_97 = ub[(v_96 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_93.zw, v_93.xy)), asfloat(select((((v_94 & 15u) >> 2u) == 2u), v_95.zw, v_95.xy)), asfloat(select((((v_96 & 15u) >> 2u) == 2u), v_97.zw, v_97.xy)));
}

float2x4 v_98(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_99(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_100(uint start_byte_offset) {
  uint4 v_101 = ub[(start_byte_offset / 16u)];
  uint v_102 = (8u + start_byte_offset);
  uint4 v_103 = ub[(v_102 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_101.zw, v_101.xy)), asfloat(select((((v_102 & 15u) >> 2u) == 2u), v_103.zw, v_103.xy)));
}

[numthreads(1, 1, 1)]
void main() {
  float scalar_f32 = asfloat(ub[0u].x);
  int scalar_i32 = asint(ub[0u].y);
  uint scalar_u32 = ub[0u].z;
  float16_t scalar_f16 = tint_bitcast_to_f16(ub[0u].w).x;
  float2 vec2_f32 = asfloat(ub[1u].xy);
  int2 vec2_i32 = asint(ub[1u].zw);
  uint2 vec2_u32 = ub[2u].xy;
  vector<float16_t, 2> vec2_f16 = tint_bitcast_to_f16(ub[2u].z);
  float3 vec3_f32 = asfloat(ub[3u].xyz);
  int3 vec3_i32 = asint(ub[4u].xyz);
  uint3 vec3_u32 = ub[5u].xyz;
  vector<float16_t, 3> vec3_f16 = tint_bitcast_to_f16_1(ub[6u].xy).xyz;
  float4 vec4_f32 = asfloat(ub[7u]);
  int4 vec4_i32 = asint(ub[8u]);
  uint4 vec4_u32 = ub[9u];
  vector<float16_t, 4> vec4_f16 = tint_bitcast_to_f16_1(ub[10u].xy);
  float2x2 mat2x2_f32 = v_100(168u);
  float2x3 mat2x3_f32 = v_99(192u);
  float2x4 mat2x4_f32 = v_98(224u);
  float3x2 mat3x2_f32 = v_92(256u);
  float3x3 mat3x3_f32 = v_91(288u);
  float3x4 mat3x4_f32 = v_90(336u);
  float4x2 mat4x2_f32 = v_82(384u);
  float4x3 mat4x3_f32 = v_81(416u);
  float4x4 mat4x4_f32 = v_80(480u);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_77(544u);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_72(552u);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_67(568u);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_62(584u);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_54(600u);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_46(624u);
  matrix<float16_t, 4, 2> mat4x2_f16 = v_11(648u);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_35(664u);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_24(696u);
  float3 arr2_vec3_f32[2] = v_21(736u);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_18(768u);
  Inner struct_inner = v_1(800u);
  Inner array_struct_inner[4] = v_7(816u);
  int v_104 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_105 = asint((asuint(v_104) + asuint(int(scalar_u32))));
  int v_106 = asint((asuint(v_105) + asuint(tint_f16_to_i32(scalar_f16))));
  int v_107 = asint((asuint(asint((asuint(v_106) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_108 = asint((asuint(v_107) + asuint(int(vec2_u32.x))));
  int v_109 = asint((asuint(v_108) + asuint(tint_f16_to_i32(vec2_f16.x))));
  int v_110 = asint((asuint(asint((asuint(v_109) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_111 = asint((asuint(v_110) + asuint(int(vec3_u32.y))));
  int v_112 = asint((asuint(v_111) + asuint(tint_f16_to_i32(vec3_f16.y))));
  int v_113 = asint((asuint(asint((asuint(v_112) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_114 = asint((asuint(v_113) + asuint(int(vec4_u32.z))));
  int v_115 = asint((asuint(v_114) + asuint(tint_f16_to_i32(vec4_f16.z))));
  int v_116 = asint((asuint(v_115) + asuint(tint_f32_to_i32(mat2x2_f32[int(0)].x))));
  int v_117 = asint((asuint(v_116) + asuint(tint_f32_to_i32(mat2x3_f32[int(0)].x))));
  int v_118 = asint((asuint(v_117) + asuint(tint_f32_to_i32(mat2x4_f32[int(0)].x))));
  int v_119 = asint((asuint(v_118) + asuint(tint_f32_to_i32(mat3x2_f32[int(0)].x))));
  int v_120 = asint((asuint(v_119) + asuint(tint_f32_to_i32(mat3x3_f32[int(0)].x))));
  int v_121 = asint((asuint(v_120) + asuint(tint_f32_to_i32(mat3x4_f32[int(0)].x))));
  int v_122 = asint((asuint(v_121) + asuint(tint_f32_to_i32(mat4x2_f32[int(0)].x))));
  int v_123 = asint((asuint(v_122) + asuint(tint_f32_to_i32(mat4x3_f32[int(0)].x))));
  int v_124 = asint((asuint(v_123) + asuint(tint_f32_to_i32(mat4x4_f32[int(0)].x))));
  int v_125 = asint((asuint(v_124) + asuint(tint_f16_to_i32(mat2x2_f16[int(0)].x))));
  int v_126 = asint((asuint(v_125) + asuint(tint_f16_to_i32(mat2x3_f16[int(0)].x))));
  int v_127 = asint((asuint(v_126) + asuint(tint_f16_to_i32(mat2x4_f16[int(0)].x))));
  int v_128 = asint((asuint(v_127) + asuint(tint_f16_to_i32(mat3x2_f16[int(0)].x))));
  int v_129 = asint((asuint(v_128) + asuint(tint_f16_to_i32(mat3x3_f16[int(0)].x))));
  int v_130 = asint((asuint(v_129) + asuint(tint_f16_to_i32(mat3x4_f16[int(0)].x))));
  int v_131 = asint((asuint(v_130) + asuint(tint_f16_to_i32(mat4x2_f16[int(0)].x))));
  int v_132 = asint((asuint(v_131) + asuint(tint_f16_to_i32(mat4x3_f16[int(0)].x))));
  int v_133 = asint((asuint(v_132) + asuint(tint_f16_to_i32(mat4x4_f16[int(0)].x))));
  int v_134 = asint((asuint(v_133) + asuint(tint_f32_to_i32(arr2_vec3_f32[int(0)].x))));
  s.Store(0u, asuint(asint((asuint(asint((asuint(asint((asuint(v_134) + asuint(tint_f16_to_i32(arr2_mat4x2_f16[int(0)][int(0)].x))))) + asuint(struct_inner.scalar_i32)))) + asuint(array_struct_inner[int(0)].scalar_i32)))));
}

