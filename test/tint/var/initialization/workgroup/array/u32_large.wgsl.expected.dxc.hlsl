struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int zero[23];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 23u)) {
        break;
      }
      zero[v_1] = int(0);
      {
        idx = (idx + 13u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int v[23] = zero;
}

[numthreads(13, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

