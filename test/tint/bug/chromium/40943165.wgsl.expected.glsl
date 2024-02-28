#version 310 es

shared mat2 W;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    W = mat2(vec2(0.0f), vec2(0.0f));
  }
  barrier();
}

void F(uint tint_symbol) {
  tint_zero_workgroup_memory(tint_symbol);
  W[0] = (W[0] + 0.0f);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  F(gl_LocalInvocationIndex);
  return;
}
