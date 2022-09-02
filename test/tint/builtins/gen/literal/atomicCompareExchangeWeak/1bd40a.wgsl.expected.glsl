#version 310 es
precision mediump float;

struct atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};


layout(binding = 0, std430) buffer SB_RW_ssbo {
  int arg_0;
} sb_rw;

void atomicCompareExchangeWeak_1bd40a() {
  atomic_compare_exchange_resulti32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.arg_0, 1, 1);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == 1;
  atomic_compare_exchange_resulti32 res = atomic_compare_result;
}

void fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};


layout(binding = 0, std430) buffer SB_RW_ssbo {
  int arg_0;
} sb_rw;

void atomicCompareExchangeWeak_1bd40a() {
  atomic_compare_exchange_resulti32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.arg_0, 1, 1);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == 1;
  atomic_compare_exchange_resulti32 res = atomic_compare_result;
}

void compute_main() {
  atomicCompareExchangeWeak_1bd40a();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
