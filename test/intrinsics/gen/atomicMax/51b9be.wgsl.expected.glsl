SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;

void atomicMax_51b9be() {
  uint atomic_result = 0u;
  InterlockedMax(sb_rw.arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

void fragment_main() {
  atomicMax_51b9be();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:11: 'InterlockedMax' : no matching overloaded function found 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;

void atomicMax_51b9be() {
  uint atomic_result = 0u;
  InterlockedMax(sb_rw.arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicMax_51b9be();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:11: 'InterlockedMax' : no matching overloaded function found 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



