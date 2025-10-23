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
  uint v_2[1] = {0u};
  return foo(v_2);
}

void main_inner(uint tint_local_index) {
  {
    uint v_3 = 0u;
    v_3 = tint_local_index;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 4u)) {
        break;
      }
      v[v_4] = int(0);
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  bar();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

