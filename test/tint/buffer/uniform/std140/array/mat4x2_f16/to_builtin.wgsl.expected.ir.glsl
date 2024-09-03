#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uniform f16mat4x2 u[4];
float16_t s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2x4 t = transpose(u[2]);
  float16_t l = length(u[0][1].yx);
  float16_t a = abs(u[0][1].yx[0u]);
  float16_t v = (t[0][0u] + float16_t(l));
  s = (v + float16_t(a));
}
