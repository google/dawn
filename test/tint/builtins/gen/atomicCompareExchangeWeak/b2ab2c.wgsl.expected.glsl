SKIP: FAILED

#version 310 es

uvec2 tint_atomicCompareExchangeWeak(inout uint param_0, uint param_1, uint param_2) {
  uvec2 result;
  result.x = atomicCompSwap(param_0, param_1, param_2);
  result.y = result.x == param_2 ? 1u : 0u;
  return result;
}


shared uint arg_0;
void atomicCompareExchangeWeak_b2ab2c() {
  uvec2 res = tint_atomicCompareExchangeWeak(arg_0, 1u, 1u);
}

void compute_main(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  atomicCompareExchangeWeak_b2ab2c();
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



