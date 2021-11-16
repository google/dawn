SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicCompareExchangeWeak_12871c() {
  ivec2 atomic_result = ivec2(0, 0);
  int atomic_compare_value = 1;
  InterlockedCompareExchange(sb_rw.arg_0, atomic_compare_value, 1, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  ivec2 res = atomic_result;
}

void fragment_main() {
  atomicCompareExchangeWeak_12871c();
  return;
}
void main() {
  fragment_main();
}


Error parsing GLSL shader:
ERROR: 0:12: 'InterlockedCompareExchange' : no matching overloaded function found 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  int arg_0;
} sb_rw;

void atomicCompareExchangeWeak_12871c() {
  ivec2 atomic_result = ivec2(0, 0);
  int atomic_compare_value = 1;
  InterlockedCompareExchange(sb_rw.arg_0, atomic_compare_value, 1, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  ivec2 res = atomic_result;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicCompareExchangeWeak_12871c();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:12: 'InterlockedCompareExchange' : no matching overloaded function found 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



