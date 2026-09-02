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
    uint v_8 = 0u;
    v_8 = 0u;
    while(true) {
      uint v_9 = v_8;
      if ((v_9 >= 4u)) {
        break;
      }
      Inner v_10 = v_1((start_byte_offset + (v_9 * 16u)));
      a[v_9] = v_10;
      {
        v_8 = (v_9 + 1u);
      }
    }
  }
  Inner v_11[4] = a;
  return v_11;
}

matrix<float16_t, 4, 2> v_12(uint start_byte_offset) {
  vector<float16_t, 2> v_13 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_14 = (4u + start_byte_offset);
  vector<float16_t, 2> v_15 = tint_bitcast_to_f16(ub[(v_14 / 16u)][((v_14 & 15u) >> 2u)]);
  uint v_16 = (8u + start_byte_offset);
  vector<float16_t, 2> v_17 = tint_bitcast_to_f16(ub[(v_16 / 16u)][((v_16 & 15u) >> 2u)]);
  uint v_18 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_13, v_15, v_17, tint_bitcast_to_f16(ub[(v_18 / 16u)][((v_18 & 15u) >> 2u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret_1[2];
ary_ret_1 v_19(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_20 = 0u;
    v_20 = 0u;
    while(true) {
      uint v_21 = v_20;
      if ((v_21 >= 2u)) {
        break;
      }
      a[v_21] = v_12((start_byte_offset + (v_21 * 16u)));
      {
        v_20 = (v_21 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_22[2] = a;
  return v_22;
}

typedef float3 ary_ret_2[2];
ary_ret_2 v_23(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_24 = 0u;
    v_24 = 0u;
    while(true) {
      uint v_25 = v_24;
      if ((v_25 >= 2u)) {
        break;
      }
      a[v_25] = asfloat(ub[((start_byte_offset + (v_25 * 16u)) / 16u)].xyz);
      {
        v_24 = (v_25 + 1u);
      }
    }
  }
  float3 v_26[2] = a;
  return v_26;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 4> v_27(uint start_byte_offset) {
  uint4 v_28 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_29 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_28.zw, v_28.xy));
  uint v_30 = (8u + start_byte_offset);
  uint4 v_31 = ub[(v_30 / 16u)];
  vector<float16_t, 4> v_32 = tint_bitcast_to_f16_1(select((((v_30 & 15u) >> 2u) == 2u), v_31.zw, v_31.xy));
  uint v_33 = (16u + start_byte_offset);
  uint4 v_34 = ub[(v_33 / 16u)];
  vector<float16_t, 4> v_35 = tint_bitcast_to_f16_1(select((((v_33 & 15u) >> 2u) == 2u), v_34.zw, v_34.xy));
  uint v_36 = (24u + start_byte_offset);
  uint4 v_37 = ub[(v_36 / 16u)];
  return matrix<float16_t, 4, 4>(v_29, v_32, v_35, tint_bitcast_to_f16_1(select((((v_36 & 15u) >> 2u) == 2u), v_37.zw, v_37.xy)));
}

matrix<float16_t, 4, 3> v_38(uint start_byte_offset) {
  uint4 v_39 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_40 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_39.zw, v_39.xy)).xyz;
  uint v_41 = (8u + start_byte_offset);
  uint4 v_42 = ub[(v_41 / 16u)];
  vector<float16_t, 3> v_43 = tint_bitcast_to_f16_1(select((((v_41 & 15u) >> 2u) == 2u), v_42.zw, v_42.xy)).xyz;
  uint v_44 = (16u + start_byte_offset);
  uint4 v_45 = ub[(v_44 / 16u)];
  vector<float16_t, 3> v_46 = tint_bitcast_to_f16_1(select((((v_44 & 15u) >> 2u) == 2u), v_45.zw, v_45.xy)).xyz;
  uint v_47 = (24u + start_byte_offset);
  uint4 v_48 = ub[(v_47 / 16u)];
  return matrix<float16_t, 4, 3>(v_40, v_43, v_46, tint_bitcast_to_f16_1(select((((v_47 & 15u) >> 2u) == 2u), v_48.zw, v_48.xy)).xyz);
}

matrix<float16_t, 3, 4> v_49(uint start_byte_offset) {
  uint4 v_50 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_51 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_50.zw, v_50.xy));
  uint v_52 = (8u + start_byte_offset);
  uint4 v_53 = ub[(v_52 / 16u)];
  vector<float16_t, 4> v_54 = tint_bitcast_to_f16_1(select((((v_52 & 15u) >> 2u) == 2u), v_53.zw, v_53.xy));
  uint v_55 = (16u + start_byte_offset);
  uint4 v_56 = ub[(v_55 / 16u)];
  return matrix<float16_t, 3, 4>(v_51, v_54, tint_bitcast_to_f16_1(select((((v_55 & 15u) >> 2u) == 2u), v_56.zw, v_56.xy)));
}

matrix<float16_t, 3, 3> v_57(uint start_byte_offset) {
  uint4 v_58 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_59 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_58.zw, v_58.xy)).xyz;
  uint v_60 = (8u + start_byte_offset);
  uint4 v_61 = ub[(v_60 / 16u)];
  vector<float16_t, 3> v_62 = tint_bitcast_to_f16_1(select((((v_60 & 15u) >> 2u) == 2u), v_61.zw, v_61.xy)).xyz;
  uint v_63 = (16u + start_byte_offset);
  uint4 v_64 = ub[(v_63 / 16u)];
  return matrix<float16_t, 3, 3>(v_59, v_62, tint_bitcast_to_f16_1(select((((v_63 & 15u) >> 2u) == 2u), v_64.zw, v_64.xy)).xyz);
}

matrix<float16_t, 3, 2> v_65(uint start_byte_offset) {
  vector<float16_t, 2> v_66 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_67 = (4u + start_byte_offset);
  vector<float16_t, 2> v_68 = tint_bitcast_to_f16(ub[(v_67 / 16u)][((v_67 & 15u) >> 2u)]);
  uint v_69 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_66, v_68, tint_bitcast_to_f16(ub[(v_69 / 16u)][((v_69 & 15u) >> 2u)]));
}

matrix<float16_t, 2, 4> v_70(uint start_byte_offset) {
  uint4 v_71 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_72 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_71.zw, v_71.xy));
  uint v_73 = (8u + start_byte_offset);
  uint4 v_74 = ub[(v_73 / 16u)];
  return matrix<float16_t, 2, 4>(v_72, tint_bitcast_to_f16_1(select((((v_73 & 15u) >> 2u) == 2u), v_74.zw, v_74.xy)));
}

matrix<float16_t, 2, 3> v_75(uint start_byte_offset) {
  uint4 v_76 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_77 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_76.zw, v_76.xy)).xyz;
  uint v_78 = (8u + start_byte_offset);
  uint4 v_79 = ub[(v_78 / 16u)];
  return matrix<float16_t, 2, 3>(v_77, tint_bitcast_to_f16_1(select((((v_78 & 15u) >> 2u) == 2u), v_79.zw, v_79.xy)).xyz);
}

matrix<float16_t, 2, 2> v_80(uint start_byte_offset) {
  vector<float16_t, 2> v_81 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_82 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_81, tint_bitcast_to_f16(ub[(v_82 / 16u)][((v_82 & 15u) >> 2u)]));
}

float4x4 v_83(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_84(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_85(uint start_byte_offset) {
  uint4 v_86 = ub[(start_byte_offset / 16u)];
  uint v_87 = (8u + start_byte_offset);
  uint4 v_88 = ub[(v_87 / 16u)];
  uint v_89 = (16u + start_byte_offset);
  uint4 v_90 = ub[(v_89 / 16u)];
  uint v_91 = (24u + start_byte_offset);
  uint4 v_92 = ub[(v_91 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_86.zw, v_86.xy)), asfloat(select((((v_87 & 15u) >> 2u) == 2u), v_88.zw, v_88.xy)), asfloat(select((((v_89 & 15u) >> 2u) == 2u), v_90.zw, v_90.xy)), asfloat(select((((v_91 & 15u) >> 2u) == 2u), v_92.zw, v_92.xy)));
}

float3x4 v_93(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_94(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_95(uint start_byte_offset) {
  uint4 v_96 = ub[(start_byte_offset / 16u)];
  uint v_97 = (8u + start_byte_offset);
  uint4 v_98 = ub[(v_97 / 16u)];
  uint v_99 = (16u + start_byte_offset);
  uint4 v_100 = ub[(v_99 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_96.zw, v_96.xy)), asfloat(select((((v_97 & 15u) >> 2u) == 2u), v_98.zw, v_98.xy)), asfloat(select((((v_99 & 15u) >> 2u) == 2u), v_100.zw, v_100.xy)));
}

float2x4 v_101(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_102(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_103(uint start_byte_offset) {
  uint4 v_104 = ub[(start_byte_offset / 16u)];
  uint v_105 = (8u + start_byte_offset);
  uint4 v_106 = ub[(v_105 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_104.zw, v_104.xy)), asfloat(select((((v_105 & 15u) >> 2u) == 2u), v_106.zw, v_106.xy)));
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
  float2x2 mat2x2_f32 = v_103(168u);
  float2x3 mat2x3_f32 = v_102(192u);
  float2x4 mat2x4_f32 = v_101(224u);
  float3x2 mat3x2_f32 = v_95(256u);
  float3x3 mat3x3_f32 = v_94(288u);
  float3x4 mat3x4_f32 = v_93(336u);
  float4x2 mat4x2_f32 = v_85(384u);
  float4x3 mat4x3_f32 = v_84(416u);
  float4x4 mat4x4_f32 = v_83(480u);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_80(544u);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_75(552u);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_70(568u);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_65(584u);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_57(600u);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_49(624u);
  matrix<float16_t, 4, 2> mat4x2_f16 = v_12(648u);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_38(664u);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_27(696u);
  float3 arr2_vec3_f32[2] = v_23(736u);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_19(768u);
  Inner struct_inner = v_1(800u);
  Inner array_struct_inner[4] = v_7(816u);
  int v_107 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_108 = asint((asuint(v_107) + asuint(int(scalar_u32))));
  int v_109 = asint((asuint(v_108) + asuint(tint_f16_to_i32(scalar_f16))));
  int v_110 = asint((asuint(asint((asuint(v_109) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_111 = asint((asuint(v_110) + asuint(int(vec2_u32.x))));
  int v_112 = asint((asuint(v_111) + asuint(tint_f16_to_i32(vec2_f16.x))));
  int v_113 = asint((asuint(asint((asuint(v_112) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_114 = asint((asuint(v_113) + asuint(int(vec3_u32.y))));
  int v_115 = asint((asuint(v_114) + asuint(tint_f16_to_i32(vec3_f16.y))));
  int v_116 = asint((asuint(asint((asuint(v_115) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_117 = asint((asuint(v_116) + asuint(int(vec4_u32.z))));
  int v_118 = asint((asuint(v_117) + asuint(tint_f16_to_i32(vec4_f16.z))));
  int v_119 = asint((asuint(v_118) + asuint(tint_f32_to_i32(mat2x2_f32[0u].x))));
  int v_120 = asint((asuint(v_119) + asuint(tint_f32_to_i32(mat2x3_f32[0u].x))));
  int v_121 = asint((asuint(v_120) + asuint(tint_f32_to_i32(mat2x4_f32[0u].x))));
  int v_122 = asint((asuint(v_121) + asuint(tint_f32_to_i32(mat3x2_f32[0u].x))));
  int v_123 = asint((asuint(v_122) + asuint(tint_f32_to_i32(mat3x3_f32[0u].x))));
  int v_124 = asint((asuint(v_123) + asuint(tint_f32_to_i32(mat3x4_f32[0u].x))));
  int v_125 = asint((asuint(v_124) + asuint(tint_f32_to_i32(mat4x2_f32[0u].x))));
  int v_126 = asint((asuint(v_125) + asuint(tint_f32_to_i32(mat4x3_f32[0u].x))));
  int v_127 = asint((asuint(v_126) + asuint(tint_f32_to_i32(mat4x4_f32[0u].x))));
  int v_128 = asint((asuint(v_127) + asuint(tint_f16_to_i32(mat2x2_f16[0u].x))));
  int v_129 = asint((asuint(v_128) + asuint(tint_f16_to_i32(mat2x3_f16[0u].x))));
  int v_130 = asint((asuint(v_129) + asuint(tint_f16_to_i32(mat2x4_f16[0u].x))));
  int v_131 = asint((asuint(v_130) + asuint(tint_f16_to_i32(mat3x2_f16[0u].x))));
  int v_132 = asint((asuint(v_131) + asuint(tint_f16_to_i32(mat3x3_f16[0u].x))));
  int v_133 = asint((asuint(v_132) + asuint(tint_f16_to_i32(mat3x4_f16[0u].x))));
  int v_134 = asint((asuint(v_133) + asuint(tint_f16_to_i32(mat4x2_f16[0u].x))));
  int v_135 = asint((asuint(v_134) + asuint(tint_f16_to_i32(mat4x3_f16[0u].x))));
  int v_136 = asint((asuint(v_135) + asuint(tint_f16_to_i32(mat4x4_f16[0u].x))));
  int v_137 = asint((asuint(v_136) + asuint(tint_f32_to_i32(arr2_vec3_f32[0u].x))));
  s.Store(0u, asuint(asint((asuint(asint((asuint(asint((asuint(v_137) + asuint(tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x))))) + asuint(struct_inner.scalar_i32)))) + asuint(array_struct_inner[0u].scalar_i32)))));
}

