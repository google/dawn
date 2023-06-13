#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void X(vec2 a, vec2 b) {
}

vec2 Y() {
  return vec2(0.0f);
}

void f() {
  vec2 v = vec2(0.0f, 0.0f);
  X(vec2(0.0f), v);
  vec2 tint_symbol = vec2(0.0f);
  vec2 tint_symbol_1 = Y();
  X(tint_symbol, tint_symbol_1);
}

