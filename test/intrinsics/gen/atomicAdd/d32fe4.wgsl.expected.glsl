SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicAdd_d32fe4() {
  int atomic_result = 0;
  InterlockedAdd(sb_rw.arg_0, 1, atomic_result);
  int res = atomic_result;
}

void fragment_main() {
  atomicAdd_d32fe4();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:11: 'InterlockedAdd' : no matching overloaded function found 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicAdd_d32fe4() {
  int atomic_result = 0;
  InterlockedAdd(sb_rw.arg_0, 1, atomic_result);
  int res = atomic_result;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicAdd_d32fe4();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:11: 'InterlockedAdd' : no matching overloaded function found 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



