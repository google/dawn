struct Inner {
  bool b;
  int4 v;
  float3x3 m;
};

struct Outer {
  Inner a[4];
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared Outer v;
Outer foo() {
  GroupMemoryBarrierWithGroupSync();
  Outer v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

void main_inner(uint tint_local_index) {
  {
    uint v_2 = 0u;
    v_2 = tint_local_index;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 4u)) {
        break;
      }
      Inner v_4 = (Inner)0;
      v.a[v_3] = v_4;
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  GroupMemoryBarrierWithGroupSync();
  foo();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

