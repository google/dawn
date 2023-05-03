#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
shared int v[128];
int[128] tint_workgroupUniformLoad_v() {
  barrier();
  int result[128] = v;
  barrier();
  return result;
}

int foo() {
  int tint_symbol[128] = tint_workgroupUniformLoad_v();
  return tint_symbol[0];
}

