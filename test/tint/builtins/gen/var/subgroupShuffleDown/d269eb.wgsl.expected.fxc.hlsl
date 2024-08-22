SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupShuffleDown_d269eb() {
  int arg_0 = 1;
  uint arg_1 = 1u;
  int res = WaveReadLaneAt(arg_0, (WaveGetLaneIndex() + arg_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_d269eb()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleDown_d269eb()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000019D9F79BBD0(6,36-53): error X3004: undeclared identifier 'WaveGetLaneIndex'

