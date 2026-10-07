struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int W[246];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 246u)) {
        break;
      }
      W[v] = int(0);
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  W[int(0)] = int(42);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

