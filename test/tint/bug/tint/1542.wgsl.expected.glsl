#version 310 es

layout(binding = 0, std140) uniform UniformBuffer_ubo {
  ivec3 d;
  uint pad;
} u_input;

void tint_symbol() {
  ivec3 temp = (u_input.d << uvec3(0u));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
