#version 310 es
precision highp float;

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


bool tint_discarded = false;
struct tint_symbol_1 {
  int old_value;
  bool exchanged;
};

layout(location = 0) out int value;
layout(binding = 0, std430) buffer a_block_ssbo {
  int inner;
} a;

int foo() {
  tint_discarded = true;
  int x = 0;
  tint_symbol_1 tint_symbol = tint_symbol_1(0, false);
  if (!(tint_discarded)) {
    atomic_compare_exchange_result_i32 atomic_compare_result;
    atomic_compare_result.old_value = atomicCompSwap(a.inner, 0, 1);
    atomic_compare_result.exchanged = atomic_compare_result.old_value == 0;
    atomic_compare_exchange_result_i32 tint_symbol_2 = atomic_compare_result;
    tint_symbol.old_value = tint_symbol_2.old_value;
    tint_symbol.exchanged = tint_symbol_2.exchanged;
  }
  tint_symbol_1 result = tint_symbol;
  if (result.exchanged) {
    x = result.old_value;
  }
  return x;
}

void main() {
  int inner_result = foo();
  value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return;
}
