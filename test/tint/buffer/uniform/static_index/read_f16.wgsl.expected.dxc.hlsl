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
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

Inner v_2(uint start_byte_offset) {
  int v_3 = asint(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  float v_5 = asfloat(ub[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  Inner v_7 = {v_3, v_5, tint_bitcast_to_f16(ub[(v_6 / 16u)][((v_6 & 15u) >> 2u)])[select(((v_6 % 4u) == 0u), 0u, 1u)]};
  return v_7;
}

typedef Inner ary_ret[4];
ary_ret v_8(uint start_byte_offset) {
  Inner a[4] = (Inner[4])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      Inner v_11 = v_2((start_byte_offset + (v_10 * 16u)));
      a[v_10] = v_11;
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  Inner v_12[4] = a;
  return v_12;
}

matrix<float16_t, 4, 2> v_13(uint start_byte_offset) {
  vector<float16_t, 2> v_14 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_15 = (4u + start_byte_offset);
  vector<float16_t, 2> v_16 = tint_bitcast_to_f16(ub[(v_15 / 16u)][((v_15 & 15u) >> 2u)]);
  uint v_17 = (8u + start_byte_offset);
  vector<float16_t, 2> v_18 = tint_bitcast_to_f16(ub[(v_17 / 16u)][((v_17 & 15u) >> 2u)]);
  uint v_19 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_14, v_16, v_18, tint_bitcast_to_f16(ub[(v_19 / 16u)][((v_19 & 15u) >> 2u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret_1[2];
ary_ret_1 v_20(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_21 = 0u;
    v_21 = 0u;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 2u)) {
        break;
      }
      a[v_22] = v_13((start_byte_offset + (v_22 * 16u)));
      {
        v_21 = (v_22 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_23[2] = a;
  return v_23;
}

typedef float3 ary_ret_2[2];
ary_ret_2 v_24(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_25 = 0u;
    v_25 = 0u;
    while(true) {
      uint v_26 = v_25;
      if ((v_26 >= 2u)) {
        break;
      }
      a[v_26] = asfloat(ub[((start_byte_offset + (v_26 * 16u)) / 16u)].xyz);
      {
        v_25 = (v_26 + 1u);
      }
    }
  }
  float3 v_27[2] = a;
  return v_27;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 4> v_28(uint start_byte_offset) {
  uint4 v_29 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_30 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_29.zw, v_29.xy));
  uint v_31 = (8u + start_byte_offset);
  uint4 v_32 = ub[(v_31 / 16u)];
  vector<float16_t, 4> v_33 = tint_bitcast_to_f16_1(select((((v_31 & 15u) >> 2u) == 2u), v_32.zw, v_32.xy));
  uint v_34 = (16u + start_byte_offset);
  uint4 v_35 = ub[(v_34 / 16u)];
  vector<float16_t, 4> v_36 = tint_bitcast_to_f16_1(select((((v_34 & 15u) >> 2u) == 2u), v_35.zw, v_35.xy));
  uint v_37 = (24u + start_byte_offset);
  uint4 v_38 = ub[(v_37 / 16u)];
  return matrix<float16_t, 4, 4>(v_30, v_33, v_36, tint_bitcast_to_f16_1(select((((v_37 & 15u) >> 2u) == 2u), v_38.zw, v_38.xy)));
}

matrix<float16_t, 4, 3> v_39(uint start_byte_offset) {
  uint4 v_40 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_41 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_40.zw, v_40.xy)).xyz;
  uint v_42 = (8u + start_byte_offset);
  uint4 v_43 = ub[(v_42 / 16u)];
  vector<float16_t, 3> v_44 = tint_bitcast_to_f16_1(select((((v_42 & 15u) >> 2u) == 2u), v_43.zw, v_43.xy)).xyz;
  uint v_45 = (16u + start_byte_offset);
  uint4 v_46 = ub[(v_45 / 16u)];
  vector<float16_t, 3> v_47 = tint_bitcast_to_f16_1(select((((v_45 & 15u) >> 2u) == 2u), v_46.zw, v_46.xy)).xyz;
  uint v_48 = (24u + start_byte_offset);
  uint4 v_49 = ub[(v_48 / 16u)];
  return matrix<float16_t, 4, 3>(v_41, v_44, v_47, tint_bitcast_to_f16_1(select((((v_48 & 15u) >> 2u) == 2u), v_49.zw, v_49.xy)).xyz);
}

matrix<float16_t, 3, 4> v_50(uint start_byte_offset) {
  uint4 v_51 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_52 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_51.zw, v_51.xy));
  uint v_53 = (8u + start_byte_offset);
  uint4 v_54 = ub[(v_53 / 16u)];
  vector<float16_t, 4> v_55 = tint_bitcast_to_f16_1(select((((v_53 & 15u) >> 2u) == 2u), v_54.zw, v_54.xy));
  uint v_56 = (16u + start_byte_offset);
  uint4 v_57 = ub[(v_56 / 16u)];
  return matrix<float16_t, 3, 4>(v_52, v_55, tint_bitcast_to_f16_1(select((((v_56 & 15u) >> 2u) == 2u), v_57.zw, v_57.xy)));
}

matrix<float16_t, 3, 3> v_58(uint start_byte_offset) {
  uint4 v_59 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_60 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_59.zw, v_59.xy)).xyz;
  uint v_61 = (8u + start_byte_offset);
  uint4 v_62 = ub[(v_61 / 16u)];
  vector<float16_t, 3> v_63 = tint_bitcast_to_f16_1(select((((v_61 & 15u) >> 2u) == 2u), v_62.zw, v_62.xy)).xyz;
  uint v_64 = (16u + start_byte_offset);
  uint4 v_65 = ub[(v_64 / 16u)];
  return matrix<float16_t, 3, 3>(v_60, v_63, tint_bitcast_to_f16_1(select((((v_64 & 15u) >> 2u) == 2u), v_65.zw, v_65.xy)).xyz);
}

matrix<float16_t, 3, 2> v_66(uint start_byte_offset) {
  vector<float16_t, 2> v_67 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_68 = (4u + start_byte_offset);
  vector<float16_t, 2> v_69 = tint_bitcast_to_f16(ub[(v_68 / 16u)][((v_68 & 15u) >> 2u)]);
  uint v_70 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_67, v_69, tint_bitcast_to_f16(ub[(v_70 / 16u)][((v_70 & 15u) >> 2u)]));
}

matrix<float16_t, 2, 4> v_71(uint start_byte_offset) {
  uint4 v_72 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_73 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_72.zw, v_72.xy));
  uint v_74 = (8u + start_byte_offset);
  uint4 v_75 = ub[(v_74 / 16u)];
  return matrix<float16_t, 2, 4>(v_73, tint_bitcast_to_f16_1(select((((v_74 & 15u) >> 2u) == 2u), v_75.zw, v_75.xy)));
}

matrix<float16_t, 2, 3> v_76(uint start_byte_offset) {
  uint4 v_77 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_78 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_77.zw, v_77.xy)).xyz;
  uint v_79 = (8u + start_byte_offset);
  uint4 v_80 = ub[(v_79 / 16u)];
  return matrix<float16_t, 2, 3>(v_78, tint_bitcast_to_f16_1(select((((v_79 & 15u) >> 2u) == 2u), v_80.zw, v_80.xy)).xyz);
}

matrix<float16_t, 2, 2> v_81(uint start_byte_offset) {
  vector<float16_t, 2> v_82 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_83 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_82, tint_bitcast_to_f16(ub[(v_83 / 16u)][((v_83 & 15u) >> 2u)]));
}

float4x4 v_84(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_85(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_86(uint start_byte_offset) {
  uint4 v_87 = ub[(start_byte_offset / 16u)];
  uint v_88 = (8u + start_byte_offset);
  uint4 v_89 = ub[(v_88 / 16u)];
  uint v_90 = (16u + start_byte_offset);
  uint4 v_91 = ub[(v_90 / 16u)];
  uint v_92 = (24u + start_byte_offset);
  uint4 v_93 = ub[(v_92 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_87.zw, v_87.xy)), asfloat(select((((v_88 & 15u) >> 2u) == 2u), v_89.zw, v_89.xy)), asfloat(select((((v_90 & 15u) >> 2u) == 2u), v_91.zw, v_91.xy)), asfloat(select((((v_92 & 15u) >> 2u) == 2u), v_93.zw, v_93.xy)));
}

float3x4 v_94(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_95(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_96(uint start_byte_offset) {
  uint4 v_97 = ub[(start_byte_offset / 16u)];
  uint v_98 = (8u + start_byte_offset);
  uint4 v_99 = ub[(v_98 / 16u)];
  uint v_100 = (16u + start_byte_offset);
  uint4 v_101 = ub[(v_100 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_97.zw, v_97.xy)), asfloat(select((((v_98 & 15u) >> 2u) == 2u), v_99.zw, v_99.xy)), asfloat(select((((v_100 & 15u) >> 2u) == 2u), v_101.zw, v_101.xy)));
}

float2x4 v_102(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_103(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_104(uint start_byte_offset) {
  uint4 v_105 = ub[(start_byte_offset / 16u)];
  uint v_106 = (8u + start_byte_offset);
  uint4 v_107 = ub[(v_106 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_105.zw, v_105.xy)), asfloat(select((((v_106 & 15u) >> 2u) == 2u), v_107.zw, v_107.xy)));
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
  float2x2 mat2x2_f32 = v_104(168u);
  float2x3 mat2x3_f32 = v_103(192u);
  float2x4 mat2x4_f32 = v_102(224u);
  float3x2 mat3x2_f32 = v_96(256u);
  float3x3 mat3x3_f32 = v_95(288u);
  float3x4 mat3x4_f32 = v_94(336u);
  float4x2 mat4x2_f32 = v_86(384u);
  float4x3 mat4x3_f32 = v_85(416u);
  float4x4 mat4x4_f32 = v_84(480u);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_81(544u);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_76(552u);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_71(568u);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_66(584u);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_58(600u);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_50(624u);
  matrix<float16_t, 4, 2> mat4x2_f16 = v_13(648u);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_39(664u);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_28(696u);
  float3 arr2_vec3_f32[2] = v_24(736u);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_20(768u);
  Inner struct_inner = v_2(800u);
  Inner array_struct_inner[4] = v_8(816u);
  int v_108 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_109 = asint((asuint(v_108) + asuint(int(scalar_u32))));
  int v_110 = asint((asuint(v_109) + asuint(tint_f16_to_i32(scalar_f16))));
  int v_111 = asint((asuint(asint((asuint(v_110) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_112 = asint((asuint(v_111) + asuint(int(vec2_u32.x))));
  int v_113 = asint((asuint(v_112) + asuint(tint_f16_to_i32(vec2_f16.x))));
  int v_114 = asint((asuint(asint((asuint(v_113) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_115 = asint((asuint(v_114) + asuint(int(vec3_u32.y))));
  int v_116 = asint((asuint(v_115) + asuint(tint_f16_to_i32(vec3_f16.y))));
  int v_117 = asint((asuint(asint((asuint(v_116) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_118 = asint((asuint(v_117) + asuint(int(vec4_u32.z))));
  int v_119 = asint((asuint(v_118) + asuint(tint_f16_to_i32(vec4_f16.z))));
  int v_120 = asint((asuint(v_119) + asuint(tint_f32_to_i32(mat2x2_f32[0u].x))));
  int v_121 = asint((asuint(v_120) + asuint(tint_f32_to_i32(mat2x3_f32[0u].x))));
  int v_122 = asint((asuint(v_121) + asuint(tint_f32_to_i32(mat2x4_f32[0u].x))));
  int v_123 = asint((asuint(v_122) + asuint(tint_f32_to_i32(mat3x2_f32[0u].x))));
  int v_124 = asint((asuint(v_123) + asuint(tint_f32_to_i32(mat3x3_f32[0u].x))));
  int v_125 = asint((asuint(v_124) + asuint(tint_f32_to_i32(mat3x4_f32[0u].x))));
  int v_126 = asint((asuint(v_125) + asuint(tint_f32_to_i32(mat4x2_f32[0u].x))));
  int v_127 = asint((asuint(v_126) + asuint(tint_f32_to_i32(mat4x3_f32[0u].x))));
  int v_128 = asint((asuint(v_127) + asuint(tint_f32_to_i32(mat4x4_f32[0u].x))));
  int v_129 = asint((asuint(v_128) + asuint(tint_f16_to_i32(mat2x2_f16[0u].x))));
  int v_130 = asint((asuint(v_129) + asuint(tint_f16_to_i32(mat2x3_f16[0u].x))));
  int v_131 = asint((asuint(v_130) + asuint(tint_f16_to_i32(mat2x4_f16[0u].x))));
  int v_132 = asint((asuint(v_131) + asuint(tint_f16_to_i32(mat3x2_f16[0u].x))));
  int v_133 = asint((asuint(v_132) + asuint(tint_f16_to_i32(mat3x3_f16[0u].x))));
  int v_134 = asint((asuint(v_133) + asuint(tint_f16_to_i32(mat3x4_f16[0u].x))));
  int v_135 = asint((asuint(v_134) + asuint(tint_f16_to_i32(mat4x2_f16[0u].x))));
  int v_136 = asint((asuint(v_135) + asuint(tint_f16_to_i32(mat4x3_f16[0u].x))));
  int v_137 = asint((asuint(v_136) + asuint(tint_f16_to_i32(mat4x4_f16[0u].x))));
  int v_138 = asint((asuint(v_137) + asuint(tint_f32_to_i32(arr2_vec3_f32[0u].x))));
  s.Store(0u, asuint(asint((asuint(asint((asuint(asint((asuint(v_138) + asuint(tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x))))) + asuint(struct_inner.scalar_i32)))) + asuint(array_struct_inner[0u].scalar_i32)))));
}

