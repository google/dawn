#version 310 es
precision mediump float;

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

struct tint_symbol_1 {
  uint local_invocation_index;
};

void compute_main_inner(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0);
  }
  memoryBarrierShared();
  atomicCompareExchangeWeak_89ea3b();
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


