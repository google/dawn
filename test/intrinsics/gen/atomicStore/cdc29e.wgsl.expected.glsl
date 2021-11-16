SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;

void atomicStore_cdc29e() {
  uint atomic_result = 0u;
  InterlockedExchange(sb_rw.arg_0, 1u, atomic_result);
}

void fragment_main() {
  atomicStore_cdc29e();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:11: 'InterlockedExchange' : no matching overloaded function found 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;

void atomicStore_cdc29e() {
  uint atomic_result = 0u;
  InterlockedExchange(sb_rw.arg_0, 1u, atomic_result);
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicStore_cdc29e();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:11: 'InterlockedExchange' : no matching overloaded function found 
ERROR: 0:11: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



