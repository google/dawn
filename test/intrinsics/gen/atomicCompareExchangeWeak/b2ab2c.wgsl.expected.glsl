#version 310 es
precision mediump float;

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

struct tint_symbol_1 {
  uint local_invocation_index;
};

void compute_main_inner(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0u);
  }
  memoryBarrierShared();
  atomicCompareExchangeWeak_b2ab2c();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner(tint_symbol.local_invocation_index);
  return;
}
void main() {
  tint_symbol_1 inputs;
  inputs.local_invocation_index = uint(gl_LocalInvocationIndex);
  compute_main(inputs);
}


