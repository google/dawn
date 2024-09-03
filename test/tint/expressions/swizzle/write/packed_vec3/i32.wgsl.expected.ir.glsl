#version 310 es

struct S {
  ivec3 v;
};

S U;
void f() {
  U.v = ivec3(1, 2, 3);
  U.v[0u] = 1;
  U.v[1u] = 2;
  U.v[2u] = 3;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
