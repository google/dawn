#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
ivec3 tint_ftoi(vec3 v) {
  return mix(ivec3(2147483647), mix(ivec3(v), ivec3((-2147483647 - 1)), lessThan(v, vec3(-2147483648.0f))), lessThanEqual(v, vec3(2147483520.0f)));
}

float t = 0.0f;
vec3 m() {
  t = 1.0f;
  return vec3(t);
}

void f() {
  vec3 tint_symbol = m();
  ivec3 v = tint_ftoi(tint_symbol);
}

