#version 310 es

struct S {
  uvec3 v;
};

S U;
void f() {
  U.v = uvec3(1u, 2u, 3u);
  U.v[0u] = 1u;
  U.v[1u] = 2u;
  U.v[2u] = 3u;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
