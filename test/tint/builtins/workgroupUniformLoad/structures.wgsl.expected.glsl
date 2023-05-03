#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct Inner {
  bool b;
  ivec4 v;
  mat3 m;
};

struct Outer {
  Inner a[4];
};

shared Outer v;
Outer tint_workgroupUniformLoad_v() {
  barrier();
  Outer result = v;
  barrier();
  return result;
}

Outer foo() {
  return tint_workgroupUniformLoad_v();
}

