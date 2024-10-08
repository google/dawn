#version 310 es

layout(binding = 0, std140) uniform i_block_ubo {
  uint inner;
} i;

vec3 v1 = vec3(0.0f, 0.0f, 0.0f);
void tint_symbol() {
  v1[i.inner] = 1.0f;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
