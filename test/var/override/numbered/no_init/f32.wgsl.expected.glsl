SKIP: FAILED

#version 310 es
precision mediump float;

#ifndef WGSL_SPEC_CONSTANT_1234
#error spec constant required for constant id 1234
#endif
const float o = WGSL_SPEC_CONSTANT_1234;

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void tint_symbol() {
  return;
}
void main() {
  tint_symbol();
}


Error parsing GLSL shader:
ERROR: 0:5: '#error' : spec constant required for constant id 1234  
ERROR: 0:6: '' : missing #endif 
ERROR: 2 compilation errors.  No code generated.



