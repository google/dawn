#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int[4] tint_workgroupUniformLoad(inout int p[4]) {
  barrier();
  int result[4] = p;
  barrier();
  return result;
}

shared int v[4];
int[4] foo() {
  return tint_workgroupUniformLoad(v);
}

