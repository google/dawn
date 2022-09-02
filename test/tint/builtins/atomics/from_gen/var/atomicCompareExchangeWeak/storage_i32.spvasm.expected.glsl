#version 310 es
precision mediump float;

struct atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};


struct SB_RW {
  int arg_0;
};

struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430) buffer SB_RW_atomic_ssbo {
  int arg_0;
} sb_rw;

void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = 0;
  int arg_2 = 0;
  x__atomic_compare_exchange_resulti32 res = x__atomic_compare_exchange_resulti32(0, false);
  arg_1 = 1;
  arg_2 = 1;
  int x_23 = arg_2;
  atomic_compare_exchange_resulti32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.arg_0, arg_1, x_23);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == arg_1;
  atomic_compare_exchange_resulti32 tint_symbol = atomic_compare_result;
  int old_value_1 = tint_symbol.old_value;
  int x_25 = old_value_1;
  x__atomic_compare_exchange_resulti32 tint_symbol_1 = x__atomic_compare_exchange_resulti32(x_25, (x_25 == x_23));
  res = tint_symbol_1;
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

struct atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};


struct SB_RW {
  int arg_0;
};

struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

layout(binding = 0, std430) buffer SB_RW_atomic_ssbo {
  int arg_0;
} sb_rw;

void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = 0;
  int arg_2 = 0;
  x__atomic_compare_exchange_resulti32 res = x__atomic_compare_exchange_resulti32(0, false);
  arg_1 = 1;
  arg_2 = 1;
  int x_23 = arg_2;
  atomic_compare_exchange_resulti32 atomic_compare_result;
  atomic_compare_result.old_value = atomicCompSwap(sb_rw.arg_0, arg_1, x_23);
  atomic_compare_result.exchanged = atomic_compare_result.old_value == arg_1;
  atomic_compare_exchange_resulti32 tint_symbol = atomic_compare_result;
  int old_value_1 = tint_symbol.old_value;
  int x_25 = old_value_1;
  x__atomic_compare_exchange_resulti32 tint_symbol_1 = x__atomic_compare_exchange_resulti32(x_25, (x_25 == x_23));
  res = tint_symbol_1;
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
