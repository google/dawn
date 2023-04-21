#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
vec4 f = vec4(0.0f);
ivec4 i = ivec4(0);
uvec4 u = uvec4(0u);
