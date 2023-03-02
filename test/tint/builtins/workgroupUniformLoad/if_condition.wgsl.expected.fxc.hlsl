[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

bool tint_workgroupUniformLoad(inout bool p) {
  GroupMemoryBarrierWithGroupSync();
  const bool result = p;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

groupshared bool v;

int foo() {
  if (tint_workgroupUniformLoad(v)) {
    return 42;
  }
  return 0;
}
