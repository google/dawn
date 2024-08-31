#version 310 es

struct S {
  ivec4 a;
};

S v;
void foo() {
  v.a = (v.a << (uvec4(2u) & uvec4(31u)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
