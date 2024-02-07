#version 310 es
precision highp float;
precision highp int;

struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};


struct SB_RW_atomic {
  uint arg_0;
};

struct SB_RW {
  uint arg_0;
};

struct tint_symbol {
  uint old_value;
  bool exchanged;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW_atomic inner;
} sb_rw;

void atomicCompareExchangeWeak_63d8e6() {
  tint_symbol res = tint_symbol(0u, false);
  atomic_compare_exchange_result_u32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.inner.arg_0, 1u, 1u);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == 1u;
  atomic_compare_exchange_result_u32 tint_symbol_1 = atomic_compare_result;
  uint old_value_1 = tint_symbol_1.old_value;
  uint x_17 = old_value_1;
  tint_symbol tint_symbol_2 = tint_symbol(x_17, (x_17 == 1u));
  res = tint_symbol_2;
  return;
}

void fragment_main_1() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}

void fragment_main() {
  fragment_main_1();
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


struct SB_RW_atomic {
  uint arg_0;
};

struct SB_RW {
  uint arg_0;
};

struct tint_symbol {
  uint old_value;
  bool exchanged;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW_atomic inner;
} sb_rw;

void atomicCompareExchangeWeak_63d8e6() {
  tint_symbol res = tint_symbol(0u, false);
  atomic_compare_exchange_result_u32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.inner.arg_0, 1u, 1u);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == 1u;
  atomic_compare_exchange_result_u32 tint_symbol_1 = atomic_compare_result;
  uint old_value_1 = tint_symbol_1.old_value;
  uint x_17 = old_value_1;
  tint_symbol tint_symbol_2 = tint_symbol(x_17, (x_17 == 1u));
  res = tint_symbol_2;
  return;
}

void compute_main_1() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}

void compute_main() {
  compute_main_1();
}

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  compute_main();
  return;
}
