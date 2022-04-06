#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  ivec3 v;
};

S P = S(ivec3(0, 0, 0));
void f() {
  P.v = ivec3(1, 2, 3);
  P.v.x = 1;
  P.v.y = 2;
  P.v.z = 3;
}

