struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint v[16];
void foo() {
  min(uint(int(0)), (4u - 1u));
  v[0u] = 1065353216u;
}

void main_inner(uint tint_local_index) {
  {
    uint v_1 = 0u;
    v_1 = tint_local_index;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 16u)) {
        break;
      }
      v[((v_2 * 4u) / 4u)] = 0u;
      {
        v_1 = (v_2 + 1u);
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

