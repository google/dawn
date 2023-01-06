#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
shared int v[4];
int tint_workgroupUniformLoad_v_X(uint p[1]) {
  barrier();
  int result = v[p[0]];
  barrier();
  return result;
}

int foo_v_X(uint p[1]) {
  uint tint_symbol[1] = uint[1](p[0u]);
  return tint_workgroupUniformLoad_v_X(tint_symbol);
}

int bar() {
  uint tint_symbol_1[1] = uint[1](0u);
  return foo_v_X(tint_symbol_1);
}

