struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int v[4];
typedef int ary_ret[4];
ary_ret foo() {
  GroupMemoryBarrierWithGroupSync();
  int v_1[4] = v;
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 4u)) {
        break;
      }
      v[v_2] = int(0);
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

