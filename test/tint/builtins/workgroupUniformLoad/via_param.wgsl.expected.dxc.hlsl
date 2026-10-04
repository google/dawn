struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int v[4];
int foo(uint p_indices[1]) {
  GroupMemoryBarrierWithGroupSync();
  int v_1 = v[p_indices[0u]];
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

int bar() {
  uint v_2[1] = (uint[1])0;
  return foo(v_2);
}

void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 4u)) {
        break;
      }
      v[v_3] = int(0);
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  bar();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

