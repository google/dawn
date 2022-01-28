#version 310 es
precision mediump float;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uint get_u32() {
  return 1u;
}

void f() {
  uvec2 v2 = uvec2(get_u32());
  uvec3 v3 = uvec3(get_u32());
  uvec4 v4 = uvec4(get_u32());
}

