[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

float3x3 tint_workgroupUniformLoad(inout float3x3 p) {
  GroupMemoryBarrierWithGroupSync();
  const float3x3 result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared float3x3 v;

float3x3 foo() {
  return tint_workgroupUniformLoad(v);
}
