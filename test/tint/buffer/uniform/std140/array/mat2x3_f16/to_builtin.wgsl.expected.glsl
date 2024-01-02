#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

struct mat2x3_f16 {
  f16vec3 col0;
  f16vec3 col1;
};

layout(binding = 0, std140) uniform u_block_std140_ubo {
  mat2x3_f16 inner[4];
} u;

layout(binding = 1, std430) buffer s_block_ssbo {
  float16_t inner;
} s;

f16mat2x3 conv_mat2x3_f16(mat2x3_f16 val) {
  return f16mat2x3(val.col0, val.col1);
}

void f() {
  f16mat3x2 t = transpose(conv_mat2x3_f16(u.inner[2u]));
  float16_t l = length(u.inner[0u].col1.zxy);
  float16_t a = abs(u.inner[0u].col1.zxy[0u]);
  s.inner = ((float16_t(a) + float16_t(l)) + t[0].x);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
