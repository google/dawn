#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uniform f16mat4 u[4];
f16mat4 s[4];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  s = u;
  s[1] = u[2];
  s[1][0] = u[0][1].ywxz;
  s[1][0][0u] = u[0][1].x;
}
