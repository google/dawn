#version 310 es

struct S {
  ivec4 a;
};

S v;
void foo() {
  v.a = (v.a + ivec4(2));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
