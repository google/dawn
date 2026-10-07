struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared uint v[16];
RWByteAddressBuffer v_1 : register(u0);
void foo() {
  v_1.Store(0u, 64u);
}

void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 16u)) {
        break;
      }
      v[((v_2 * 4u) / 4u)] = 0u;
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

