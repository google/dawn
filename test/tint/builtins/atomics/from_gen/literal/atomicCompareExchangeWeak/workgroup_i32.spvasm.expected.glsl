#version 310 es

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


shared int arg_0;
void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    atomicExchange(arg_0, 0);
  }
  barrier();
}

struct tint_symbol {
  int old_value;
  bool exchanged;
};

uint local_invocation_index_1 = 0u;
void atomicCompareExchangeWeak_e88938() {
  tint_symbol res = tint_symbol(0, false);
  atomic_compare_exchange_result_i32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(arg_0, 1, 1);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == 1;
  atomic_compare_exchange_result_i32 tint_symbol_1 = atomic_compare_result;
  int old_value_1 = tint_symbol_1.old_value;
  int x_18 = old_value_1;
  tint_symbol tint_symbol_2 = tint_symbol(x_18, (x_18 == 1));
  res = tint_symbol_2;
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  atomicExchange(arg_0, 0);
  barrier();
  atomicCompareExchangeWeak_e88938();
  return;
}

void compute_main_1() {
  uint x_36 = local_invocation_index_1;
  compute_main_inner(x_36);
  return;
}

void compute_main(uint local_invocation_index_1_param) {
  tint_zero_workgroup_memory(local_invocation_index_1_param);
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main(gl_LocalInvocationIndex);
  return;
}
