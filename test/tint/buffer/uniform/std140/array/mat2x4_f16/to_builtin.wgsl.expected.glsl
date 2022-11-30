#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat2x4_f16 {
  f16vec4 col0;
  f16vec4 col1;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat2x4_f16 inner[4];
} u;

f16mat2x4 conv_mat2x4_f16(mat2x4_f16 val) {
  return f16mat2x4(val.col0, val.col1);
}

void f() {
  f16mat4x2 t = transpose(conv_mat2x4_f16(u.inner[2u]));
  float16_t l = length(u.inner[0u].col1.ywxz);
  float16_t a = abs(u.inner[0u].col1.ywxz[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
