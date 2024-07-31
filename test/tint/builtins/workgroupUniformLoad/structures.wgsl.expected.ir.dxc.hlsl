struct Inner {
  bool b;
  int4 v;
  float3x3 m;
};

struct Outer {
  Inner a[4];
};


groupshared Outer v;
Outer foo() {
  GroupMemoryBarrierWithGroupSync();
  Outer v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  Outer v_2 = v_1;
  return v_2;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

