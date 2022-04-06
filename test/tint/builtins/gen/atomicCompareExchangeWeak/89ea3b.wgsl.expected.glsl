SKIP: FAILED

#version 310 es

ivec2 tint_atomicCompareExchangeWeak(inout int param_0, int param_1, int param_2) {
  ivec2 result;
  result.x = atomicCompSwap(param_0, param_1, param_2);
  result.y = result.x == param_2 ? 1 : 0;
  return result;
}


shared int arg_0;
void atomicCompareExchangeWeak_89ea3b() {
  ivec2 res = tint_atomicCompareExchangeWeak(arg_0, 1, 1);
}

void compute_main(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0);
  }
  barrier();
  atomicCompareExchangeWeak_89ea3b();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
Error parsing GLSL shader:
ERROR: 0:5: 'atomicCompSwap' : Atomic memory function can only be used for shader storage block member or shared variable. 
ERROR: 0:5: '' : compilation terminated 
ERROR: 2 compilation errors.  No code generated.



