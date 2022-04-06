#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
int get_i32() {
  return 1;
}

void f() {
  int tint_symbol = get_i32();
  ivec2 v2 = ivec2(tint_symbol);
  int tint_symbol_1 = get_i32();
  ivec3 v3 = ivec3(tint_symbol_1);
  int tint_symbol_2 = get_i32();
  ivec4 v4 = ivec4(tint_symbol_2);
}

