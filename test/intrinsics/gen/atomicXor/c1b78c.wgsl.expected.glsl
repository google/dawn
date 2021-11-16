SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicXor_c1b78c() {
  int atomic_result = 0;
  InterlockedXor(sb_rw.arg_0, 1, atomic_result);
  int res = atomic_result;
}

void fragment_main() {
  atomicXor_c1b78c();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:11: 'InterlockedXor' : no matching overloaded function found 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicXor_c1b78c() {
  int atomic_result = 0;
  InterlockedXor(sb_rw.arg_0, 1, atomic_result);
  int res = atomic_result;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicXor_c1b78c();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:11: 'InterlockedXor' : no matching overloaded function found 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



