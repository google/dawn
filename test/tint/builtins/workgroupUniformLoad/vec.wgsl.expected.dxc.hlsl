[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

float4 tint_workgroupUniformLoad(inout float4 p) {
  GroupMemoryBarrierWithGroupSync();
  const float4 result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared float4 v;

float4 foo() {
  return tint_workgroupUniformLoad(v);
}
