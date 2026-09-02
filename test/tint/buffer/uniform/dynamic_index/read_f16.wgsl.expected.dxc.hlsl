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
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 2u)) {
        break;
      }
      a[v_10] = v_1((start_byte_offset + (v_10 * 16u)));
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
  matrix<float16_t, 4, 2> v_11[2] = a;
  return v_11;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_12(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 2u)) {
        break;
      }
      a[v_14] = asfloat(ub[((start_byte_offset + (v_14 * 16u)) / 16u)].xyz);
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  float3 v_15[2] = a;
  return v_15;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  vector<uint16_t, 4> v16 = vector<uint16_t, 4>(((v.xxyy >> uint4(0u, 16u, 0u, 16u)) & (65535u).xxxx));
  return asfloat16(v16);
}

matrix<float16_t, 4, 4> v_16(uint start_byte_offset) {
  uint4 v_17 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_18 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_17.zw, v_17.xy));
  uint v_19 = (8u + start_byte_offset);
  uint4 v_20 = ub[(v_19 / 16u)];
  vector<float16_t, 4> v_21 = tint_bitcast_to_f16_1(select((((v_19 & 15u) >> 2u) == 2u), v_20.zw, v_20.xy));
  uint v_22 = (16u + start_byte_offset);
  uint4 v_23 = ub[(v_22 / 16u)];
  vector<float16_t, 4> v_24 = tint_bitcast_to_f16_1(select((((v_22 & 15u) >> 2u) == 2u), v_23.zw, v_23.xy));
  uint v_25 = (24u + start_byte_offset);
  uint4 v_26 = ub[(v_25 / 16u)];
  return matrix<float16_t, 4, 4>(v_18, v_21, v_24, tint_bitcast_to_f16_1(select((((v_25 & 15u) >> 2u) == 2u), v_26.zw, v_26.xy)));
}

matrix<float16_t, 4, 3> v_27(uint start_byte_offset) {
  uint4 v_28 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_29 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_28.zw, v_28.xy)).xyz;
  uint v_30 = (8u + start_byte_offset);
  uint4 v_31 = ub[(v_30 / 16u)];
  vector<float16_t, 3> v_32 = tint_bitcast_to_f16_1(select((((v_30 & 15u) >> 2u) == 2u), v_31.zw, v_31.xy)).xyz;
  uint v_33 = (16u + start_byte_offset);
  uint4 v_34 = ub[(v_33 / 16u)];
  vector<float16_t, 3> v_35 = tint_bitcast_to_f16_1(select((((v_33 & 15u) >> 2u) == 2u), v_34.zw, v_34.xy)).xyz;
  uint v_36 = (24u + start_byte_offset);
  uint4 v_37 = ub[(v_36 / 16u)];
  return matrix<float16_t, 4, 3>(v_29, v_32, v_35, tint_bitcast_to_f16_1(select((((v_36 & 15u) >> 2u) == 2u), v_37.zw, v_37.xy)).xyz);
}

matrix<float16_t, 3, 4> v_38(uint start_byte_offset) {
  uint4 v_39 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_40 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_39.zw, v_39.xy));
  uint v_41 = (8u + start_byte_offset);
  uint4 v_42 = ub[(v_41 / 16u)];
  vector<float16_t, 4> v_43 = tint_bitcast_to_f16_1(select((((v_41 & 15u) >> 2u) == 2u), v_42.zw, v_42.xy));
  uint v_44 = (16u + start_byte_offset);
  uint4 v_45 = ub[(v_44 / 16u)];
  return matrix<float16_t, 3, 4>(v_40, v_43, tint_bitcast_to_f16_1(select((((v_44 & 15u) >> 2u) == 2u), v_45.zw, v_45.xy)));
}

matrix<float16_t, 3, 3> v_46(uint start_byte_offset) {
  uint4 v_47 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_48 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_47.zw, v_47.xy)).xyz;
  uint v_49 = (8u + start_byte_offset);
  uint4 v_50 = ub[(v_49 / 16u)];
  vector<float16_t, 3> v_51 = tint_bitcast_to_f16_1(select((((v_49 & 15u) >> 2u) == 2u), v_50.zw, v_50.xy)).xyz;
  uint v_52 = (16u + start_byte_offset);
  uint4 v_53 = ub[(v_52 / 16u)];
  return matrix<float16_t, 3, 3>(v_48, v_51, tint_bitcast_to_f16_1(select((((v_52 & 15u) >> 2u) == 2u), v_53.zw, v_53.xy)).xyz);
}

matrix<float16_t, 3, 2> v_54(uint start_byte_offset) {
  vector<float16_t, 2> v_55 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_56 = (4u + start_byte_offset);
  vector<float16_t, 2> v_57 = tint_bitcast_to_f16(ub[(v_56 / 16u)][((v_56 & 15u) >> 2u)]);
  uint v_58 = (8u + start_byte_offset);
  return matrix<float16_t, 3, 2>(v_55, v_57, tint_bitcast_to_f16(ub[(v_58 / 16u)][((v_58 & 15u) >> 2u)]));
}

matrix<float16_t, 2, 4> v_59(uint start_byte_offset) {
  uint4 v_60 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_61 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_60.zw, v_60.xy));
  uint v_62 = (8u + start_byte_offset);
  uint4 v_63 = ub[(v_62 / 16u)];
  return matrix<float16_t, 2, 4>(v_61, tint_bitcast_to_f16_1(select((((v_62 & 15u) >> 2u) == 2u), v_63.zw, v_63.xy)));
}

matrix<float16_t, 2, 3> v_64(uint start_byte_offset) {
  uint4 v_65 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_66 = tint_bitcast_to_f16_1(select((((start_byte_offset & 15u) >> 2u) == 2u), v_65.zw, v_65.xy)).xyz;
  uint v_67 = (8u + start_byte_offset);
  uint4 v_68 = ub[(v_67 / 16u)];
  return matrix<float16_t, 2, 3>(v_66, tint_bitcast_to_f16_1(select((((v_67 & 15u) >> 2u) == 2u), v_68.zw, v_68.xy)).xyz);
}

matrix<float16_t, 2, 2> v_69(uint start_byte_offset) {
  vector<float16_t, 2> v_70 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  uint v_71 = (4u + start_byte_offset);
  return matrix<float16_t, 2, 2>(v_70, tint_bitcast_to_f16(ub[(v_71 / 16u)][((v_71 & 15u) >> 2u)]));
}

float4x4 v_72(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_73(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_74(uint start_byte_offset) {
  uint4 v_75 = ub[(start_byte_offset / 16u)];
  uint v_76 = (8u + start_byte_offset);
  uint4 v_77 = ub[(v_76 / 16u)];
  uint v_78 = (16u + start_byte_offset);
  uint4 v_79 = ub[(v_78 / 16u)];
  uint v_80 = (24u + start_byte_offset);
  uint4 v_81 = ub[(v_80 / 16u)];
  return float4x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_75.zw, v_75.xy)), asfloat(select((((v_76 & 15u) >> 2u) == 2u), v_77.zw, v_77.xy)), asfloat(select((((v_78 & 15u) >> 2u) == 2u), v_79.zw, v_79.xy)), asfloat(select((((v_80 & 15u) >> 2u) == 2u), v_81.zw, v_81.xy)));
}

float3x4 v_82(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_83(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_84(uint start_byte_offset) {
  uint4 v_85 = ub[(start_byte_offset / 16u)];
  uint v_86 = (8u + start_byte_offset);
  uint4 v_87 = ub[(v_86 / 16u)];
  uint v_88 = (16u + start_byte_offset);
  uint4 v_89 = ub[(v_88 / 16u)];
  return float3x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_85.zw, v_85.xy)), asfloat(select((((v_86 & 15u) >> 2u) == 2u), v_87.zw, v_87.xy)), asfloat(select((((v_88 & 15u) >> 2u) == 2u), v_89.zw, v_89.xy)));
}

float2x4 v_90(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_91(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_92(uint start_byte_offset) {
  uint4 v_93 = ub[(start_byte_offset / 16u)];
  uint v_94 = (8u + start_byte_offset);
  uint4 v_95 = ub[(v_94 / 16u)];
  return float2x2(asfloat(select((((start_byte_offset & 15u) >> 2u) == 2u), v_93.zw, v_93.xy)), asfloat(select((((v_94 & 15u) >> 2u) == 2u), v_95.zw, v_95.xy)));
}

void main_inner(uint idx) {
  uint v_96 = (idx * 800u);
  float scalar_f32 = asfloat(ub[(v_96 / 16u)][((v_96 & 15u) >> 2u)]);
  uint v_97 = (4u + (idx * 800u));
  int scalar_i32 = asint(ub[(v_97 / 16u)][((v_97 & 15u) >> 2u)]);
  uint v_98 = (8u + (idx * 800u));
  uint scalar_u32 = ub[(v_98 / 16u)][((v_98 & 15u) >> 2u)];
  uint v_99 = (12u + (idx * 800u));
  float16_t scalar_f16 = tint_bitcast_to_f16(ub[(v_99 / 16u)][((v_99 & 15u) >> 2u)])[select(((v_99 % 4u) == 0u), 0u, 1u)];
  uint v_100 = (16u + (idx * 800u));
  uint4 v_101 = ub[(v_100 / 16u)];
  float2 vec2_f32 = asfloat(select((((v_100 & 15u) >> 2u) == 2u), v_101.zw, v_101.xy));
  uint v_102 = (24u + (idx * 800u));
  uint4 v_103 = ub[(v_102 / 16u)];
  int2 vec2_i32 = asint(select((((v_102 & 15u) >> 2u) == 2u), v_103.zw, v_103.xy));
  uint v_104 = (32u + (idx * 800u));
  uint4 v_105 = ub[(v_104 / 16u)];
  uint2 vec2_u32 = select((((v_104 & 15u) >> 2u) == 2u), v_105.zw, v_105.xy);
  uint v_106 = (40u + (idx * 800u));
  vector<float16_t, 2> vec2_f16 = tint_bitcast_to_f16(ub[(v_106 / 16u)][((v_106 & 15u) >> 2u)]);
  float3 vec3_f32 = asfloat(ub[((48u + (idx * 800u)) / 16u)].xyz);
  int3 vec3_i32 = asint(ub[((64u + (idx * 800u)) / 16u)].xyz);
  uint3 vec3_u32 = ub[((80u + (idx * 800u)) / 16u)].xyz;
  uint v_107 = (96u + (idx * 800u));
  uint4 v_108 = ub[(v_107 / 16u)];
  vector<float16_t, 3> vec3_f16 = tint_bitcast_to_f16_1(select((((v_107 & 15u) >> 2u) == 2u), v_108.zw, v_108.xy)).xyz;
  float4 vec4_f32 = asfloat(ub[((112u + (idx * 800u)) / 16u)]);
  int4 vec4_i32 = asint(ub[((128u + (idx * 800u)) / 16u)]);
  uint4 vec4_u32 = ub[((144u + (idx * 800u)) / 16u)];
  uint v_109 = (160u + (idx * 800u));
  uint4 v_110 = ub[(v_109 / 16u)];
  vector<float16_t, 4> vec4_f16 = tint_bitcast_to_f16_1(select((((v_109 & 15u) >> 2u) == 2u), v_110.zw, v_110.xy));
  float2x2 mat2x2_f32 = v_92((168u + (idx * 800u)));
  float2x3 mat2x3_f32 = v_91((192u + (idx * 800u)));
  float2x4 mat2x4_f32 = v_90((224u + (idx * 800u)));
  float3x2 mat3x2_f32 = v_84((256u + (idx * 800u)));
  float3x3 mat3x3_f32 = v_83((288u + (idx * 800u)));
  float3x4 mat3x4_f32 = v_82((336u + (idx * 800u)));
  float4x2 mat4x2_f32 = v_74((384u + (idx * 800u)));
  float4x3 mat4x3_f32 = v_73((416u + (idx * 800u)));
  float4x4 mat4x4_f32 = v_72((480u + (idx * 800u)));
  matrix<float16_t, 2, 2> mat2x2_f16 = v_69((544u + (idx * 800u)));
  matrix<float16_t, 2, 3> mat2x3_f16 = v_64((552u + (idx * 800u)));
  matrix<float16_t, 2, 4> mat2x4_f16 = v_59((568u + (idx * 800u)));
  matrix<float16_t, 3, 2> mat3x2_f16 = v_54((584u + (idx * 800u)));
  matrix<float16_t, 3, 3> mat3x3_f16 = v_46((600u + (idx * 800u)));
  matrix<float16_t, 3, 4> mat3x4_f16 = v_38((624u + (idx * 800u)));
  matrix<float16_t, 4, 2> mat4x2_f16 = v_1((648u + (idx * 800u)));
  matrix<float16_t, 4, 3> mat4x3_f16 = v_27((664u + (idx * 800u)));
  matrix<float16_t, 4, 4> mat4x4_f16 = v_16((696u + (idx * 800u)));
  float3 arr2_vec3_f32[2] = v_12((736u + (idx * 800u)));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_8((768u + (idx * 800u)));
  int v_111 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_112 = asint((asuint(v_111) + asuint(int(scalar_u32))));
  int v_113 = asint((asuint(v_112) + asuint(tint_f16_to_i32(scalar_f16))));
  int v_114 = asint((asuint(asint((asuint(v_113) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_115 = asint((asuint(v_114) + asuint(int(vec2_u32.x))));
  int v_116 = asint((asuint(v_115) + asuint(tint_f16_to_i32(vec2_f16.x))));
  int v_117 = asint((asuint(asint((asuint(v_116) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_118 = asint((asuint(v_117) + asuint(int(vec3_u32.y))));
  int v_119 = asint((asuint(v_118) + asuint(tint_f16_to_i32(vec3_f16.y))));
  int v_120 = asint((asuint(asint((asuint(v_119) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_121 = asint((asuint(v_120) + asuint(int(vec4_u32.z))));
  int v_122 = asint((asuint(v_121) + asuint(tint_f16_to_i32(vec4_f16.z))));
  int v_123 = asint((asuint(v_122) + asuint(tint_f32_to_i32(mat2x2_f32[0u].x))));
  int v_124 = asint((asuint(v_123) + asuint(tint_f32_to_i32(mat2x3_f32[0u].x))));
  int v_125 = asint((asuint(v_124) + asuint(tint_f32_to_i32(mat2x4_f32[0u].x))));
  int v_126 = asint((asuint(v_125) + asuint(tint_f32_to_i32(mat3x2_f32[0u].x))));
  int v_127 = asint((asuint(v_126) + asuint(tint_f32_to_i32(mat3x3_f32[0u].x))));
  int v_128 = asint((asuint(v_127) + asuint(tint_f32_to_i32(mat3x4_f32[0u].x))));
  int v_129 = asint((asuint(v_128) + asuint(tint_f32_to_i32(mat4x2_f32[0u].x))));
  int v_130 = asint((asuint(v_129) + asuint(tint_f32_to_i32(mat4x3_f32[0u].x))));
  int v_131 = asint((asuint(v_130) + asuint(tint_f32_to_i32(mat4x4_f32[0u].x))));
  int v_132 = asint((asuint(v_131) + asuint(tint_f16_to_i32(mat2x2_f16[0u].x))));
  int v_133 = asint((asuint(v_132) + asuint(tint_f16_to_i32(mat2x3_f16[0u].x))));
  int v_134 = asint((asuint(v_133) + asuint(tint_f16_to_i32(mat2x4_f16[0u].x))));
  int v_135 = asint((asuint(v_134) + asuint(tint_f16_to_i32(mat3x2_f16[0u].x))));
  int v_136 = asint((asuint(v_135) + asuint(tint_f16_to_i32(mat3x3_f16[0u].x))));
  int v_137 = asint((asuint(v_136) + asuint(tint_f16_to_i32(mat3x4_f16[0u].x))));
  int v_138 = asint((asuint(v_137) + asuint(tint_f16_to_i32(mat4x2_f16[0u].x))));
  int v_139 = asint((asuint(v_138) + asuint(tint_f16_to_i32(mat4x3_f16[0u].x))));
  int v_140 = asint((asuint(v_139) + asuint(tint_f16_to_i32(mat4x4_f16[0u].x))));
  int v_141 = asint((asuint(v_140) + asuint(tint_f32_to_i32(arr2_vec3_f32[0u].x))));
  s.Store(0u, asuint(asint((asuint(v_141) + asuint(tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x))))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

