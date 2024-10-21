#version 310 es

shared mat2x3 v;
void tint_symbol_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    v = mat2x3(vec3(0.0f), vec3(0.0f));
  }
  barrier();
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol_inner(gl_LocalInvocationIndex);
}
