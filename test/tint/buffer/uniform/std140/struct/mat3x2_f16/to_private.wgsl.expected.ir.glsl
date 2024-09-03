#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct S {
  int before;
  f16mat3x2 m;
  int after;
};

uniform S u[4];
S p[4] = S[4](S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0), S(0, f16mat3x2(f16vec2(0.0hf), f16vec2(0.0hf), f16vec2(0.0hf)), 0));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = u;
  p[1] = u[2];
  p[3].m = u[2].m;
  p[1].m[0] = u[0].m[1].yx;
}
