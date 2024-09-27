#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uvec2 tint_ftou(vec2 v) {
  return mix(uvec2(4294967295u), mix(uvec2(v), uvec2(0u), lessThan(v, vec2(0.0f))), lessThanEqual(v, vec2(4294967040.0f)));
}

float t = 0.0f;
vec2 m() {
  t = 1.0f;
  return vec2(t);
}

void f() {
  vec2 tint_symbol = m();
  uvec2 v = tint_ftou(tint_symbol);
}

