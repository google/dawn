#version 310 es

layout(binding = 0, std140) uniform i_block_ubo {
  uint inner;
} i;

layout(binding = 1, std430) buffer v1_block_ssbo {
  vec3 inner;
} v1;

void tint_symbol() {
  v1.inner[i.inner] = 1.0f;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
