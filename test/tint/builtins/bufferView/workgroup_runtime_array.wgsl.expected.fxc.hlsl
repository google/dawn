struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
groupshared uint b[64];
void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 64u)) {
        break;
      }
      b[((v_2 * 4u) / 4u)] = 0u;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  {
    int i = int(0);
    for( ; (i < int(64)); i = asint((asuint(i) + 1u))) {
      uint v_3 = (min(uint(i), (64u - 1u)) * 4u);
      uint v_4 = 0u;
      v.GetDimensions(v_4);
      uint v_5 = ((v_4 / 4u) - 1u);
      b[(v_3 / 4u)] = v.Load((0u + (min(uint(i), v_5) * 4u)));
    }
  }
  GroupMemoryBarrierWithGroupSync();
  {
    int i = int(0);
    for( ; (i < int(64)); i = asint((asuint(i) + 1u))) {
      uint v_6 = 0u;
      v_1.GetDimensions(v_6);
      uint v_7 = ((v_6 / 4u) - 1u);
      uint v_8 = (min(uint(i), v_7) * 4u);
      uint v_9 = ((min(uint(i), (64u - 1u)) * 4u) / 4u);
      v_1.Store((0u + v_8), b[v_9]);
    }
  }
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

