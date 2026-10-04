struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared float a[2];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 2u)) {
        break;
      }
      a[v] = 0.0f;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

