RWByteAddressBuffer sb_rw : register(u0);

struct atomic_compare_exchange_weak_ret_type {
  uint old_value;
  bool exchanged;
};

atomic_compare_exchange_weak_ret_type sb_rwatomicCompareExchangeWeak(uint offset, uint compare, uint value) {
  atomic_compare_exchange_weak_ret_type result=(atomic_compare_exchange_weak_ret_type)0;
  sb_rw.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


void atomicCompareExchangeWeak_63d8e6() {
  atomic_compare_exchange_weak_ret_type res = sb_rwatomicCompareExchangeWeak(0u, 1u, 1u);
}

void fragment_main() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  atomicCompareExchangeWeak_63d8e6();
  return;
}
