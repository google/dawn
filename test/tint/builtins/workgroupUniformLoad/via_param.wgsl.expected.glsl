#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int tint_workgroupUniformLoad(inout int p) {
  barrier();
  int result = p;
  barrier();
  return result;
}

shared int v[4];
int foo_v_X(uint p[1]) {
  return tint_workgroupUniformLoad(v[p[0]]);
}

int bar() {
  uint tint_symbol[1] = uint[1](0u);
  return foo_v_X(tint_symbol);
}

