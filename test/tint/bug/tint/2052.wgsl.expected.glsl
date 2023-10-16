SKIP: FAILED

#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void unused_entry_point() {
  return;
}
void f() {
  discard;
}

error: Error parsing GLSL shader:
ERROR: 0:8: 'discard' : not supported in this stage: compute
ERROR: 0:8: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



