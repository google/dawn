#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct mat4x2_f16_std140 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
};

struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
  uint tint_pad_0;
};

struct S_std140 {
  float scalar_f32;
  int scalar_i32;
  uint scalar_u32;
  float16_t scalar_f16;
  vec2 vec2_f32;
  ivec2 vec2_i32;
  uvec2 vec2_u32;
  f16vec2 vec2_f16;
  uint tint_pad_0;
  vec3 vec3_f32;
  uint tint_pad_1;
  ivec3 vec3_i32;
  uint tint_pad_2;
  uvec3 vec3_u32;
  uint tint_pad_3;
  f16vec3 vec3_f16;
  uint tint_pad_4;
  uint tint_pad_5;
  vec4 vec4_f32;
  ivec4 vec4_i32;
  uvec4 vec4_u32;
  f16vec4 vec4_f16;
  vec2 mat2x2_f32_col0;
  vec2 mat2x2_f32_col1;
  uint tint_pad_6;
  uint tint_pad_7;
  vec3 mat2x3_f32_col0;
  uint tint_pad_8;
  vec3 mat2x3_f32_col1;
  uint tint_pad_9;
  mat2x4 mat2x4_f32;
  vec2 mat3x2_f32_col0;
  vec2 mat3x2_f32_col1;
  vec2 mat3x2_f32_col2;
  uint tint_pad_10;
  uint tint_pad_11;
  vec3 mat3x3_f32_col0;
  uint tint_pad_12;
  vec3 mat3x3_f32_col1;
  uint tint_pad_13;
  vec3 mat3x3_f32_col2;
  uint tint_pad_14;
  mat3x4 mat3x4_f32;
  vec2 mat4x2_f32_col0;
  vec2 mat4x2_f32_col1;
  vec2 mat4x2_f32_col2;
  vec2 mat4x2_f32_col3;
  vec3 mat4x3_f32_col0;
  uint tint_pad_15;
  vec3 mat4x3_f32_col1;
  uint tint_pad_16;
  vec3 mat4x3_f32_col2;
  uint tint_pad_17;
  vec3 mat4x3_f32_col3;
  uint tint_pad_18;
  mat4 mat4x4_f32;
  f16vec2 mat2x2_f16_col0;
  f16vec2 mat2x2_f16_col1;
  f16vec3 mat2x3_f16_col0;
  f16vec3 mat2x3_f16_col1;
  f16vec4 mat2x4_f16_col0;
  f16vec4 mat2x4_f16_col1;
  f16vec2 mat3x2_f16_col0;
  f16vec2 mat3x2_f16_col1;
  f16vec2 mat3x2_f16_col2;
  uint tint_pad_19;
  f16vec3 mat3x3_f16_col0;
  f16vec3 mat3x3_f16_col1;
  f16vec3 mat3x3_f16_col2;
  f16vec4 mat3x4_f16_col0;
  f16vec4 mat3x4_f16_col1;
  f16vec4 mat3x4_f16_col2;
  f16vec2 mat4x2_f16_col0;
  f16vec2 mat4x2_f16_col1;
  f16vec2 mat4x2_f16_col2;
  f16vec2 mat4x2_f16_col3;
  f16vec3 mat4x3_f16_col0;
  f16vec3 mat4x3_f16_col1;
  f16vec3 mat4x3_f16_col2;
  f16vec3 mat4x3_f16_col3;
  f16vec4 mat4x4_f16_col0;
  f16vec4 mat4x4_f16_col1;
  f16vec4 mat4x4_f16_col2;
  f16vec4 mat4x4_f16_col3;
  uint tint_pad_20;
  uint tint_pad_21;
  vec3 arr2_vec3_f32[2];
  mat4x2_f16_std140 arr2_mat4x2_f16[2];
  Inner struct_inner;
  Inner array_struct_inner[4];
};

layout(binding = 0, std140)
uniform ub_block_std140_1_ubo {
  S_std140 inner;
} v;
layout(binding = 1, std430)
buffer s_block_1_ssbo {
  int inner;
} v_1;
int tint_f16_to_i32(float16_t value) {
  return int(clamp(value, -65504.0hf, 65504.0hf));
}
int tint_f32_to_i32(float value) {
  return int(clamp(value, -2147483648.0f, 2147483520.0f));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float scalar_f32 = v.inner.scalar_f32;
  int scalar_i32 = v.inner.scalar_i32;
  uint scalar_u32 = v.inner.scalar_u32;
  float16_t scalar_f16 = v.inner.scalar_f16;
  vec2 vec2_f32 = v.inner.vec2_f32;
  ivec2 vec2_i32 = v.inner.vec2_i32;
  uvec2 vec2_u32 = v.inner.vec2_u32;
  f16vec2 vec2_f16 = v.inner.vec2_f16;
  vec3 vec3_f32 = v.inner.vec3_f32;
  ivec3 vec3_i32 = v.inner.vec3_i32;
  uvec3 vec3_u32 = v.inner.vec3_u32;
  f16vec3 vec3_f16 = v.inner.vec3_f16;
  vec4 vec4_f32 = v.inner.vec4_f32;
  ivec4 vec4_i32 = v.inner.vec4_i32;
  uvec4 vec4_u32 = v.inner.vec4_u32;
  f16vec4 vec4_f16 = v.inner.vec4_f16;
  mat2 mat2x2_f32 = mat2(v.inner.mat2x2_f32_col0, v.inner.mat2x2_f32_col1);
  mat2x3 mat2x3_f32 = mat2x3(v.inner.mat2x3_f32_col0, v.inner.mat2x3_f32_col1);
  mat2x4 mat2x4_f32 = v.inner.mat2x4_f32;
  mat3x2 mat3x2_f32 = mat3x2(v.inner.mat3x2_f32_col0, v.inner.mat3x2_f32_col1, v.inner.mat3x2_f32_col2);
  mat3 mat3x3_f32 = mat3(v.inner.mat3x3_f32_col0, v.inner.mat3x3_f32_col1, v.inner.mat3x3_f32_col2);
  mat3x4 mat3x4_f32 = v.inner.mat3x4_f32;
  mat4x2 mat4x2_f32 = mat4x2(v.inner.mat4x2_f32_col0, v.inner.mat4x2_f32_col1, v.inner.mat4x2_f32_col2, v.inner.mat4x2_f32_col3);
  mat4x3 mat4x3_f32 = mat4x3(v.inner.mat4x3_f32_col0, v.inner.mat4x3_f32_col1, v.inner.mat4x3_f32_col2, v.inner.mat4x3_f32_col3);
  mat4 mat4x4_f32 = v.inner.mat4x4_f32;
  f16mat2 mat2x2_f16 = f16mat2(v.inner.mat2x2_f16_col0, v.inner.mat2x2_f16_col1);
  f16mat2x3 mat2x3_f16 = f16mat2x3(v.inner.mat2x3_f16_col0, v.inner.mat2x3_f16_col1);
  f16mat2x4 mat2x4_f16 = f16mat2x4(v.inner.mat2x4_f16_col0, v.inner.mat2x4_f16_col1);
  f16mat3x2 mat3x2_f16 = f16mat3x2(v.inner.mat3x2_f16_col0, v.inner.mat3x2_f16_col1, v.inner.mat3x2_f16_col2);
  f16mat3 mat3x3_f16 = f16mat3(v.inner.mat3x3_f16_col0, v.inner.mat3x3_f16_col1, v.inner.mat3x3_f16_col2);
  f16mat3x4 mat3x4_f16 = f16mat3x4(v.inner.mat3x4_f16_col0, v.inner.mat3x4_f16_col1, v.inner.mat3x4_f16_col2);
  f16mat4x2 mat4x2_f16 = f16mat4x2(v.inner.mat4x2_f16_col0, v.inner.mat4x2_f16_col1, v.inner.mat4x2_f16_col2, v.inner.mat4x2_f16_col3);
  f16mat4x3 mat4x3_f16 = f16mat4x3(v.inner.mat4x3_f16_col0, v.inner.mat4x3_f16_col1, v.inner.mat4x3_f16_col2, v.inner.mat4x3_f16_col3);
  f16mat4 mat4x4_f16 = f16mat4(v.inner.mat4x4_f16_col0, v.inner.mat4x4_f16_col1, v.inner.mat4x4_f16_col2, v.inner.mat4x4_f16_col3);
  vec3 arr2_vec3_f32[2] = v.inner.arr2_vec3_f32;
  mat4x2_f16_std140 v_2[2] = v.inner.arr2_mat4x2_f16;
  f16mat4x2 v_3[2] = f16mat4x2[2](f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), f16mat4x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)));
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 2u)) {
        break;
      }
      v_3[v_5] = f16mat4x2(v_2[v_5].col0, v_2[v_5].col1, v_2[v_5].col2, v_2[v_5].col3);
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  f16mat4x2 arr2_mat4x2_f16[2] = v_3;
  Inner struct_inner = v.inner.struct_inner;
  Inner array_struct_inner[4] = v.inner.array_struct_inner;
  uint v_6 = uint(tint_f32_to_i32(scalar_f32));
  int v_7 = int((v_6 + uint(scalar_i32)));
  int v_8 = int(scalar_u32);
  uint v_9 = uint(v_7);
  int v_10 = int((v_9 + uint(v_8)));
  int v_11 = tint_f16_to_i32(scalar_f16);
  uint v_12 = uint(v_10);
  int v_13 = int((v_12 + uint(v_11)));
  int v_14 = tint_f32_to_i32(vec2_f32.x);
  uint v_15 = uint(v_13);
  uint v_16 = uint(int((v_15 + uint(v_14))));
  int v_17 = int((v_16 + uint(vec2_i32.x)));
  int v_18 = int(vec2_u32.x);
  uint v_19 = uint(v_17);
  int v_20 = int((v_19 + uint(v_18)));
  int v_21 = tint_f16_to_i32(vec2_f16.x);
  uint v_22 = uint(v_20);
  int v_23 = int((v_22 + uint(v_21)));
  int v_24 = tint_f32_to_i32(vec3_f32.y);
  uint v_25 = uint(v_23);
  uint v_26 = uint(int((v_25 + uint(v_24))));
  int v_27 = int((v_26 + uint(vec3_i32.y)));
  int v_28 = int(vec3_u32.y);
  uint v_29 = uint(v_27);
  int v_30 = int((v_29 + uint(v_28)));
  int v_31 = tint_f16_to_i32(vec3_f16.y);
  uint v_32 = uint(v_30);
  int v_33 = int((v_32 + uint(v_31)));
  int v_34 = tint_f32_to_i32(vec4_f32.z);
  uint v_35 = uint(v_33);
  uint v_36 = uint(int((v_35 + uint(v_34))));
  int v_37 = int((v_36 + uint(vec4_i32.z)));
  int v_38 = int(vec4_u32.z);
  uint v_39 = uint(v_37);
  int v_40 = int((v_39 + uint(v_38)));
  int v_41 = tint_f16_to_i32(vec4_f16.z);
  uint v_42 = uint(v_40);
  int v_43 = int((v_42 + uint(v_41)));
  int v_44 = tint_f32_to_i32(mat2x2_f32[0u].x);
  uint v_45 = uint(v_43);
  int v_46 = int((v_45 + uint(v_44)));
  int v_47 = tint_f32_to_i32(mat2x3_f32[0u].x);
  uint v_48 = uint(v_46);
  int v_49 = int((v_48 + uint(v_47)));
  int v_50 = tint_f32_to_i32(mat2x4_f32[0u].x);
  uint v_51 = uint(v_49);
  int v_52 = int((v_51 + uint(v_50)));
  int v_53 = tint_f32_to_i32(mat3x2_f32[0u].x);
  uint v_54 = uint(v_52);
  int v_55 = int((v_54 + uint(v_53)));
  int v_56 = tint_f32_to_i32(mat3x3_f32[0u].x);
  uint v_57 = uint(v_55);
  int v_58 = int((v_57 + uint(v_56)));
  int v_59 = tint_f32_to_i32(mat3x4_f32[0u].x);
  uint v_60 = uint(v_58);
  int v_61 = int((v_60 + uint(v_59)));
  int v_62 = tint_f32_to_i32(mat4x2_f32[0u].x);
  uint v_63 = uint(v_61);
  int v_64 = int((v_63 + uint(v_62)));
  int v_65 = tint_f32_to_i32(mat4x3_f32[0u].x);
  uint v_66 = uint(v_64);
  int v_67 = int((v_66 + uint(v_65)));
  int v_68 = tint_f32_to_i32(mat4x4_f32[0u].x);
  uint v_69 = uint(v_67);
  int v_70 = int((v_69 + uint(v_68)));
  int v_71 = tint_f16_to_i32(mat2x2_f16[0u].x);
  uint v_72 = uint(v_70);
  int v_73 = int((v_72 + uint(v_71)));
  int v_74 = tint_f16_to_i32(mat2x3_f16[0u].x);
  uint v_75 = uint(v_73);
  int v_76 = int((v_75 + uint(v_74)));
  int v_77 = tint_f16_to_i32(mat2x4_f16[0u].x);
  uint v_78 = uint(v_76);
  int v_79 = int((v_78 + uint(v_77)));
  int v_80 = tint_f16_to_i32(mat3x2_f16[0u].x);
  uint v_81 = uint(v_79);
  int v_82 = int((v_81 + uint(v_80)));
  int v_83 = tint_f16_to_i32(mat3x3_f16[0u].x);
  uint v_84 = uint(v_82);
  int v_85 = int((v_84 + uint(v_83)));
  int v_86 = tint_f16_to_i32(mat3x4_f16[0u].x);
  uint v_87 = uint(v_85);
  int v_88 = int((v_87 + uint(v_86)));
  int v_89 = tint_f16_to_i32(mat4x2_f16[0u].x);
  uint v_90 = uint(v_88);
  int v_91 = int((v_90 + uint(v_89)));
  int v_92 = tint_f16_to_i32(mat4x3_f16[0u].x);
  uint v_93 = uint(v_91);
  int v_94 = int((v_93 + uint(v_92)));
  int v_95 = tint_f16_to_i32(mat4x4_f16[0u].x);
  uint v_96 = uint(v_94);
  int v_97 = int((v_96 + uint(v_95)));
  int v_98 = tint_f32_to_i32(arr2_vec3_f32[0u].x);
  uint v_99 = uint(v_97);
  int v_100 = int((v_99 + uint(v_98)));
  int v_101 = tint_f16_to_i32(arr2_mat4x2_f16[0u][0u].x);
  uint v_102 = uint(v_100);
  uint v_103 = uint(int((v_102 + uint(v_101))));
  uint v_104 = uint(int((v_103 + uint(struct_inner.scalar_i32))));
  v_1.inner = int((v_104 + uint(array_struct_inner[0u].scalar_i32)));
}
