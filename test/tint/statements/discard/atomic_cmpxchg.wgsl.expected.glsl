#version 310 es
precision highp float;
precision highp int;

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


bool tint_discarded = false;
layout(location = 0) out int value;
layout(binding = 0, std430) buffer a_block_ssbo {
  int inner;
} a;

int foo() {
  tint_discarded = true;
  int x = 0;
  atomic_compare_exchange_result_i32 tint_symbol = atomic_compare_exchange_result_i32(0, false);
  if (!(tint_discarded)) {
    atomic_compare_exchange_result_i32 atomic_compare_result;
    atomic_compare_result.old_value = atomicCompSwap(a.inner, 0, 1);
    atomic_compare_result.exchanged = atomic_compare_result.old_value == 0;
    tint_symbol = atomic_compare_result;
  }
  atomic_compare_exchange_result_i32 result = tint_symbol;
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
