struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint wg[4];
void compute_main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 4u)) {
        break;
      }
      uint v_1 = 0u;
      InterlockedExchange(wg[v], 0u, v_1);
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_2 = 0u;
  InterlockedExchange(wg[int(1)], 1u, v_2);
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

