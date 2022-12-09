#version 310 es

void tint_symbol() {
  vec2 tint_symbol_2 = vec2(0.25f, 0.75f);
  vec2 whole = vec2(1.0f, 3.0f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
