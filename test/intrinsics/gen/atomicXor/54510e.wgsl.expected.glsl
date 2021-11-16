SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;

void atomicXor_54510e() {
  uint atomic_result = 0u;
  InterlockedXor(sb_rw.arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

void fragment_main() {
  atomicXor_54510e();
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
  uint arg_0;
} sb_rw;

void atomicXor_54510e() {
  uint atomic_result = 0u;
  InterlockedXor(sb_rw.arg_0, 1u, atomic_result);
  uint res = atomic_result;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicXor_54510e();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:11: 'InterlockedXor' : no matching overloaded function found 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



