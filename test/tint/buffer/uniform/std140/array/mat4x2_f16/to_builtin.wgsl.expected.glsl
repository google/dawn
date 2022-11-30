#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat4x2_f16 {
  f16vec2 col0;
  f16vec2 col1;
  f16vec2 col2;
  f16vec2 col3;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat4x2_f16 inner[4];
} u;

f16mat4x2 conv_mat4x2_f16(mat4x2_f16 val) {
  return f16mat4x2(val.col0, val.col1, val.col2, val.col3);
}

void f() {
  f16mat2x4 t = transpose(conv_mat4x2_f16(u.inner[2u]));
  float16_t l = length(u.inner[0u].col1.yx);
  float16_t a = abs(u.inner[0u].col1.yx[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
