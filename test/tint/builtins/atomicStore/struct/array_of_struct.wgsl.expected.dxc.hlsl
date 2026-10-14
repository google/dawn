struct S {
  int x;
  uint a;
  uint y;
};

struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared S wg[10];
void compute_main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 10u)) {
        break;
      }
      wg[v].x = int(0);
      uint v_1 = 0u;
      InterlockedExchange(wg[v].a, 0u, v_1);
      wg[v].y = 0u;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_2 = 0u;
  InterlockedExchange(wg[int(4)].a, 1u, v_2);
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

