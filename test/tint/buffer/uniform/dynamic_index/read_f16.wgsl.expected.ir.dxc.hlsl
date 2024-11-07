struct main_inputs {
  uint idx : SV_GroupIndex;
};


cbuffer cbuffer_ub : register(b0) {
  uint4 ub[400];
};
RWByteAddressBuffer s : register(u1);
int tint_f16_to_i32(float16_t value) {
  return (((value <= float16_t(65504.0h))) ? ((((value >= float16_t(-65504.0h))) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 4, 2> v_2(uint start_byte_offset) {
  vector<float16_t, 2> v_3 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  vector<float16_t, 2> v_5 = tint_bitcast_to_f16(ub[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 4, 2>(v_3, v_4, v_5, tint_bitcast_to_f16(ub[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) % 16u) / 4u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret[2];
ary_ret v_6(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 2u)) {
        break;
      }
      a[v_8] = v_2((start_byte_offset + (v_8 * 16u)));
      {
        v_7 = (v_8 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_9[2] = a;
  return v_9;
}

typedef float3 ary_ret_1[2];
ary_ret_1 v_10(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_11 = 0u;
    v_11 = 0u;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 2u)) {
        break;
      }
      a[v_12] = asfloat(ub[((start_byte_offset + (v_12 * 16u)) / 16u)].xyz);
      {
        v_11 = (v_12 + 1u);
      }
      continue;
    }
  }
  float3 v_13[2] = a;
  return v_13;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_14 = float16_t(t_low.x);
  float16_t v_15 = float16_t(t_high.x);
  float16_t v_16 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_14, v_15, v_16, float16_t(t_high.y));
}

matrix<float16_t, 4, 4> v_17(uint start_byte_offset) {
  uint4 v_18 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_19 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_18.zw) : (v_18.xy)));
  uint4 v_20 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_21 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_20.zw) : (v_20.xy)));
  uint4 v_22 = ub[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_23 = tint_bitcast_to_f16_1(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_22.zw) : (v_22.xy)));
  uint4 v_24 = ub[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 4>(v_19, v_21, v_23, tint_bitcast_to_f16_1(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_24.zw) : (v_24.xy))));
}

matrix<float16_t, 4, 3> v_25(uint start_byte_offset) {
  uint4 v_26 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_27 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_26.zw) : (v_26.xy))).xyz;
  uint4 v_28 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_29 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_28.zw) : (v_28.xy))).xyz;
  uint4 v_30 = ub[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_31 = tint_bitcast_to_f16_1(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_30.zw) : (v_30.xy))).xyz;
  uint4 v_32 = ub[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 3>(v_27, v_29, v_31, tint_bitcast_to_f16_1(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_32.zw) : (v_32.xy))).xyz);
}

matrix<float16_t, 3, 4> v_33(uint start_byte_offset) {
  uint4 v_34 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_35 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_34.zw) : (v_34.xy)));
  uint4 v_36 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_37 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_36.zw) : (v_36.xy)));
  uint4 v_38 = ub[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 4>(v_35, v_37, tint_bitcast_to_f16_1(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_38.zw) : (v_38.xy))));
}

matrix<float16_t, 3, 3> v_39(uint start_byte_offset) {
  uint4 v_40 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_41 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_40.zw) : (v_40.xy))).xyz;
  uint4 v_42 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_43 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_42.zw) : (v_42.xy))).xyz;
  uint4 v_44 = ub[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 3>(v_41, v_43, tint_bitcast_to_f16_1(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_44.zw) : (v_44.xy))).xyz);
}

matrix<float16_t, 3, 2> v_45(uint start_byte_offset) {
  vector<float16_t, 2> v_46 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_47 = tint_bitcast_to_f16(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 3, 2>(v_46, v_47, tint_bitcast_to_f16(ub[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]));
}

matrix<float16_t, 2, 4> v_48(uint start_byte_offset) {
  uint4 v_49 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_50 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_49.zw) : (v_49.xy)));
  uint4 v_51 = ub[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 4>(v_50, tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_51.zw) : (v_51.xy))));
}

matrix<float16_t, 2, 3> v_52(uint start_byte_offset) {
  uint4 v_53 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_54 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_53.zw) : (v_53.xy))).xyz;
  uint4 v_55 = ub[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_54, tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_55.zw) : (v_55.xy))).xyz);
}

matrix<float16_t, 2, 2> v_56(uint start_byte_offset) {
  vector<float16_t, 2> v_57 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  return matrix<float16_t, 2, 2>(v_57, tint_bitcast_to_f16(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]));
}

float4x4 v_58(uint start_byte_offset) {
  float4 v_59 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_60 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  float4 v_61 = asfloat(ub[((32u + start_byte_offset) / 16u)]);
  return float4x4(v_59, v_60, v_61, asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_62(uint start_byte_offset) {
  float3 v_63 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_64 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  float3 v_65 = asfloat(ub[((32u + start_byte_offset) / 16u)].xyz);
  return float4x3(v_63, v_64, v_65, asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_66(uint start_byte_offset) {
  uint4 v_67 = ub[(start_byte_offset / 16u)];
  float2 v_68 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_67.zw) : (v_67.xy)));
  uint4 v_69 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_70 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_69.zw) : (v_69.xy)));
  uint4 v_71 = ub[((16u + start_byte_offset) / 16u)];
  float2 v_72 = asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_71.zw) : (v_71.xy)));
  uint4 v_73 = ub[((24u + start_byte_offset) / 16u)];
  return float4x2(v_68, v_70, v_72, asfloat(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_73.zw) : (v_73.xy))));
}

float3x4 v_74(uint start_byte_offset) {
  float4 v_75 = asfloat(ub[(start_byte_offset / 16u)]);
  float4 v_76 = asfloat(ub[((16u + start_byte_offset) / 16u)]);
  return float3x4(v_75, v_76, asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_77(uint start_byte_offset) {
  float3 v_78 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  float3 v_79 = asfloat(ub[((16u + start_byte_offset) / 16u)].xyz);
  return float3x3(v_78, v_79, asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_80(uint start_byte_offset) {
  uint4 v_81 = ub[(start_byte_offset / 16u)];
  float2 v_82 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_81.zw) : (v_81.xy)));
  uint4 v_83 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_84 = asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_83.zw) : (v_83.xy)));
  uint4 v_85 = ub[((16u + start_byte_offset) / 16u)];
  return float3x2(v_82, v_84, asfloat(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_85.zw) : (v_85.xy))));
}

float2x4 v_86(uint start_byte_offset) {
  float4 v_87 = asfloat(ub[(start_byte_offset / 16u)]);
  return float2x4(v_87, asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_88(uint start_byte_offset) {
  float3 v_89 = asfloat(ub[(start_byte_offset / 16u)].xyz);
  return float2x3(v_89, asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_90(uint start_byte_offset) {
  uint4 v_91 = ub[(start_byte_offset / 16u)];
  float2 v_92 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_91.zw) : (v_91.xy)));
  uint4 v_93 = ub[((8u + start_byte_offset) / 16u)];
  return float2x2(v_92, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_93.zw) : (v_93.xy))));
}

void main_inner(uint idx) {
  uint v_94 = (800u * uint(idx));
  float scalar_f32 = asfloat(ub[(v_94 / 16u)][((v_94 % 16u) / 4u)]);
  uint v_95 = (4u + (800u * uint(idx)));
  int scalar_i32 = asint(ub[(v_95 / 16u)][((v_95 % 16u) / 4u)]);
  uint v_96 = (8u + (800u * uint(idx)));
  uint scalar_u32 = ub[(v_96 / 16u)][((v_96 % 16u) / 4u)];
  uint v_97 = (12u + (800u * uint(idx)));
  uint v_98 = ub[(v_97 / 16u)][((v_97 % 16u) / 4u)];
  float16_t scalar_f16 = float16_t(f16tof32((v_98 >> ((((v_97 % 4u) == 0u)) ? (0u) : (16u)))));
  uint v_99 = (16u + (800u * uint(idx)));
  uint4 v_100 = ub[(v_99 / 16u)];
  float2 vec2_f32 = asfloat((((((v_99 % 16u) / 4u) == 2u)) ? (v_100.zw) : (v_100.xy)));
  uint v_101 = (24u + (800u * uint(idx)));
  uint4 v_102 = ub[(v_101 / 16u)];
  int2 vec2_i32 = asint((((((v_101 % 16u) / 4u) == 2u)) ? (v_102.zw) : (v_102.xy)));
  uint v_103 = (32u + (800u * uint(idx)));
  uint4 v_104 = ub[(v_103 / 16u)];
  uint2 vec2_u32 = (((((v_103 % 16u) / 4u) == 2u)) ? (v_104.zw) : (v_104.xy));
  uint v_105 = (40u + (800u * uint(idx)));
  vector<float16_t, 2> vec2_f16 = tint_bitcast_to_f16(ub[(v_105 / 16u)][((v_105 % 16u) / 4u)]);
  uint v_106 = ((48u + (800u * uint(idx))) / 16u);
  float3 vec3_f32 = asfloat(ub[v_106].xyz);
  uint v_107 = ((64u + (800u * uint(idx))) / 16u);
  int3 vec3_i32 = asint(ub[v_107].xyz);
  uint v_108 = ((80u + (800u * uint(idx))) / 16u);
  uint3 vec3_u32 = ub[v_108].xyz;
  uint v_109 = (96u + (800u * uint(idx)));
  uint4 v_110 = ub[(v_109 / 16u)];
  vector<float16_t, 3> vec3_f16 = tint_bitcast_to_f16_1((((((v_109 % 16u) / 4u) == 2u)) ? (v_110.zw) : (v_110.xy))).xyz;
  uint v_111 = ((112u + (800u * uint(idx))) / 16u);
  float4 vec4_f32 = asfloat(ub[v_111]);
  uint v_112 = ((128u + (800u * uint(idx))) / 16u);
  int4 vec4_i32 = asint(ub[v_112]);
  uint v_113 = ((144u + (800u * uint(idx))) / 16u);
  uint4 vec4_u32 = ub[v_113];
  uint v_114 = (160u + (800u * uint(idx)));
  uint4 v_115 = ub[(v_114 / 16u)];
  vector<float16_t, 4> vec4_f16 = tint_bitcast_to_f16_1((((((v_114 % 16u) / 4u) == 2u)) ? (v_115.zw) : (v_115.xy)));
  float2x2 mat2x2_f32 = v_90((168u + (800u * uint(idx))));
  float2x3 mat2x3_f32 = v_88((192u + (800u * uint(idx))));
  float2x4 mat2x4_f32 = v_86((224u + (800u * uint(idx))));
  float3x2 mat3x2_f32 = v_80((256u + (800u * uint(idx))));
  float3x3 mat3x3_f32 = v_77((288u + (800u * uint(idx))));
  float3x4 mat3x4_f32 = v_74((336u + (800u * uint(idx))));
  float4x2 mat4x2_f32 = v_66((384u + (800u * uint(idx))));
  float4x3 mat4x3_f32 = v_62((416u + (800u * uint(idx))));
  float4x4 mat4x4_f32 = v_58((480u + (800u * uint(idx))));
  matrix<float16_t, 2, 2> mat2x2_f16 = v_56((544u + (800u * uint(idx))));
  matrix<float16_t, 2, 3> mat2x3_f16 = v_52((552u + (800u * uint(idx))));
  matrix<float16_t, 2, 4> mat2x4_f16 = v_48((568u + (800u * uint(idx))));
  matrix<float16_t, 3, 2> mat3x2_f16 = v_45((584u + (800u * uint(idx))));
  matrix<float16_t, 3, 3> mat3x3_f16 = v_39((600u + (800u * uint(idx))));
  matrix<float16_t, 3, 4> mat3x4_f16 = v_33((624u + (800u * uint(idx))));
  matrix<float16_t, 4, 2> mat4x2_f16 = v_2((648u + (800u * uint(idx))));
  matrix<float16_t, 4, 3> mat4x3_f16 = v_25((664u + (800u * uint(idx))));
  matrix<float16_t, 4, 4> mat4x4_f16 = v_17((696u + (800u * uint(idx))));
  float3 arr2_vec3_f32[2] = v_10((736u + (800u * uint(idx))));
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_6((768u + (800u * uint(idx))));
  int v_116 = (tint_f32_to_i32(scalar_f32) + scalar_i32);
  int v_117 = (v_116 + int(scalar_u32));
  int v_118 = (v_117 + tint_f16_to_i32(scalar_f16));
  int v_119 = ((v_118 + tint_f32_to_i32(vec2_f32[0u])) + vec2_i32[0u]);
  int v_120 = (v_119 + int(vec2_u32[0u]));
  int v_121 = (v_120 + tint_f16_to_i32(vec2_f16[0u]));
  int v_122 = ((v_121 + tint_f32_to_i32(vec3_f32[1u])) + vec3_i32[1u]);
  int v_123 = (v_122 + int(vec3_u32[1u]));
  int v_124 = (v_123 + tint_f16_to_i32(vec3_f16[1u]));
  int v_125 = ((v_124 + tint_f32_to_i32(vec4_f32[2u])) + vec4_i32[2u]);
  int v_126 = (v_125 + int(vec4_u32[2u]));
  int v_127 = (v_126 + tint_f16_to_i32(vec4_f16[2u]));
  int v_128 = (v_127 + tint_f32_to_i32(mat2x2_f32[int(0)][0u]));
  int v_129 = (v_128 + tint_f32_to_i32(mat2x3_f32[int(0)][0u]));
  int v_130 = (v_129 + tint_f32_to_i32(mat2x4_f32[int(0)][0u]));
  int v_131 = (v_130 + tint_f32_to_i32(mat3x2_f32[int(0)][0u]));
  int v_132 = (v_131 + tint_f32_to_i32(mat3x3_f32[int(0)][0u]));
  int v_133 = (v_132 + tint_f32_to_i32(mat3x4_f32[int(0)][0u]));
  int v_134 = (v_133 + tint_f32_to_i32(mat4x2_f32[int(0)][0u]));
  int v_135 = (v_134 + tint_f32_to_i32(mat4x3_f32[int(0)][0u]));
  int v_136 = (v_135 + tint_f32_to_i32(mat4x4_f32[int(0)][0u]));
  int v_137 = (v_136 + tint_f16_to_i32(mat2x2_f16[int(0)][0u]));
  int v_138 = (v_137 + tint_f16_to_i32(mat2x3_f16[int(0)][0u]));
  int v_139 = (v_138 + tint_f16_to_i32(mat2x4_f16[int(0)][0u]));
  int v_140 = (v_139 + tint_f16_to_i32(mat3x2_f16[int(0)][0u]));
  int v_141 = (v_140 + tint_f16_to_i32(mat3x3_f16[int(0)][0u]));
  int v_142 = (v_141 + tint_f16_to_i32(mat3x4_f16[int(0)][0u]));
  int v_143 = (v_142 + tint_f16_to_i32(mat4x2_f16[int(0)][0u]));
  int v_144 = (v_143 + tint_f16_to_i32(mat4x3_f16[int(0)][0u]));
  int v_145 = (v_144 + tint_f16_to_i32(mat4x4_f16[int(0)][0u]));
  int v_146 = (v_145 + tint_f32_to_i32(arr2_vec3_f32[int(0)][0u]));
  s.Store(0u, asuint((v_146 + tint_f16_to_i32(arr2_mat4x2_f16[int(0)][int(0)][0u]))));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

