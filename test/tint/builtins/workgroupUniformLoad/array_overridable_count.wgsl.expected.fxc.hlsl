struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int v[128];
int foo() {
  GroupMemoryBarrierWithGroupSync();
  int v_1 = v[0u];
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

void main_inner(uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 128u)) {
        break;
      }
      v[v_3] = int(0);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  foo();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

