#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
uvec3 u = uvec3(1u);
void f() {
  vec3 v = vec3(u);
}

