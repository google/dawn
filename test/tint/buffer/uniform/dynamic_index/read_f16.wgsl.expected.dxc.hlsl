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
  uint2 v_1 = uint2(v, v);
  vector<uint16_t, 2> v16 = vector<uint16_t, 2>(((v_1 >> uint2(0u, 16u)) & (65535u).xx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_4 = (4u + start_byte_offset);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(ub[(v_4 / 16u)][((v_4 & 15u) >> 2u)]);
  uint v_6 = (8u + start_byte_offset);
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16(ub[(v_6 / 16u)][((v_6 & 15u) >> 2u)]);
  uint v_8 = (12u + start_byte_offset);
  return matrix<float16_t, 4, 2>(v_3, v_5, v_7, tint_bitcast_to_f16(ub[(v_8 / 16u)][((v_8 & 15u) >> 2u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret[2];
ary_ret v_9(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 2u)) {
        break;
      }
      a[v_11] = v_2((start_byte_offset + (v_11 * 16u)));
      {
        v_10 = (v_11 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_12[2] = a;
  return v_12;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_13(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 2u)) {
        break;
      }
      a[v_15] = asfloat(ub[((start_byte_offset + (v_15 * 16u)) / 16u)].xyz);
      {
        v_14 = (v_15 + 1u);
      }
    }
  }
  float3 v_16[2] = a;
  return v_16;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 4> v_17(uint start_byte_offset) {
  uint4 v_18 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_19 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_18.zw, v_18.xy));
  uint v_20 = (8u + start_byte_offset);
  uint4 v_21 = ub[(v_20 / 16u)];
  vector<float16_t, 4> v_22 = tint_bitcast_to_f16_1(select((((v_20 & 15u) >> 2u) == 2u), v_21.zw, v_21.xy));
  uint v_23 = (16u + start_byte_offset);
  uint4 v_24 = ub[(v_23 / 16u)];
  vector<float16_t, 4> v_25 = tint_bitcast_to_f16_1(select((((v_23 & 15u) >> 2u) == 2u), v_24.zw, v_24.xy));
  uint v_26 = (24u + start_byte_offset);
  uint4 v_27 = ub[(v_26 / 16u)];
  return matrix<float16_t, 4, 4>(v_19, v_22, v_25, tint_bitcast_to_f16_1(select((((v_26 & 15u) >> 2u) == 2u), v_27.zw, v_27.xy)));
}

matrix<float16_t, 4, 3> v_28(uint start_byte_offset) {
  uint4 v_29 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_30 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_29.zw, v_29.xy)).xyz;
  uint v_31 = (8u + start_byte_offset);
  uint4 v_32 = ub[(v_31 / 16u)];
  vector<float16_t, 3> v_33 = tint_bitcast_to_f16_1(select((((v_31 & 15u) >> 2u) == 2u), v_32.zw, v_32.xy)).xyz;
  uint v_34 = (16u + start_byte_offset);
  uint4 v_35 = ub[(v_34 / 16u)];
  vector<float16_t, 3> v_36 = tint_bitcast_to_f16_1(select((((v_34 & 15u) >> 2u) == 2u), v_35.zw, v_35.xy)).xyz;
  uint v_37 = (24u + start_byte_offset);
  uint4 v_38 = ub[(v_37 / 16u)];
  return matrix<float16_t, 4, 3>(v_30, v_33, v_36, tint_bitcast_to_f16_1(select((((v_37 & 15u) >> 2u) == 2u), v_38.zw, v_38.xy)).xyz);
}

matrix<float16_t, 3, 4> v_39(uint start_byte_offset) {
  uint4 v_40 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_41 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_40.zw, v_40.xy));
  uint v_42 = (8u + start_byte_offset);
  uint4 v_43 = ub[(v_42 / 16u)];
  vector<float16_t, 4> v_44 = tint_bitcast_to_f16_1(select((((v_42 & 15u) >> 2u) == 2u), v_43.zw, v_43.xy));
  uint v_45 = (16u + start_byte_offset);
  uint4 v_46 = ub[(v_45 / 16u)];
  return matrix<float16_t, 3, 4>(v_41, v_44, tint_bitcast_to_f16_1(select((((v_45 & 15u) >> 2u) == 2u), v_46.zw, v_46.xy)));
}

matrix<float16_t, 3, 3> v_47(uint start_byte_offset) {
  uint4 v_48 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_49 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_48.zw, v_48.xy)).xyz;
  uint v_50 = (8u + start_byte_offset);
  uint4 v_51 = ub[(v_50 / 16u)];
  vector<float16_t, 3> v_52 = tint_bitcast_to_f16_1(select((((v_50 & 15u) >> 2u) == 2u), v_51.zw, v_51.xy)).xyz;
  uint v_53 = (16u + start_byte_offset);
  uint4 v_54 = ub[(v_53 / 16u)];
  return matrix<float16_t, 3, 3>(v_49, v_52, tint_bitcast_to_f16_1(select((((v_53 & 15u) >> 2u) == 2u), v_54.zw, v_54.xy)).xyz);
}

matrix<float16_t, 3, 2> v_55(uint start_byte_offset) {
  vector<float16_t, 2> v_56 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_57 = (4u + start_byte_offset);
  vector<float16_t, 2> v_58 = tint_bitcast_to_f16(ub[(v_57 / 16u)][((v_57 & 15u) >> 2u)]);
  uint v_59 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_56, v_58, tint_bitcast_to_f16(ub[(v_59 / 16u)][((v_59 & 15u) >> 2u)]));
}

matrix<float16_t, 2, 4> v_60(uint start_byte_offset) {
  uint4 v_61 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_62 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_61.zw, v_61.xy));
  uint v_63 = (8u + start_byte_offset);
  uint4 v_64 = ub[(v_63 / 16u)];
  return matrix<float16_t, 2, 4>(v_62, tint_bitcast_to_f16_1(select((((v_63 & 15u) >> 2u) == 2u), v_64.zw, v_64.xy)));
}

matrix<float16_t, 2, 3> v_65(uint start_byte_offset) {
  uint4 v_66 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_67 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_66.zw, v_66.xy)).xyz;
  uint v_68 = (8u + start_byte_offset);
  uint4 v_69 = ub[(v_68 / 16u)];
  return matrix<float16_t, 2, 3>(v_67, tint_bitcast_to_f16_1(select((((v_68 & 15u) >> 2u) == 2u), v_69.zw, v_69.xy)).xyz);
}

matrix<float16_t, 2, 2> v_70(uint start_byte_offset) {
  vector<float16_t, 2> v_71 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_72 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_71, tint_bitcast_to_f16(ub[(v_72 / 16u)][((v_72 & 15u) >> 2u)]));
}

float4x4 v_73(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_74(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_75(uint start_byte_offset) {
  uint4 v_76 = ub[(start_byte_offset / 16u)];
  uint v_77 = (8u + start_byte_offset);
  uint4 v_78 = ub[(v_77 / 16u)];
  uint v_79 = (16u + start_byte_offset);
  uint4 v_80 = ub[(v_79 / 16u)];
  uint v_81 = (24u + start_byte_offset);
  uint4 v_82 = ub[(v_81 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_76.zw, v_76.xy)), asfloat(select((((v_77 & 15u) >> 2u) == 2u), v_78.zw, v_78.xy)), asfloat(select((((v_79 & 15u) >> 2u) == 2u), v_80.zw, v_80.xy)), asfloat(select((((v_81 & 15u) >> 2u) == 2u), v_82.zw, v_82.xy)));
}

float3x4 v_83(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_84(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_85(uint start_byte_offset) {
  uint4 v_86 = ub[(start_byte_offset / 16u)];
  uint v_87 = (8u + start_byte_offset);
  uint4 v_88 = ub[(v_87 / 16u)];
  uint v_89 = (16u + start_byte_offset);
  uint4 v_90 = ub[(v_89 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_86.zw, v_86.xy)), asfloat(select((((v_87 & 15u) >> 2u) == 2u), v_88.zw, v_88.xy)), asfloat(select((((v_89 & 15u) >> 2u) == 2u), v_90.zw, v_90.xy)));
}

float2x4 v_91(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_92(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_93(uint start_byte_offset) {
  uint4 v_94 = ub[(start_byte_offset / 16u)];
  uint v_95 = (8u + start_byte_offset);
  uint4 v_96 = ub[(v_95 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_94.zw, v_94.xy)), asfloat(select((((v_95 & 15u) >> 2u) == 2u), v_96.zw, v_96.xy)));
}

void main_inner(uint idx) {
  uint v_97 = (idx * 800u);
  float scalar_f32 = asfloat(ub[(v_97 / 16u)][((v_97 & 15u) >> 2u)]);
  uint v_98 = (4u + (idx * 800u));
  int scalar_i32 = asint(ub[(v_98 / 16u)][((v_98 & 15u) >> 2u)]);
  uint v_99 = (8u + (idx * 800u));
  uint scalar_u32 = ub[(v_99 / 16u)][((v_99 & 15u) >> 2u)];
  uint v_100 = (12u + (idx * 800u));
  float16_t scalar_f16 = tint_bitcast_to_f16(ub[(v_100 / 16u)][((v_100 & 15u) >> 2u)])[select(((v_100 % 4u) == 0u), 0u, 1u)];
  uint v_101 = (16u + (idx * 800u));
  uint4 v_102 = ub[(v_101 / 16u)];
  float2 vec2_f32 = asfloat(select((((v_101 & 15u) >> 2u) == 2u), v_102.zw, v_102.xy));
  uint v_103 = (24u + (idx * 800u));
  uint4 v_104 = ub[(v_103 / 16u)];
  int2 vec2_i32 = asint(select((((v_103 & 15u) >> 2u) == 2u), v_104.zw, v_104.xy));
  uint v_105 = (32u + (idx * 800u));
  uint4 v_106 = ub[(v_105 / 16u)];
  uint2 vec2_u32 = select((((v_105 & 15u) >> 2u) == 2u), v_106.zw, v_106.xy);
  uint v_107 = (40u + (idx * 800u));
  vector<float16_t, 2> vec2_f16 = tint_bitcast_to_f16(ub[(v_107 / 16u)][((v_107 & 15u) >> 2u)]);
  float3 vec3_f32 = asfloat(ub[((48u + (idx * 800u)) / 16u)].xyz);
  int3 vec3_i32 = asint(ub[((64u + (idx * 800u)) / 16u)].xyz);
  uint3 vec3_u32 = ub[((80u + (idx * 800u)) / 16u)].xyz;
  uint v_108 = (96u + (idx * 800u));
  uint4 v_109 = ub[(v_108 / 16u)];
  vector<float16_t, 3> vec3_f16 = tint_bitcast_to_f16_1(select((((v_108 & 15u) >> 2u) == 2u), v_109.zw, v_109.xy)).xyz;
  float4 vec4_f32 = asfloat(ub[((112u + (idx * 800u)) / 16u)]);
  int4 vec4_i32 = asint(ub[((128u + (idx * 800u)) / 16u)]);
  uint4 vec4_u32 = ub[((144u + (idx * 800u)) / 16u)];
  uint v_110 = (160u + (idx * 800u));
  uint4 v_111 = ub[(v_110 / 16u)];
  vector<float16_t, 4> vec4_f16 = tint_bitcast_to_f16_1(select((((v_110 & 15u) >> 2u) == 2u), v_111.zw, v_111.xy));
  float2x2 mat2x2_f32 = v_93((168u + (idx * 800u)));
  float2x3 mat2x3_f32 = v_92((192u + (idx * 800u)));
  float2x4 mat2x4_f32 = v_91((224u + (idx * 800u)));
  float3x2 mat3x2_f32 = v_85((256u + (idx * 800u)));
  float3x3 mat3x3_f32 = v_84((288u + (idx * 800u)));
  float3x4 mat3x4_f32 = v_83((336u + (idx * 800u)));
  float4x2 mat4x2_f32 = v_75((384u + (idx * 800u)));
  float4x3 mat4x3_f32 = v_74((416u + (idx * 800u)));
  float4x4 mat4x4_f32 = v_73((480u + (idx * 800u)));
  matrix<float16_t, 2, 2> mat2x2_f16 = v_70((544u + (idx * 800u)));
  matrix<float16_t, 2, 3> mat2x3_f16 = v_65((552u + (idx * 800u)));
  matrix<float16_t, 2, 4> mat2x4_f16 = v_60((568u + (idx * 800u)));
  matrix<float16_t, 3, 2> mat3x2_f16 = v_55((584u + (idx * 800u)));
  matrix<float16_t, 3, 3> mat3x3_f16 = v_47((600u + (idx * 800u)));
  matrix<float16_t, 3, 4> mat3x4_f16 = v_39((624u + (idx * 800u)));
  matrix<float16_t, 4, 2> mat4x2_f16 = v_2((648u + (idx * 800u)));
  matrix<float16_t, 4, 3> mat4x3_f16 = v_28((664u + (idx * 800u)));
  matrix<float16_t, 4, 4> mat4x4_f16 = v_17((696u + (idx * 800u)));
  float3 arr2_vec3_f32[2] = v_13((736u + (idx * 800u)));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_9((768u + (idx * 800u)));
  int v_112 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_113 = asint((asuint(v_112) + asuint(int(scalar_u32))));
  int v_114 = asint((asuint(v_113) + asuint(tint_f16_to_i32(scalar_f16))));
  int v_115 = asint((asuint(asint((asuint(v_114) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_116 = asint((asuint(v_115) + asuint(int(vec2_u32.x))));
  int v_117 = asint((asuint(v_116) + asuint(tint_f16_to_i32(vec2_f16.x))));
  int v_118 = asint((asuint(asint((asuint(v_117) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_119 = asint((asuint(v_118) + asuint(int(vec3_u32.y))));
  int v_120 = asint((asuint(v_119) + asuint(tint_f16_to_i32(vec3_f16.y))));
  int v_121 = asint((asuint(asint((asuint(v_120) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_122 = asint((asuint(v_121) + asuint(int(vec4_u32.z))));
  int v_123 = asint((asuint(v_122) + asuint(tint_f16_to_i32(vec4_f16.z))));
  int v_124 = asint((asuint(v_123) + asuint(tint_f32_to_i32(mat2x2_f32[0u].x))));
  int v_125 = asint((asuint(v_124) + asuint(tint_f32_to_i32(mat2x3_f32[0u].x))));
  int v_126 = asint((asuint(v_125) + asuint(tint_f32_to_i32(mat2x4_f32[0u].x))));
  int v_127 = asint((asuint(v_126) + asuint(tint_f32_to_i32(mat3x2_f32[0u].x))));
  int v_128 = asint((asuint(v_127) + asuint(tint_f32_to_i32(mat3x3_f32[0u].x))));
  int v_129 = asint((asuint(v_128) + asuint(tint_f32_to_i32(mat3x4_f32[0u].x))));
  int v_130 = asint((asuint(v_129) + asuint(tint_f32_to_i32(mat4x2_f32[0u].x))));
  int v_131 = asint((asuint(v_130) + asuint(tint_f32_to_i32(mat4x3_f32[0u].x))));
  int v_132 = asint((asuint(v_131) + asuint(tint_f32_to_i32(mat4x4_f32[0u].x))));
  int v_133 = asint((asuint(v_132) + asuint(tint_f16_to_i32(mat2x2_f16[0u].x))));
  int v_134 = asint((asuint(v_133) + asuint(tint_f16_to_i32(mat2x3_f16[0u].x))));
  int v_135 = asint((asuint(v_134) + asuint(tint_f16_to_i32(mat2x4_f16[0u].x))));
  int v_136 = asint((asuint(v_135) + asuint(tint_f16_to_i32(mat3x2_f16[0u].x))));
  int v_137 = asint((asuint(v_136) + asuint(tint_f16_to_i32(mat3x3_f16[0u].x))));
  int v_138 = asint((asuint(v_137) + asuint(tint_f16_to_i32(mat3x4_f16[0u].x))));
  int v_139 = asint((asuint(v_138) + asuint(tint_f16_to_i32(mat4x2_f16[0u].x))));
  int v_140 = asint((asuint(v_139) + asuint(tint_f16_to_i32(mat4x3_f16[0u].x))));
  int v_141 = asint((asuint(v_140) + asuint(tint_f16_to_i32(mat4x4_f16[0u].x))));
  int v_142 = asint((asuint(v_141) + asuint(tint_f32_to_i32(arr2_vec3_f32[0u].x))));
  s.Store(0u, asuint(asint((asuint(v_142) + asuint(tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x))))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

