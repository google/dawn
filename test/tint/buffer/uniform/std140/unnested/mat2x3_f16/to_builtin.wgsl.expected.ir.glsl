#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uniform f16mat2x3 u;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat3x2 t = transpose(u);
  float16_t l = length(u[1]);
  float16_t a = abs(u[0].zxy[0u]);
}
