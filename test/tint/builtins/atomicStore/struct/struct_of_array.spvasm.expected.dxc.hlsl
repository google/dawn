struct S_atomic {
  int x;
  uint a[10];
  uint y;
};

static uint local_invocation_index_1 = 0u;
groupshared S_atomic wg;

void compute_main_inner(uint local_invocation_index) {
  uint idx = 0u;
  wg.x = 0;
  wg.y = 0u;
  idx = local_invocation_index;
  {
    [loop] for(; !(!((idx < 10u))); idx = (idx + 1u)) {
      uint atomic_result = 0u;
      InterlockedExchange(wg.a[idx], 0u, atomic_result);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_1 = 0u;
  InterlockedExchange(wg.a[4], 1u, atomic_result_1);
  return;
}

void compute_main_1() {
  compute_main_inner(local_invocation_index_1);
  return;
}

struct tint_symbol_1 {
  uint local_invocation_index_1_param : SV_GroupIndex;
};

void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    wg.x = 0;
    wg.y = 0u;
  }
  {
    [loop] for(uint idx_1 = local_invocation_index_1_param; (idx_1 < 10u); idx_1 = (idx_1 + 1u)) {
      const uint i = idx_1;
      uint atomic_result_2 = 0u;
      InterlockedExchange(wg.a[i], 0u, atomic_result_2);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(tint_symbol_1 tint_symbol) {
  compute_main_inner_1(tint_symbol.local_invocation_index_1_param);
  return;
}
