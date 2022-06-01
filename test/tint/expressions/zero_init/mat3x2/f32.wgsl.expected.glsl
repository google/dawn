#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  mat3x2 v = mat3x2(vec2(0.0f), vec2(0.0f), vec2(0.0f));
}

