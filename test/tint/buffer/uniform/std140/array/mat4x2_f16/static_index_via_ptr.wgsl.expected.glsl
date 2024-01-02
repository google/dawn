#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat4x2_f16 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
};

layout(binding = 0, std140) uniform a_block_std140_ubo {
  mat4x2_f16 inner[4];
} a;

layout(binding = 1, std430) buffer s_block_ssbo {
  float16_t inner;
} s;

f16mat4x2 conv_mat4x2_f16(mat4x2_f16 val) {
  return f16mat4x2(val.col0, val.col1, val.col2, val.col3);
}

f16mat4x2[4] conv_arr4_mat4x2_f16(mat4x2_f16 val[4]) {
  f16mat4x2 arr[4] = f16mat4x2[4](f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat4x2(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat4x2_f16(val[i]);
    }
  }
  return arr;
}

void f() {
  f16mat4x2 p_a[4] = conv_arr4_mat4x2_f16(a.inner);
  f16mat4x2 p_a_2 = conv_mat4x2_f16(a.inner[2u]);
  f16vec2 p_a_2_1 = a.inner[2u].col1;
  f16mat4x2 l_a[4] = conv_arr4_mat4x2_f16(a.inner);
  f16mat4x2 l_a_i = conv_mat4x2_f16(a.inner[2u]);
  f16vec2 l_a_i_i = a.inner[2u].col1;
  s.inner = (((a.inner[2u].col1[0u] + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
