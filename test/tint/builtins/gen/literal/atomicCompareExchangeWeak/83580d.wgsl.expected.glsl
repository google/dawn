#version 310 es

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


shared uint arg_0;
void atomicCompareExchangeWeak_83580d() {
  atomic_compare_exchange_result_u32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(arg_0, 1u, 1u);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == 1u;
  atomic_compare_exchange_result_u32 res = atomic_compare_result;
}

void compute_main(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0u);
  }
  barrier();
  atomicCompareExchangeWeak_83580d();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
