struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared float a[10];
groupshared float b[20];
void f_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 10u)) {
        break;
      }
      a[v_1] = 0.0f;
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 20u)) {
        break;
      }
      b[v_3] = 0.0f;
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  float x = a[0u];
  float y = b[0u];
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

