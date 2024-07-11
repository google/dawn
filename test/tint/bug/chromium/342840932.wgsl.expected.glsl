#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
layout(binding = 0, r32ui) uniform highp readonly uimage2D image_dup_src;
layout(binding = 1, r32ui) uniform highp writeonly uimage2D image_dst;
