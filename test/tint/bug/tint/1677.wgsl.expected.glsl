#version 310 es

layout(binding = 0, std430) buffer Input_ssbo {
  ivec3 position;
  uint pad;
} tint_symbol;

void tint_symbol_1(uvec3 id) {
  ivec3 pos = (tint_symbol.position - ivec3(0));
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_1(gl_GlobalInvocationID);
  return;
}
