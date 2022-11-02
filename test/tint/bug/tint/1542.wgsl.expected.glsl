#version 310 es

struct UniformBuffer {
  ivec3 d;
  uint pad;
};

layout(binding = 0, std140) uniform u_input_block_ubo {
  UniformBuffer inner;
} u_input;

void tint_symbol() {
  ivec3 temp = (u_input.inner.d << uvec3(0u));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
