SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupShuffleDown_7f8886() {
  float res = WaveReadLaneAt(1.0f, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_7f8886()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_7f8886()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000002126B3F5C40(4,37-54): error X3004: undeclared identifier 'WaveGetLaneIndex'

