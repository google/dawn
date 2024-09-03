#version 310 es

struct S {
  int before;
  mat2x4 m;
  int after;
};

uniform S u[4];
S p[4] = S[4](S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0), S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0), S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0), S(0, mat2x4(vec4(0.0f), vec4(0.0f)), 0));
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  p = u;
  p[1] = u[2];
  p[3].m = u[2].m;
  p[1].m[0] = u[0].m[1].ywxz;
}
