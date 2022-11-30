#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat2x4_f16 {
  f16vec4 col0;
  f16vec4 col1;
};

layout(binding = 0, std140) uniform a_block_std140_ubo {
  mat2x4_f16 inner[4];
} a;

f16mat2x4 conv_mat2x4_f16(mat2x4_f16 val) {
  return f16mat2x4(val.col0, val.col1);
}

f16mat2x4[4] conv_arr4_mat2x4_f16(mat2x4_f16 val[4]) {
  f16mat2x4 arr[4] = f16mat2x4[4](f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf), f16mat2x4(0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf, 0.0hf));
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = conv_mat2x4_f16(val[i]);
    }
  }
  return arr;
}

void f() {
  f16mat2x4 p_a[4] = conv_arr4_mat2x4_f16(a.inner);
  f16mat2x4 p_a_2 = conv_mat2x4_f16(a.inner[2u]);
  f16vec4 p_a_2_1 = a.inner[2u].col1;
  f16mat2x4 l_a[4] = conv_arr4_mat2x4_f16(a.inner);
  f16mat2x4 l_a_i = conv_mat2x4_f16(a.inner[2u]);
  f16vec4 l_a_i_i = a.inner[2u].col1;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
