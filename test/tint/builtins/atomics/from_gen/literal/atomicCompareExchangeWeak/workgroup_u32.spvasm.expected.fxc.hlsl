struct atomic_compare_exchange_result_u32 {
  uint old_value;
  bool exchanged;
};
struct x__atomic_compare_exchange_resultu32 {
  uint old_value;
  bool exchanged;
};

static uint local_invocation_index_1 = 0u;
groupshared uint arg_0;

void atomicCompareExchangeWeak_83580d() {
  x__atomic_compare_exchange_resultu32 res = (x__atomic_compare_exchange_resultu32)0;
  atomic_compare_exchange_result_u32 atomic_result = (atomic_compare_exchange_result_u32)0;
  uint atomic_compare_value = 1u;
  InterlockedCompareExchange(arg_0, atomic_compare_value, 1u, atomic_result.old_value);
  atomic_result.exchanged = atomic_result.old_value == atomic_compare_value;
  const atomic_compare_exchange_result_u32 tint_symbol = atomic_result;
  const uint old_value_1 = tint_symbol.old_value;
  const uint x_17 = old_value_1;
  const x__atomic_compare_exchange_resultu32 tint_symbol_3 = {x_17, (x_17 == 1u)};
  res = tint_symbol_3;
  return;
}

void compute_main_inner(uint local_invocation_index_2) {
  uint atomic_result_1 = 0u;
  InterlockedExchange(arg_0, 0u, atomic_result_1);
  GroupMemoryBarrierWithGroupSync();
  atomicCompareExchangeWeak_83580d();
  return;
}

void compute_main_1() {
  const uint x_35 = local_invocation_index_1;
  compute_main_inner(x_35);
  return;
}

struct tint_symbol_2 {
  uint local_invocation_index_1_param : SV_GroupIndex;
};

void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    uint atomic_result_2 = 0u;
    InterlockedExchange(arg_0, 0u, atomic_result_2);
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_2 tint_symbol_1) {
  compute_main_inner_1(tint_symbol_1.local_invocation_index_1_param);
  return;
}
