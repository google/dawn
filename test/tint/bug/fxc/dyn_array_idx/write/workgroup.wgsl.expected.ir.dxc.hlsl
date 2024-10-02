struct S {
  int data[64];
};

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_ubo : register(b0) {
  uint4 ubo[1];
};
RWByteAddressBuffer result : register(u1);
groupshared S s;
void f_inner(uint tint_local_index) {
  {
    uint v = 0u;
    v = tint_local_index;
    while(true) {
      uint v_1 = v;
      if ((v_1 >= 64u)) {
        break;
      }
      s.data[v_1] = int(0);
      {
        v = (v_1 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int v_2 = asint(ubo[0u].x);
  s.data[v_2] = int(1);
  result.Store(0u, asuint(s.data[int(3)]));
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

