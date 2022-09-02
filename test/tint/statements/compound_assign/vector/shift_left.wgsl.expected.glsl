#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
layout(binding = 0, std430) buffer S_ssbo {
  ivec4 a;
} v;

void foo() {
  v.a = (v.a << uvec4(2u));
}

