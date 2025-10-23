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
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

Inner v_2(uint start_byte_offset) {
  int v_3 = asint(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float v_4 = asfloat(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]);
  uint v_5 = ub[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) & 15u) >> 2u)];
  uint v_6 = (((((8u + start_byte_offset) % 4u) == 0u)) ? (0u) : (1u));
  Inner v_7 = {v_3, v_4, tint_bitcast_to_f16(v_5)[v_6]};
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
      continue;
    }
  }
  Inner v_12[4] = a;
  return v_12;
}

matrix<float16_t, 4, 2> v_13(uint start_byte_offset) {
  vector<float16_t, 2> v_14 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  vector<float16_t, 2> v_15 = tint_bitcast_to_f16(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]);
  vector<float16_t, 2> v_16 = tint_bitcast_to_f16(ub[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) & 15u) >> 2u)]);
  return matrix<float16_t, 4, 2>(v_14, v_15, v_16, tint_bitcast_to_f16(ub[((12u + start_byte_offset) / 16u)][(((12u + start_byte_offset) & 15u) >> 2u)]));
}

typedef matrix<float16_t, 4, 2> ary_ret_1[2];
ary_ret_1 v_17(uint start_byte_offset) {
  matrix<float16_t, 4, 2> a[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    uint v_18 = 0u;
    v_18 = 0u;
    while(true) {
      uint v_19 = v_18;
      if ((v_19 >= 2u)) {
        break;
      }
      a[v_19] = v_13((start_byte_offset + (v_19 * 16u)));
      {
        v_18 = (v_19 + 1u);
      }
      continue;
    }
  }
  matrix<float16_t, 4, 2> v_20[2] = a;
  return v_20;
}

typedef float3 ary_ret_2[2];
ary_ret_2 v_21(uint start_byte_offset) {
  float3 a[2] = (float3[2])0;
  {
    uint v_22 = 0u;
    v_22 = 0u;
    while(true) {
      uint v_23 = v_22;
      if ((v_23 >= 2u)) {
        break;
      }
      a[v_23] = asfloat(ub[((start_byte_offset + (v_23 * 16u)) / 16u)].xyz);
      {
        v_22 = (v_23 + 1u);
      }
      continue;
    }
  }
  float3 v_24[2] = a;
  return v_24;
}

vector<float16_t, 4> tint_bitcast_to_f16_1(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_25 = float16_t(t_low.x);
  float16_t v_26 = float16_t(t_high.x);
  float16_t v_27 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_25, v_26, v_27, float16_t(t_high.y));
}

matrix<float16_t, 4, 4> v_28(uint start_byte_offset) {
  uint4 v_29 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_30 = tint_bitcast_to_f16_1((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_29.zw) : (v_29.xy)));
  uint4 v_31 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_32 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_31.zw) : (v_31.xy)));
  uint4 v_33 = ub[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_34 = tint_bitcast_to_f16_1(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_33.zw) : (v_33.xy)));
  uint4 v_35 = ub[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 4>(v_30, v_32, v_34, tint_bitcast_to_f16_1(((((((24u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_35.zw) : (v_35.xy))));
}

matrix<float16_t, 4, 3> v_36(uint start_byte_offset) {
  uint4 v_37 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_38 = tint_bitcast_to_f16_1((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_37.zw) : (v_37.xy))).xyz;
  uint4 v_39 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_40 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_39.zw) : (v_39.xy))).xyz;
  uint4 v_41 = ub[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_42 = tint_bitcast_to_f16_1(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_41.zw) : (v_41.xy))).xyz;
  uint4 v_43 = ub[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 3>(v_38, v_40, v_42, tint_bitcast_to_f16_1(((((((24u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_43.zw) : (v_43.xy))).xyz);
}

matrix<float16_t, 3, 4> v_44(uint start_byte_offset) {
  uint4 v_45 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_46 = tint_bitcast_to_f16_1((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_45.zw) : (v_45.xy)));
  uint4 v_47 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 4> v_48 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_47.zw) : (v_47.xy)));
  uint4 v_49 = ub[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 4>(v_46, v_48, tint_bitcast_to_f16_1(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_49.zw) : (v_49.xy))));
}

matrix<float16_t, 3, 3> v_50(uint start_byte_offset) {
  uint4 v_51 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_52 = tint_bitcast_to_f16_1((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_51.zw) : (v_51.xy))).xyz;
  uint4 v_53 = ub[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_54 = tint_bitcast_to_f16_1(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_53.zw) : (v_53.xy))).xyz;
  uint4 v_55 = ub[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 3>(v_52, v_54, tint_bitcast_to_f16_1(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_55.zw) : (v_55.xy))).xyz);
}

matrix<float16_t, 3, 2> v_56(uint start_byte_offset) {
  vector<float16_t, 2> v_57 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  vector<float16_t, 2> v_58 = tint_bitcast_to_f16(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]);
  return matrix<float16_t, 3, 2>(v_57, v_58, tint_bitcast_to_f16(ub[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) & 15u) >> 2u)]));
}

matrix<float16_t, 2, 4> v_59(uint start_byte_offset) {
  uint4 v_60 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 4> v_61 = tint_bitcast_to_f16_1((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_60.zw) : (v_60.xy)));
  uint4 v_62 = ub[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 4>(v_61, tint_bitcast_to_f16_1(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_62.zw) : (v_62.xy))));
}

matrix<float16_t, 2, 3> v_63(uint start_byte_offset) {
  uint4 v_64 = ub[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_65 = tint_bitcast_to_f16_1((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_64.zw) : (v_64.xy))).xyz;
  uint4 v_66 = ub[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_65, tint_bitcast_to_f16_1(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_66.zw) : (v_66.xy))).xyz);
}

matrix<float16_t, 2, 2> v_67(uint start_byte_offset) {
  vector<float16_t, 2> v_68 = tint_bitcast_to_f16(ub[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  return matrix<float16_t, 2, 2>(v_68, tint_bitcast_to_f16(ub[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) & 15u) >> 2u)]));
}

float4x4 v_69(uint start_byte_offset) {
  return float4x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]), asfloat(ub[((48u + start_byte_offset) / 16u)]));
}

float4x3 v_70(uint start_byte_offset) {
  return float4x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz), asfloat(ub[((48u + start_byte_offset) / 16u)].xyz));
}

float4x2 v_71(uint start_byte_offset) {
  uint4 v_72 = ub[(start_byte_offset / 16u)];
  float2 v_73 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_72.zw) : (v_72.xy)));
  uint4 v_74 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_75 = asfloat(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_74.zw) : (v_74.xy)));
  uint4 v_76 = ub[((16u + start_byte_offset) / 16u)];
  float2 v_77 = asfloat(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_76.zw) : (v_76.xy)));
  uint4 v_78 = ub[((24u + start_byte_offset) / 16u)];
  return float4x2(v_73, v_75, v_77, asfloat(((((((24u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_78.zw) : (v_78.xy))));
}

float3x4 v_79(uint start_byte_offset) {
  return float3x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]), asfloat(ub[((32u + start_byte_offset) / 16u)]));
}

float3x3 v_80(uint start_byte_offset) {
  return float3x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz), asfloat(ub[((32u + start_byte_offset) / 16u)].xyz));
}

float3x2 v_81(uint start_byte_offset) {
  uint4 v_82 = ub[(start_byte_offset / 16u)];
  float2 v_83 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_82.zw) : (v_82.xy)));
  uint4 v_84 = ub[((8u + start_byte_offset) / 16u)];
  float2 v_85 = asfloat(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_84.zw) : (v_84.xy)));
  uint4 v_86 = ub[((16u + start_byte_offset) / 16u)];
  return float3x2(v_83, v_85, asfloat(((((((16u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_86.zw) : (v_86.xy))));
}

float2x4 v_87(uint start_byte_offset) {
  return float2x4(asfloat(ub[(start_byte_offset / 16u)]), asfloat(ub[((16u + start_byte_offset) / 16u)]));
}

float2x3 v_88(uint start_byte_offset) {
  return float2x3(asfloat(ub[(start_byte_offset / 16u)].xyz), asfloat(ub[((16u + start_byte_offset) / 16u)].xyz));
}

float2x2 v_89(uint start_byte_offset) {
  uint4 v_90 = ub[(start_byte_offset / 16u)];
  float2 v_91 = asfloat((((((start_byte_offset & 15u) >> 2u) == 2u)) ? (v_90.zw) : (v_90.xy)));
  uint4 v_92 = ub[((8u + start_byte_offset) / 16u)];
  return float2x2(v_91, asfloat(((((((8u + start_byte_offset) & 15u) >> 2u) == 2u)) ? (v_92.zw) : (v_92.xy))));
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
  float2x2 mat2x2_f32 = v_89(168u);
  float2x3 mat2x3_f32 = v_88(192u);
  float2x4 mat2x4_f32 = v_87(224u);
  float3x2 mat3x2_f32 = v_81(256u);
  float3x3 mat3x3_f32 = v_80(288u);
  float3x4 mat3x4_f32 = v_79(336u);
  float4x2 mat4x2_f32 = v_71(384u);
  float4x3 mat4x3_f32 = v_70(416u);
  float4x4 mat4x4_f32 = v_69(480u);
  matrix<float16_t, 2, 2> mat2x2_f16 = v_67(544u);
  matrix<float16_t, 2, 3> mat2x3_f16 = v_63(552u);
  matrix<float16_t, 2, 4> mat2x4_f16 = v_59(568u);
  matrix<float16_t, 3, 2> mat3x2_f16 = v_56(584u);
  matrix<float16_t, 3, 3> mat3x3_f16 = v_50(600u);
  matrix<float16_t, 3, 4> mat3x4_f16 = v_44(624u);
  matrix<float16_t, 4, 2> mat4x2_f16 = v_13(648u);
  matrix<float16_t, 4, 3> mat4x3_f16 = v_36(664u);
  matrix<float16_t, 4, 4> mat4x4_f16 = v_28(696u);
  float3 arr2_vec3_f32[2] = v_21(736u);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = v_17(768u);
  Inner struct_inner = v_2(800u);
  Inner array_struct_inner[4] = v_8(816u);
  int v_93 = asint((asuint(tint_f32_to_i32(scalar_f32)) + asuint(scalar_i32)));
  int v_94 = asint((asuint(v_93) + asuint(int(scalar_u32))));
  int v_95 = asint((asuint(v_94) + asuint(tint_f16_to_i32(scalar_f16))));
  int v_96 = asint((asuint(asint((asuint(v_95) + asuint(tint_f32_to_i32(vec2_f32.x))))) + asuint(vec2_i32.x)));
  int v_97 = asint((asuint(v_96) + asuint(int(vec2_u32.x))));
  int v_98 = asint((asuint(v_97) + asuint(tint_f16_to_i32(vec2_f16.x))));
  int v_99 = asint((asuint(asint((asuint(v_98) + asuint(tint_f32_to_i32(vec3_f32.y))))) + asuint(vec3_i32.y)));
  int v_100 = asint((asuint(v_99) + asuint(int(vec3_u32.y))));
  int v_101 = asint((asuint(v_100) + asuint(tint_f16_to_i32(vec3_f16.y))));
  int v_102 = asint((asuint(asint((asuint(v_101) + asuint(tint_f32_to_i32(vec4_f32.z))))) + asuint(vec4_i32.z)));
  int v_103 = asint((asuint(v_102) + asuint(int(vec4_u32.z))));
  int v_104 = asint((asuint(v_103) + asuint(tint_f16_to_i32(vec4_f16.z))));
  int v_105 = asint((asuint(v_104) + asuint(tint_f32_to_i32(mat2x2_f32[0u].x))));
  int v_106 = asint((asuint(v_105) + asuint(tint_f32_to_i32(mat2x3_f32[0u].x))));
  int v_107 = asint((asuint(v_106) + asuint(tint_f32_to_i32(mat2x4_f32[0u].x))));
  int v_108 = asint((asuint(v_107) + asuint(tint_f32_to_i32(mat3x2_f32[0u].x))));
  int v_109 = asint((asuint(v_108) + asuint(tint_f32_to_i32(mat3x3_f32[0u].x))));
  int v_110 = asint((asuint(v_109) + asuint(tint_f32_to_i32(mat3x4_f32[0u].x))));
  int v_111 = asint((asuint(v_110) + asuint(tint_f32_to_i32(mat4x2_f32[0u].x))));
  int v_112 = asint((asuint(v_111) + asuint(tint_f32_to_i32(mat4x3_f32[0u].x))));
  int v_113 = asint((asuint(v_112) + asuint(tint_f32_to_i32(mat4x4_f32[0u].x))));
  int v_114 = asint((asuint(v_113) + asuint(tint_f16_to_i32(mat2x2_f16[0u].x))));
  int v_115 = asint((asuint(v_114) + asuint(tint_f16_to_i32(mat2x3_f16[0u].x))));
  int v_116 = asint((asuint(v_115) + asuint(tint_f16_to_i32(mat2x4_f16[0u].x))));
  int v_117 = asint((asuint(v_116) + asuint(tint_f16_to_i32(mat3x2_f16[0u].x))));
  int v_118 = asint((asuint(v_117) + asuint(tint_f16_to_i32(mat3x3_f16[0u].x))));
  int v_119 = asint((asuint(v_118) + asuint(tint_f16_to_i32(mat3x4_f16[0u].x))));
  int v_120 = asint((asuint(v_119) + asuint(tint_f16_to_i32(mat4x2_f16[0u].x))));
  int v_121 = asint((asuint(v_120) + asuint(tint_f16_to_i32(mat4x3_f16[0u].x))));
  int v_122 = asint((asuint(v_121) + asuint(tint_f16_to_i32(mat4x4_f16[0u].x))));
  int v_123 = asint((asuint(v_122) + asuint(tint_f32_to_i32(arr2_vec3_f32[0u].x))));
  s.Store(0u, asuint(asint((asuint(asint((asuint(asint((asuint(v_123) + asuint(tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x))))) + asuint(struct_inner.scalar_i32)))) + asuint(array_struct_inner[0u].scalar_i32)))));
}

