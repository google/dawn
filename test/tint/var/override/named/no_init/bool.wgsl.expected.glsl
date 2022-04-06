SKIP: FAILED

#version 310 es

#ifndef WGSL_SPEC_CONSTANT_0
#error spec constant required for constant id 0
#endif
const bool o = WGSL_SPEC_CONSTANT_0;
void tint_symbol() {
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
Error parsing GLSL shader:
ERROR: 0:4: '#error' : spec constant required for constant id 0  
ERROR: 0:5: '' : missing #endif 
ERROR: 0:5: '' : compilation terminated 
ERROR: 3 compilation errors.  No code generated.



