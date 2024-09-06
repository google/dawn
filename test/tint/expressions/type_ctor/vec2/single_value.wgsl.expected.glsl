#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
vec2 v() {
  return vec2(0.0f);
}

void f() {
  vec2 a = vec2(1.0f);
  vec2 b = vec2(a);
  vec2 tint_symbol = v();
  vec2 c = vec2(tint_symbol);
  vec2 d = vec2((a * 2.0f));
}

