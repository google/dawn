#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
shared int v[4];
int[4] tint_workgroupUniformLoad_v() {
  barrier();
  int result[4] = v;
  barrier();
  return result;
}

int[4] foo() {
  return tint_workgroupUniformLoad_v();
}

