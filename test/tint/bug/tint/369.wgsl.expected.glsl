#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
struct S {
  mat2 m;
};

layout(binding = 0, std430) buffer S_1 {
  mat2 m;
} SSBO;
layout(binding = 0) uniform S_2 {
  mat2 m;
} UBO;

