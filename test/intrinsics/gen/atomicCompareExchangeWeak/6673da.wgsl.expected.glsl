SKIP: FAILED

#version 310 es
precision mediump float;


layout (binding = 0) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;

void atomicCompareExchangeWeak_6673da() {
  uvec2 atomic_result = uvec2(0u, 0u);
  uint atomic_compare_value = 1u;
  InterlockedCompareExchange(sb_rw.arg_0, atomic_compare_value, 1u, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  uvec2 res = atomic_result;
}

void fragment_main() {
  atomicCompareExchangeWeak_6673da();
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
  uint arg_0;
} sb_rw;

void atomicCompareExchangeWeak_6673da() {
  uvec2 atomic_result = uvec2(0u, 0u);
  uint atomic_compare_value = 1u;
  InterlockedCompareExchange(sb_rw.arg_0, atomic_compare_value, 1u, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  uvec2 res = atomic_result;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main() {
  atomicCompareExchangeWeak_6673da();
  return;
}
void main() {
  compute_main();
}


Error parsing GLSL shader:
ERROR: 0:12: 'InterlockedCompareExchange' : no matching overloaded function found 
ERROR: 0:12: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



