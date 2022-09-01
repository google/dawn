#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uint t = 0u;
uvec3 m() {
  t = 1u;
  return uvec3(t);
}

void f() {
  uvec3 tint_symbol = m();
  vec3 v = vec3(tint_symbol);
}

