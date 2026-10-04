struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint v[16];
void foo() {
  min(0u, (4u - 1u));
  v[0u] = 1065353216u;
}

void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 16u)) {
        break;
      }
      v[((v_1 * 4u) / 4u)] = 0u;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  foo();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

