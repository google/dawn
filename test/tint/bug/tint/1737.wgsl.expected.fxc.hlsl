struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared float a[10];
groupshared float b[20];
void f_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v = idx;
      if ((v >= 10u)) {
        break;
      }
      a[v] = 0.0f;
      {
        idx = (idx + 1u);
      }
    }
  }
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 20u)) {
        break;
      }
      b[v_1] = 0.0f;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  float x = a[int(0)];
  float y = b[int(0)];
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

