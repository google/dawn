static bool tint_discarded = false;

struct tint_symbol_2 {
  int old_value;
  bool exchanged;
};

RWByteAddressBuffer a : register(u0, space0);

struct tint_symbol {
  int value : SV_Target0;
};
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


int foo_inner() {
  tint_discarded = true;
  int x = 0;
  tint_symbol_2 tint_symbol_1 = (tint_symbol_2)0;
  if (!(tint_discarded)) {
    const atomic_compare_exchange_weak_ret_type tint_symbol_3 = tint_atomicCompareExchangeWeak(a, 0u, 0, 1);
    tint_symbol_1.old_value = tint_symbol_3.old_value;
    tint_symbol_1.exchanged = tint_symbol_3.exchanged;
  }
  const tint_symbol_2 result = tint_symbol_1;
  if (result.exchanged) {
    x = result.old_value;
  }
  return x;
}

tint_symbol foo() {
  const int inner_result = foo_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.value = inner_result;
  if (tint_discarded) {
    discard;
  }
  return wrapper_result;
}
