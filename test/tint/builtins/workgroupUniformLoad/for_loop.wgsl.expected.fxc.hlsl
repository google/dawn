struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int a;
groupshared int b;
void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    a = int(0);
    b = int(0);
  }
  GroupMemoryBarrierWithGroupSync();
  {
    uint2 tint_loop_idx = (4294967295u).xx;
    int i = int(0);
    while(true) {
      if (all((tint_loop_idx == (0u).xx))) {
        break;
      }
      int v = i;
      GroupMemoryBarrierWithGroupSync();
      int v_1 = a;
      GroupMemoryBarrierWithGroupSync();
      if ((v < v_1)) {
      } else {
        break;
      }
      {
        uint tint_low_inc = (tint_loop_idx.x - 1u);
        tint_loop_idx.x = tint_low_inc;
        uint tint_carry = uint((tint_low_inc == 4294967295u));
        tint_loop_idx.y = (tint_loop_idx.y - tint_carry);
        GroupMemoryBarrierWithGroupSync();
        int v_2 = b;
        GroupMemoryBarrierWithGroupSync();
        i = asint((asuint(i) + asuint(v_2)));
      }
      continue;
    }
  }
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

