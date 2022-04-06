SKIP: FAILED

#version 310 es
precision mediump float;

ivec2 tint_atomicCompareExchangeWeak(inout int param_0, int param_1, int param_2) {
  ivec2 result;
  result.x = atomicCompSwap(param_0, param_1, param_2);
  result.y = result.x == param_2 ? 1 : 0;
  return result;
}


struct SB_RW {
  int arg_0;
};

layout(binding = 0, std430) buffer SB_RW_1 {
  int arg_0;
} sb_rw;
void atomicCompareExchangeWeak_12871c() {
  ivec2 res = tint_atomicCompareExchangeWeak(sb_rw.arg_0, 1, 1);
}

void fragment_main() {
  atomicCompareExchangeWeak_12871c();
}

void main() {
  fragment_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:6: 'atomicCompSwap' : Atomic memory function can only be used for shader storage block member or shared variable. 
ERROR: 0:6: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



#version 310 es

ivec2 tint_atomicCompareExchangeWeak(inout int param_0, int param_1, int param_2) {
  ivec2 result;
  result.x = atomicCompSwap(param_0, param_1, param_2);
  result.y = result.x == param_2 ? 1 : 0;
  return result;
}


struct SB_RW {
  int arg_0;
};

layout(binding = 0, std430) buffer SB_RW_1 {
  int arg_0;
} sb_rw;
void atomicCompareExchangeWeak_12871c() {
  ivec2 res = tint_atomicCompareExchangeWeak(sb_rw.arg_0, 1, 1);
}

void compute_main() {
  atomicCompareExchangeWeak_12871c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
Error parsing GLSL shader:
ERROR: 0:5: 'atomicCompSwap' : Atomic memory function can only be used for shader storage block member or shared variable. 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



