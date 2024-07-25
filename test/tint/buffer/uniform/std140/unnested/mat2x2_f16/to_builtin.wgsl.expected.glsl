#version 310 es
#extension GL_AMD_gpu_shader_half_float : require

layout(binding = 0, std140) uniform u_block_std140_ubo {
  f16vec2 inner_0;
  f16vec2 inner_1;
} u;

f16mat2 load_u_inner() {
  return f16mat2(u.inner_0, u.inner_1);
}

void f() {
  f16mat2 t = transpose(load_u_inner());
  float16_t l = length(u.inner_1);
  float16_t a = abs(u.inner_0.yx[0u]);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f();
  return;
}
