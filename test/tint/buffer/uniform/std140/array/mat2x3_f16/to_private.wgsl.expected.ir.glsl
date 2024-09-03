#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

uniform f16mat2x3 u[4];
float16_t s;
f16mat2x3 p[4] = f16mat2x3[4](f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)), f16mat2x3(f16vec3(0.0hf), f16vec3(0.0hf)));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = u;
  p[1] = u[2];
  p[1][0] = u[0][1].zxy;
  p[1][0][0u] = u[0][1].x;
  s = p[1][0].x;
}
