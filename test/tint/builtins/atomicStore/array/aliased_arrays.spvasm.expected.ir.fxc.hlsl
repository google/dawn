struct compute_main_inputs {
  uint local_invocation_index_1_param : SV_GroupIndex;
};


static uint local_invocation_index_1 = 0u;
groupshared uint wg[3][2][1];
uint tint_mod_u32(uint lhs, uint rhs) {
  uint v = (((rhs == 0u)) ? (1u) : (rhs));
  return (lhs - ((lhs / v) * v));
}

uint tint_div_u32(uint lhs, uint rhs) {
  return (lhs / (((rhs == 0u)) ? (1u) : (rhs)));
}

void compute_main_inner(uint local_invocation_index_2) {
  uint idx = 0u;
  idx = local_invocation_index_2;
  {
    while(true) {
      if (!((idx < 6u))) {
        break;
      }
      uint x_31 = idx;
      uint x_33 = idx;
      uint x_35 = idx;
      uint v_1 = tint_div_u32(x_31, 2u);
      uint v_2 = tint_mod_u32(x_33, 2u);
      uint v_3 = 0u;
      InterlockedExchange(wg[v_1][v_2][tint_mod_u32(x_35, 1u)], 0u, v_3);
      {
        idx = (idx + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_4 = 0u;
  InterlockedExchange(wg[2][1][0], 1u, v_4);
}

void compute_main_1() {
  uint x_57 = local_invocation_index_1;
  compute_main_inner(x_57);
}

void compute_main_inner_1(uint local_invocation_index_1_param) {
  {
    uint v_5 = 0u;
    v_5 = local_invocation_index_1_param;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 6u)) {
        break;
      }
      uint v_7 = 0u;
      InterlockedExchange(wg[(v_6 / 2u)][(v_6 % 2u)][0u], 0u, v_7);
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  local_invocation_index_1 = local_invocation_index_1_param;
  compute_main_1();
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner_1(inputs.local_invocation_index_1_param);
}

