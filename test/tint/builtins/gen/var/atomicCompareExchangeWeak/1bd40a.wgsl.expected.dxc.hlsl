RWByteAddressBuffer sb_rw : register(u0, space0);

struct atomic_compare_exchange_weak_ret_type {
  int old_value;
  bool exchanged;
};

atomic_compare_exchange_weak_ret_type sb_rwatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_weak_ret_type result=(atomic_compare_exchange_weak_ret_type)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_1bd40a() {
  int arg_1 = 1;
  int arg_2 = 1;
  atomic_compare_exchange_weak_ret_type res = sb_rwatomicCompareExchangeWeak(0u, arg_1, arg_2);
}

void fragment_main() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_1bd40a();
  return;
}
