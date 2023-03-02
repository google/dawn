#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int[128] tint_workgroupUniformLoad(inout int p[128]) {
  barrier();
  int result[128] = p;
  barrier();
  return result;
}

shared int v[128];
int foo() {
  int tint_symbol[128] = tint_workgroupUniformLoad(v);
  return tint_symbol[0];
}

