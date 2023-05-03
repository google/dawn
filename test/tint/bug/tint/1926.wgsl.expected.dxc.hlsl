groupshared uint sh_atomic_failed;

uint tint_workgroupUniformLoad_sh_atomic_failed() {
  GroupMemoryBarrierWithGroupSync();
  const uint result = sh_atomic_failed;
  GroupMemoryBarrierWithGroupSync();
  return result;
}

RWByteAddressBuffer output : register(u4);

struct tint_symbol_1 {
  uint3 local_id : SV_GroupThreadID;
  uint local_invocation_index : SV_GroupIndex;
  uint3 global_id : SV_DispatchThreadID;
};

void main_inner(uint3 global_id, uint3 local_id, uint local_invocation_index) {
  if ((local_invocation_index < 1u)) {
    sh_atomic_failed = 0u;
  }
  GroupMemoryBarrierWithGroupSync();
  const uint failed = tint_workgroupUniformLoad_sh_atomic_failed();
  if ((local_id.x == 0u)) {
    output.Store(0u, asuint(failed));
  }
}

[numthreads(256, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.global_id, tint_symbol.local_id, tint_symbol.local_invocation_index);
  return;
}
