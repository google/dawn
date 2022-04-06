#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
bool get_bool() {
  return true;
}

void f() {
  bool tint_symbol = get_bool();
  bvec2 v2 = bvec2(tint_symbol);
  bool tint_symbol_1 = get_bool();
  bvec3 v3 = bvec3(tint_symbol_1);
  bool tint_symbol_2 = get_bool();
  bvec4 v4 = bvec4(tint_symbol_2);
}

