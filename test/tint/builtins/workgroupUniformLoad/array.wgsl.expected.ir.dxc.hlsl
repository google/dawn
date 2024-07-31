
groupshared int v[4];
typedef int ary_ret[4];
ary_ret foo() {
  GroupMemoryBarrierWithGroupSync();
  int v_1[4] = v;
  GroupMemoryBarrierWithGroupSync();
  int v_2[4] = v_1;
  return v_2;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

