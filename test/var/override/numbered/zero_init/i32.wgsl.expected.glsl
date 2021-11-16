#version 310 es
precision mediump float;

#ifndef WGSL_SPEC_CONSTANT_1234
#define WGSL_SPEC_CONSTANT_1234 0
#endif
const int o = WGSL_SPEC_CONSTANT_1234;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  return;
}
void main() {
  tint_symbol();
}


