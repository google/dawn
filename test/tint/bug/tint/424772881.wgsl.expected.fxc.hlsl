struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


RWByteAddressBuffer result : register(u0);
groupshared bool3 wgvar;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wgvar = (false).xxx;
  }
  GroupMemoryBarrierWithGroupSync();
  bool3 v = wgvar;
  wgvar = v;
  bool e = wgvar.x;
  wgvar.y = e;
  GroupMemoryBarrierWithGroupSync();
  result.Store3(0u, uint3(wgvar));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

