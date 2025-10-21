#version 310 es

layout(binding = 0, std140)
uniform ub_block_1_ubo {
  uvec4 inner[272];
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  int inner;
} v_1;
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
vec3[2] v_2(uint start_byte_offset) {
  vec3 a[2] = vec3[2](vec3(0.0f), vec3(0.0f));
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 2u)) {
        break;
      }
      a[v_4] = uintBitsToFloat(v.inner[((start_byte_offset + (v_4 * 16u)) / 16u)].xyz);
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  return a;
}
mat4 v_5(uint start_byte_offset) {
  return mat4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)]));
}
mat4x3 v_6(uint start_byte_offset) {
  return mat4x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((48u + start_byte_offset) / 16u)].xyz));
}
mat4x2 v_7(uint start_byte_offset) {
  uvec4 v_8 = v.inner[(start_byte_offset / 16u)];
  vec2 v_9 = uintBitsToFloat(mix(v_8.xy, v_8.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_10 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_11 = uintBitsToFloat(mix(v_10.xy, v_10.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_12 = v.inner[((16u + start_byte_offset) / 16u)];
  vec2 v_13 = uintBitsToFloat(mix(v_12.xy, v_12.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_14 = v.inner[((24u + start_byte_offset) / 16u)];
  return mat4x2(v_9, v_11, v_13, uintBitsToFloat(mix(v_14.xy, v_14.zw, bvec2(((((24u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat3x4 v_15(uint start_byte_offset) {
  return mat3x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)]));
}
mat3 v_16(uint start_byte_offset) {
  return mat3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz), uintBitsToFloat(v.inner[((32u + start_byte_offset) / 16u)].xyz));
}
mat3x2 v_17(uint start_byte_offset) {
  uvec4 v_18 = v.inner[(start_byte_offset / 16u)];
  vec2 v_19 = uintBitsToFloat(mix(v_18.xy, v_18.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_20 = v.inner[((8u + start_byte_offset) / 16u)];
  vec2 v_21 = uintBitsToFloat(mix(v_20.xy, v_20.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u))));
  uvec4 v_22 = v.inner[((16u + start_byte_offset) / 16u)];
  return mat3x2(v_19, v_21, uintBitsToFloat(mix(v_22.xy, v_22.zw, bvec2(((((16u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
mat2x4 v_23(uint start_byte_offset) {
  return mat2x4(uintBitsToFloat(v.inner[(start_byte_offset / 16u)]), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)]));
}
mat2x3 v_24(uint start_byte_offset) {
  return mat2x3(uintBitsToFloat(v.inner[(start_byte_offset / 16u)].xyz), uintBitsToFloat(v.inner[((16u + start_byte_offset) / 16u)].xyz));
}
mat2 v_25(uint start_byte_offset) {
  uvec4 v_26 = v.inner[(start_byte_offset / 16u)];
  vec2 v_27 = uintBitsToFloat(mix(v_26.xy, v_26.zw, bvec2((((start_byte_offset % 16u) / 4u) == 2u))));
  uvec4 v_28 = v.inner[((8u + start_byte_offset) / 16u)];
  return mat2(v_27, uintBitsToFloat(mix(v_28.xy, v_28.zw, bvec2(((((8u + start_byte_offset) % 16u) / 4u) == 2u)))));
}
void main_inner(uint idx) {
  uvec4 v_29 = v.inner[((544u * min(idx, 7u)) / 16u)];
  float scalar_f32 = uintBitsToFloat(v_29[(((544u * min(idx, 7u)) % 16u) / 4u)]);
  uvec4 v_30 = v.inner[((4u + (544u * min(idx, 7u))) / 16u)];
  int scalar_i32 = int(v_30[(((4u + (544u * min(idx, 7u))) % 16u) / 4u)]);
  uvec4 v_31 = v.inner[((8u + (544u * min(idx, 7u))) / 16u)];
  uint scalar_u32 = v_31[(((8u + (544u * min(idx, 7u))) % 16u) / 4u)];
  uvec4 v_32 = v.inner[((16u + (544u * min(idx, 7u))) / 16u)];
  vec2 vec2_f32 = uintBitsToFloat(mix(v_32.xy, v_32.zw, bvec2(((((16u + (544u * min(idx, 7u))) % 16u) / 4u) == 2u))));
  uvec4 v_33 = v.inner[((24u + (544u * min(idx, 7u))) / 16u)];
  ivec2 vec2_i32 = ivec2(mix(v_33.xy, v_33.zw, bvec2(((((24u + (544u * min(idx, 7u))) % 16u) / 4u) == 2u))));
  uvec4 v_34 = v.inner[((32u + (544u * min(idx, 7u))) / 16u)];
  uvec2 vec2_u32 = mix(v_34.xy, v_34.zw, bvec2(((((32u + (544u * min(idx, 7u))) % 16u) / 4u) == 2u)));
  vec3 vec3_f32 = uintBitsToFloat(v.inner[((48u + (544u * min(idx, 7u))) / 16u)].xyz);
  ivec3 vec3_i32 = ivec3(v.inner[((64u + (544u * min(idx, 7u))) / 16u)].xyz);
  uvec3 vec3_u32 = v.inner[((80u + (544u * min(idx, 7u))) / 16u)].xyz;
  vec4 vec4_f32 = uintBitsToFloat(v.inner[((96u + (544u * min(idx, 7u))) / 16u)]);
  ivec4 vec4_i32 = ivec4(v.inner[((112u + (544u * min(idx, 7u))) / 16u)]);
  uvec4 vec4_u32 = v.inner[((128u + (544u * min(idx, 7u))) / 16u)];
  mat2 mat2x2_f32 = v_25((144u + (544u * min(idx, 7u))));
  mat2x3 mat2x3_f32 = v_24((160u + (544u * min(idx, 7u))));
  mat2x4 mat2x4_f32 = v_23((192u + (544u * min(idx, 7u))));
  mat3x2 mat3x2_f32 = v_17((224u + (544u * min(idx, 7u))));
  mat3 mat3x3_f32 = v_16((256u + (544u * min(idx, 7u))));
  mat3x4 mat3x4_f32 = v_15((304u + (544u * min(idx, 7u))));
  mat4x2 mat4x2_f32 = v_7((352u + (544u * min(idx, 7u))));
  mat4x3 mat4x3_f32 = v_6((384u + (544u * min(idx, 7u))));
  mat4 mat4x4_f32 = v_5((448u + (544u * min(idx, 7u))));
  vec3 arr2_vec3_f32[2] = v_2((512u + (544u * min(idx, 7u))));
  uint v_35 = uint(tint_f32_to_i32(scalar_f32));
  int v_36 = int((v_35 + uint(scalar_i32)));
  int v_37 = int(scalar_u32);
  uint v_38 = uint(v_36);
  int v_39 = int((v_38 + uint(v_37)));
  int v_40 = tint_f32_to_i32(vec2_f32.x);
  uint v_41 = uint(v_39);
  uint v_42 = uint(int((v_41 + uint(v_40))));
  int v_43 = int((v_42 + uint(vec2_i32.x)));
  int v_44 = int(vec2_u32.x);
  uint v_45 = uint(v_43);
  int v_46 = int((v_45 + uint(v_44)));
  int v_47 = tint_f32_to_i32(vec3_f32.y);
  uint v_48 = uint(v_46);
  uint v_49 = uint(int((v_48 + uint(v_47))));
  int v_50 = int((v_49 + uint(vec3_i32.y)));
  int v_51 = int(vec3_u32.y);
  uint v_52 = uint(v_50);
  int v_53 = int((v_52 + uint(v_51)));
  int v_54 = tint_f32_to_i32(vec4_f32.z);
  uint v_55 = uint(v_53);
  uint v_56 = uint(int((v_55 + uint(v_54))));
  int v_57 = int((v_56 + uint(vec4_i32.z)));
  int v_58 = int(vec4_u32.z);
  uint v_59 = uint(v_57);
  int v_60 = int((v_59 + uint(v_58)));
  int v_61 = tint_f32_to_i32(mat2x2_f32[0u].x);
  uint v_62 = uint(v_60);
  int v_63 = int((v_62 + uint(v_61)));
  int v_64 = tint_f32_to_i32(mat2x3_f32[0u].x);
  uint v_65 = uint(v_63);
  int v_66 = int((v_65 + uint(v_64)));
  int v_67 = tint_f32_to_i32(mat2x4_f32[0u].x);
  uint v_68 = uint(v_66);
  int v_69 = int((v_68 + uint(v_67)));
  int v_70 = tint_f32_to_i32(mat3x2_f32[0u].x);
  uint v_71 = uint(v_69);
  int v_72 = int((v_71 + uint(v_70)));
  int v_73 = tint_f32_to_i32(mat3x3_f32[0u].x);
  uint v_74 = uint(v_72);
  int v_75 = int((v_74 + uint(v_73)));
  int v_76 = tint_f32_to_i32(mat3x4_f32[0u].x);
  uint v_77 = uint(v_75);
  int v_78 = int((v_77 + uint(v_76)));
  int v_79 = tint_f32_to_i32(mat4x2_f32[0u].x);
  uint v_80 = uint(v_78);
  int v_81 = int((v_80 + uint(v_79)));
  int v_82 = tint_f32_to_i32(mat4x3_f32[0u].x);
  uint v_83 = uint(v_81);
  int v_84 = int((v_83 + uint(v_82)));
  int v_85 = tint_f32_to_i32(mat4x4_f32[0u].x);
  uint v_86 = uint(v_84);
  int v_87 = int((v_86 + uint(v_85)));
  int v_88 = tint_f32_to_i32(arr2_vec3_f32[0u].x);
  uint v_89 = uint(v_87);
  v_1.inner = int((v_89 + uint(v_88)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  main_inner(gl_LocalInvocationIndex);
}
