#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
vec2 f = vec2(0.0f);
ivec2 i = ivec2(0);
uvec2 u = uvec2(0u);
