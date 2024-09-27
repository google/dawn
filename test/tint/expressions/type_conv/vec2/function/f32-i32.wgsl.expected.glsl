#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
ivec2 tint_ftoi(vec2 v) {
  return mix(ivec2(2147483647), mix(ivec2(v), ivec2((-2147483647 - 1)), lessThan(v, vec2(-2147483648.0f))), lessThanEqual(v, vec2(2147483520.0f)));
}

float t = 0.0f;
vec2 m() {
  t = 1.0f;
  return vec2(t);
}

void f() {
  vec2 tint_symbol = m();
  ivec2 v = tint_ftoi(tint_symbol);
}

