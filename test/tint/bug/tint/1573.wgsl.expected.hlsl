RWByteAddressBuffer a : register(u0, space0);

struct atomic_compare_exchange_weak_ret_type {
  uint old_value;
  bool exchanged;
};

atomic_compare_exchange_weak_ret_type tint_atomicCompareExchangeWeak(RWByteAddressBuffer buffer, uint offset, uint compare, uint value) {
  atomic_compare_exchange_weak_ret_type result=(atomic_compare_exchange_weak_ret_type)0;
  buffer.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


[numthreads(16, 1, 1)]
void main() {
  uint value = 42u;
  const atomic_compare_exchange_weak_ret_type result = tint_atomicCompareExchangeWeak(a, 0u, 0u, value);
  return;
}
