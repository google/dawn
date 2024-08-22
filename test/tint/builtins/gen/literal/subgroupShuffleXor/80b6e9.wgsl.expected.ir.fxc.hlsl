SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupShuffleXor_80b6e9() {
  uint res = WaveReadLaneAt(1u, (WaveGetLaneIndex() ^ 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupShuffleXor_80b6e9());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupShuffleXor_80b6e9());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000027B93CA4E60(4,34-51): error X3004: undeclared identifier 'WaveGetLaneIndex'

