#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
float t = 0.0f;
vec4 m() {
  t = 1.0f;
  return vec4(t);
}

void f() {
  vec4 tint_symbol = m();
  bvec4 v = bvec4(tint_symbol);
}

