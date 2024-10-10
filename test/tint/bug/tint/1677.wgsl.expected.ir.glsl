#version 310 es


struct Input {
  ivec3 position;
  uint tint_pad_0;
};

layout(binding = 0, std430)
buffer tint_symbol_block_1_ssbo {
  Input inner;
} v;
void tint_symbol_1_inner(uvec3 id) {
  ivec3 pos = (v.inner.position - ivec3(0));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1_inner(gl_GlobalInvocationID);
}
