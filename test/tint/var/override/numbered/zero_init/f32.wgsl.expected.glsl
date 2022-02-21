#version 310 es

#ifndef WGSL_SPEC_CONSTANT_1234
#define WGSL_SPEC_CONSTANT_1234 0.0f
#endif
const float o = WGSL_SPEC_CONSTANT_1234;
void tint_symbol() {
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
