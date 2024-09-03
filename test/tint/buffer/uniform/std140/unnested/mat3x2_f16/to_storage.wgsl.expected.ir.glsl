#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uniform f16mat3x2 u;
f16mat3x2 s;
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  s = u;
  s[1] = u[0];
  s[1] = u[0].yx;
  s[0][1] = u[1].x;
}
