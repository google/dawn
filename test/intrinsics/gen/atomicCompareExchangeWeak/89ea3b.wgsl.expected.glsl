SKIP: FAILED

#version 310 es
precision mediump float;

groupshared int arg_0;

void atomicCompareExchangeWeak_89ea3b() {
  ivec2 atomic_result = ivec2(0, 0);
  int atomic_compare_value = 1;
  InterlockedCompareExchange(arg_0, atomic_compare_value, 1, atomic_result.x);
  atomic_result.y = atomic_result.x == atomic_compare_value;
  ivec2 res = atomic_result;
}

struct tint_symbol_1 {
  uint local_invocation_index;
};

void compute_main_inner(uint local_invocation_index) {
  {
    int atomic_result_1 = 0;
    InterlockedExchange(arg_0, 0, atomic_result_1);
  }
  GroupMemoryBarrierWithGroupSync();
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


Error parsing GLSL shader:
ERROR: 0:4: '' :  syntax error, unexpected IDENTIFIER
ERROR: 1 compilation errors.  No code generated.



