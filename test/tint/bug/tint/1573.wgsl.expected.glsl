#version 310 es

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


layout(binding = 0, std430) buffer a_block_ssbo {
  uint inner;
} a;

void tint_symbol() {
  uint value = 42u;
  atomic_compare_exchange_result_u32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(a.inner, 0u, value);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == 0u;
  atomic_compare_exchange_result_u32 result = atomic_compare_result;
}

layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main() {
  tint_symbol();
  return;
}
