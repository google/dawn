SKIP: FAILED

#version 310 es
precision mediump float;

uvec2 tint_atomicCompareExchangeWeak(inout uint param_0, uint param_1, uint param_2) {
  uvec2 result;
  result.x = atomicCompSwap(param_0, param_1, param_2);
  result.y = result.x == param_2 ? 1u : 0u;
  return result;
}


struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;
void atomicCompareExchangeWeak_6673da() {
  uvec2 res = tint_atomicCompareExchangeWeak(sb_rw.arg_0, 1u, 1u);
}

void fragment_main() {
  atomicCompareExchangeWeak_6673da();
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

uvec2 tint_atomicCompareExchangeWeak(inout uint param_0, uint param_1, uint param_2) {
  uvec2 result;
  result.x = atomicCompSwap(param_0, param_1, param_2);
  result.y = result.x == param_2 ? 1u : 0u;
  return result;
}


struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer SB_RW_1 {
  uint arg_0;
} sb_rw;
void atomicCompareExchangeWeak_6673da() {
  uvec2 res = tint_atomicCompareExchangeWeak(sb_rw.arg_0, 1u, 1u);
}

void compute_main() {
  atomicCompareExchangeWeak_6673da();
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



