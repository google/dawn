#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uniform f16mat2x4 u;
f16mat2x4 p = f16mat2x4(f16vec4(0.0hf), f16vec4(0.0hf));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = u;
  p[1] = u[0];
  p[1] = u[0].ywxz;
  p[0][1] = u[1].x;
}
