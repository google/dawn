#version 310 es
precision highp float;
precision highp int;

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


struct SB_RW_atomic {
  int arg_0;
};

struct SB_RW {
  int arg_0;
};

struct tint_symbol {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW_atomic inner;
} sb_rw;

void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = 0;
  int arg_2 = 0;
  tint_symbol res = tint_symbol(0, false);
  arg_1 = 1;
  arg_2 = 1;
  int x_23 = arg_2;
  int x_24 = arg_1;
  atomic_compare_exchange_result_i32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.inner.arg_0, x_24, x_23);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == x_24;
  atomic_compare_exchange_result_i32 tint_symbol_1 = atomic_compare_result;
  int old_value_1 = tint_symbol_1.old_value;
  int x_25 = old_value_1;
  tint_symbol tint_symbol_2 = tint_symbol(x_25, (x_25 == x_23));
  res = tint_symbol_2;
  return;
}

void fragment_main_1() {
  atomicCompareExchangeWeak_1bd40a();
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

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


struct SB_RW_atomic {
  int arg_0;
};

struct SB_RW {
  int arg_0;
};

struct tint_symbol {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430) buffer sb_rw_block_ssbo {
  SB_RW_atomic inner;
} sb_rw;

void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = 0;
  int arg_2 = 0;
  tint_symbol res = tint_symbol(0, false);
  arg_1 = 1;
  arg_2 = 1;
  int x_23 = arg_2;
  int x_24 = arg_1;
  atomic_compare_exchange_result_i32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.inner.arg_0, x_24, x_23);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == x_24;
  atomic_compare_exchange_result_i32 tint_symbol_1 = atomic_compare_result;
  int old_value_1 = tint_symbol_1.old_value;
  int x_25 = old_value_1;
  tint_symbol tint_symbol_2 = tint_symbol(x_25, (x_25 == x_23));
  res = tint_symbol_2;
  return;
}

void compute_main_1() {
  atomicCompareExchangeWeak_1bd40a();
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
