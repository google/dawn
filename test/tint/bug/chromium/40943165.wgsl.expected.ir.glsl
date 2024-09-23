#version 310 es

shared mat2 W;
void F_inner(uint tint_symbol) {
  if ((tint_symbol == 0u)) {
    W = mat2(vec2(0.0f), vec2(0.0f));
  }
  barrier();
  W[0] = (W[0] + 0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  F_inner(gl_LocalInvocationIndex);
}
