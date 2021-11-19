#version 310 es
precision mediump float;

shared uint arg_0;

void atomicOr_5e3d61() {
  uint res = atomicOr(arg_0, 1u);
}

struct tint_symbol_1 {
  uint local_invocation_index;
};

void compute_main_inner(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0u);
  }
  memoryBarrierShared();
  atomicOr_5e3d61();
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


