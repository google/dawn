#version 310 es
#extension GL_AMD_gpu_shader_half_float: require

struct S {
  f16vec3 v;
};

S U;
void f() {
  U.v = f16vec3(1.0hf, 2.0hf, 3.0hf);
  U.v[0u] = 1.0hf;
  U.v[1u] = 2.0hf;
  U.v[2u] = 3.0hf;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
