#version 310 es

shared mat2 S;
void func(uint pointer_indices[1]) {
  S[pointer_indices[0u]] = vec2(0.0f);
}
void tint_symbol_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    S = mat2(vec2(0.0f), vec2(0.0f));
  }
  barrier();
  func(uint[1](uint(1)));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationIndex);
}
