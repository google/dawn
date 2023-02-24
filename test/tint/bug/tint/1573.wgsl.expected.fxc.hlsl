RWByteAddressBuffer a : register(u0, space0);

struct atomic_compare_exchange_weak_ret_type {
  uint old_value;
  bool exchanged;
};

atomic_compare_exchange_weak_ret_type aatomicCompareExchangeWeak(uint offset, uint compare, uint value) {
  atomic_compare_exchange_weak_ret_type result=(atomic_compare_exchange_weak_ret_type)0;
  a.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


[numthreads(16, 1, 1)]
void main() {
  uint value = 42u;
  const atomic_compare_exchange_weak_ret_type result = aatomicCompareExchangeWeak(0u, 0u, value);
  return;
}
