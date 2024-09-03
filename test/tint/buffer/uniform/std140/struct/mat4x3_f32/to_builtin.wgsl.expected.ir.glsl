#version 310 es

struct S {
  int before;
  mat4x3 m;
  int after;
};

uniform S u[4];
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  mat3x4 t = transpose(u[2].m);
  float l = length(u[0].m[1].zxy);
  float a = abs(u[0].m[1].zxy[0u]);
}
