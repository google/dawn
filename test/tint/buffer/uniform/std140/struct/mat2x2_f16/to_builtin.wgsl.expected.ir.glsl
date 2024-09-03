#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct S {
  int before;
  f16mat2 m;
  int after;
};

uniform S u[4];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  f16mat2 t = transpose(u[2].m);
  float16_t l = length(u[0].m[1].yx);
  float16_t a = abs(u[0].m[1].yx[0u]);
}
