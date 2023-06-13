#version 310 es

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


layout(binding = 0, std430) buffer a_block_ssbo {
  int inner;
} a;

void compute_main() {
  atomic_compare_exchange_result_i32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(a.inner, 1, 1);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == 1;
  atomic_compare_exchange_result_i32 tint_symbol = atomic_compare_result;
  int v = tint_symbol.old_value;
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
