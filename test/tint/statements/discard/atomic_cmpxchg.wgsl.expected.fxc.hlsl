struct atomic_compare_exchange_result_i32 {
  int old_value;
  bool exchanged;
};
static bool tint_discarded = false;

struct tint_symbol_2 {
  int old_value;
  bool exchanged;
};

RWByteAddressBuffer a : register(u0);

struct tint_symbol {
  int value : SV_Target0;
};

atomic_compare_exchange_result_i32 aatomicCompareExchangeWeak(uint offset, int compare, int value) {
  atomic_compare_exchange_result_i32 result=(atomic_compare_exchange_result_i32)0;
  a.InterlockedCompareExchange(offset, compare, value, result.old_value);
  result.exchanged = result.old_value == compare;
  return result;
}


int foo_inner() {
  tint_discarded = true;
  int x = 0;
  tint_symbol_2 tint_symbol_1 = (tint_symbol_2)0;
  if (!(tint_discarded)) {
    const atomic_compare_exchange_result_i32 tint_symbol_3 = aatomicCompareExchangeWeak(0u, 0, 1);
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
