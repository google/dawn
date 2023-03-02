[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct Inner {
  bool b;
  int4 v;
  float3x3 m;
};
struct Outer {
  Inner a[4];
};

Outer tint_workgroupUniformLoad(inout Outer p) {
  GroupMemoryBarrierWithGroupSync();
  const Outer result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared Outer v;

Outer foo() {
  return tint_workgroupUniformLoad(v);
}
