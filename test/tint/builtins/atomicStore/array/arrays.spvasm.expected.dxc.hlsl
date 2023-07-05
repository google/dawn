static uint local_invocation_index_1 = 0u;
groupshared uint wg[3][2][1];

uint tint_div(uint lhs, uint rhs) {
  return (lhs / ((rhs == 0u) ? 1u : rhs));
}

uint tint_mod(uint lhs, uint rhs) {
  return (lhs % ((rhs == 0u) ? 1u : rhs));
}

void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  while (true) {
    if (!((idx < 6u))) {
      break;
    }
    const uint x_31 = idx;
    const uint x_33 = idx;
    const uint x_35 = idx;
    uint atomic_result = 0u;
    InterlockedExchange(wg[tint_div(x_31, 2u)][tint_mod(x_33, 2u)][tint_mod(x_35, 1u)], 0u, atomic_result);
    {
      idx = (idx + 1u);
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint atomic_result_1 = 0u;
  InterlockedExchange(wg[2][1][0], 1u, atomic_result_1);
  return;
}

void compute_main_1() {
  const uint x_57 = local_invocation_index_1;
  compute_main_inner(x_57);
  return;
}

struct tint_symbol_1 {
  uint local_invocation_index_1_param : SV_GroupIndex;
};

void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    for(uint idx_1 = local_invocation_index_1_param; (idx_1 < 6u); idx_1 = (idx_1 + 1u)) {
      const uint i = (idx_1 / 2u);
      const uint i_1 = (idx_1 % 2u);
      const uint i_2 = (idx_1 % 1u);
      uint atomic_result_2 = 0u;
      InterlockedExchange(wg[i][i_1][i_2], 0u, atomic_result_2);
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
