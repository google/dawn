struct x__atomic_compare_exchange_resulti32 {
  int old_value;
  bool exchanged;
};

RWByteAddressBuffer sb_rw : register(u0, space0);

struct atomic_compare_exchange_weak_ret_type {
  int old_value;
  bool exchanged;
};

atomic_compare_exchange_weak_ret_type tint_atomicCompareExchangeWeak(RWByteAddressBuffer buffer, uint offset, int compare, int value) {
  atomic_compare_exchange_weak_ret_type result=(atomic_compare_exchange_weak_ret_type)0;
  buffer.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_1bd40a() {
  x__atomic_compare_exchange_resulti32 res = {0, false};
  const atomic_compare_exchange_weak_ret_type tint_symbol = tint_atomicCompareExchangeWeak(sb_rw, 0u, 1, 1);
  const int old_value_1 = tint_symbol.old_value;
  const int x_19 = old_value_1;
  const x__atomic_compare_exchange_resulti32 tint_symbol_1 = {x_19, (x_19 == 1)};
  res = tint_symbol_1;
  return;
}

void fragment_main_1() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
