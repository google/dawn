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

Outer tint_workgroupUniformLoad(inout Outer p) {
  barrier();
  Outer result = p;
  barrier();
  return result;
}

shared Outer v;
Outer foo() {
  return tint_workgroupUniformLoad(v);
}

