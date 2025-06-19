groupshared bool3 wgvar;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    wgvar = (false).xxx;
  }
  GroupMemoryBarrierWithGroupSync();
}

RWByteAddressBuffer result : register(u0);

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  bool3 v = wgvar;
  wgvar = v;
  bool e = wgvar[0];
  wgvar[1] = e;
  GroupMemoryBarrierWithGroupSync();
  result.Store3(0u, asuint(uint3(wgvar)));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
