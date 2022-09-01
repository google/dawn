#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
bool t = false;
bvec3 m() {
  t = true;
  return bvec3(t);
}

void f() {
  bvec3 tint_symbol = m();
  vec3 v = vec3(tint_symbol);
}

