#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
const mat4x2 m = mat4x2(vec2(0.0f, 1.0f), vec2(2.0f, 3.0f), vec2(4.0f, 5.0f), vec2(6.0f, 7.0f));
