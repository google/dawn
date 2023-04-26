#version 310 es
precision highp float;

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

void atomicCompareExchangeWeak_63d8e6() {
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  atomic_compare_exchange_result_u32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.inner.arg_0, arg_1, arg_2);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == arg_1;
  atomic_compare_exchange_result_u32 res = atomic_compare_result;
}

void fragment_main() {
  atomicCompareExchangeWeak_63d8e6();
}

void main() {
  fragment_main();
  return;
}
#version 310 es

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


struct SB_RW {
  uint arg_0;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW inner;
} sb_rw;

void atomicCompareExchangeWeak_63d8e6() {
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  atomic_compare_exchange_result_u32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.inner.arg_0, arg_1, arg_2);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == arg_1;
  atomic_compare_exchange_result_u32 res = atomic_compare_result;
}

void compute_main() {
  atomicCompareExchangeWeak_63d8e6();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
