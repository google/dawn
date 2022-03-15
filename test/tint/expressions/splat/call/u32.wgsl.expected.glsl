#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uint get_u32() {
  return 1u;
}

void f() {
  uint tint_symbol = get_u32();
  uvec2 v2 = uvec2(tint_symbol);
  uint tint_symbol_1 = get_u32();
  uvec3 v3 = uvec3(tint_symbol_1);
  uint tint_symbol_2 = get_u32();
  uvec4 v4 = uvec4(tint_symbol_2);
}

