#version 310 es

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


shared int arg_0;
void atomicCompareExchangeWeak_e88938() {
  int arg_1 = 1;
  int arg_2 = 1;
  atomic_compare_exchange_result_i32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(arg_0, arg_1, arg_2);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == arg_1;
  atomic_compare_exchange_result_i32 res = atomic_compare_result;
}

void compute_main(uint local_invocation_index) {
  {
    atomicExchange(arg_0, 0);
  }
  barrier();
  atomicCompareExchangeWeak_e88938();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
