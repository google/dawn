#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
const mat4x3 m = mat4x3(vec3(0.0f, 1.0f, 2.0f), vec3(3.0f, 4.0f, 5.0f), vec3(6.0f, 7.0f, 8.0f), vec3(9.0f, 10.0f, 11.0f));
