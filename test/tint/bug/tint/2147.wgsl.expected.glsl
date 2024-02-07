#version 310 es
precision highp float;
precision highp int;

struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};


bool tint_discarded = false;
layout(location = 0) out vec4 value;
layout(binding = 0, std430) buffer S_block_ssbo {
  int inner;
} S;

vec4 tint_symbol() {
  if (false) {
    tint_discarded = true;
  }
  atomic_compare_exchange_result_i32 tint_symbol_2 = atomic_compare_exchange_result_i32(0, false);
  if (!(tint_discarded)) {
    atomic_compare_exchange_result_i32 atomic_compare_result;
    atomic_compare_result.old_value = atomicCompSwap(S.inner, 0, 1);
    atomic_compare_result.exchanged = atomic_compare_result.old_value == 0;
    tint_symbol_2 = atomic_compare_result;
  }
  atomic_compare_exchange_result_i32 tint_symbol_1 = tint_symbol_2;
  int old_value = tint_symbol_1.old_value;
  return vec4(float(old_value));
}

void main() {
  vec4 inner_result = tint_symbol();
  value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return;
}
