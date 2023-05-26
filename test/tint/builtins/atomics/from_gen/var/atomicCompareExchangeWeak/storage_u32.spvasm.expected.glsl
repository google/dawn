#version 310 es
precision highp float;

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
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  tint_symbol res = tint_symbol(0u, false);
  arg_1 = 1u;
  arg_2 = 1u;
  uint x_21 = arg_2;
  uint x_22 = arg_1;
  atomic_compare_exchange_result_u32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.inner.arg_0, x_22, x_21);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == x_22;
  atomic_compare_exchange_result_u32 tint_symbol_1 = atomic_compare_result;
  uint old_value_1 = tint_symbol_1.old_value;
  uint x_23 = old_value_1;
  tint_symbol tint_symbol_2 = tint_symbol(x_23, (x_23 == x_21));
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
  uint arg_1 = 0u;
  uint arg_2 = 0u;
  tint_symbol res = tint_symbol(0u, false);
  arg_1 = 1u;
  arg_2 = 1u;
  uint x_21 = arg_2;
  uint x_22 = arg_1;
  atomic_compare_exchange_result_u32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.inner.arg_0, x_22, x_21);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == x_22;
  atomic_compare_exchange_result_u32 tint_symbol_1 = atomic_compare_result;
  uint old_value_1 = tint_symbol_1.old_value;
  uint x_23 = old_value_1;
  tint_symbol tint_symbol_2 = tint_symbol(x_23, (x_23 == x_21));
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
