#version 310 es

#ifndef WGSL_SPEC_CONSTANT_0
#define WGSL_SPEC_CONSTANT_0 2
#endif
const int x_dim = WGSL_SPEC_CONSTANT_0;
void tint_symbol() {
}

layout(local_size_x = 3, local_size_y = WGSL_SPEC_CONSTANT_0, local_size_z = 3) in;
void main() {
  tint_symbol();
  return;
}
