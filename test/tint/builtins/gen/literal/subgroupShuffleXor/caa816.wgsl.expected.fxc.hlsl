SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupShuffleXor_caa816() {
  float3 res = WaveReadLaneAt((1.0f).xxx, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleXor_caa816()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleXor_caa816()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000025B82AF0DF0(4,44-61): error X3004: undeclared identifier 'WaveGetLaneIndex'

