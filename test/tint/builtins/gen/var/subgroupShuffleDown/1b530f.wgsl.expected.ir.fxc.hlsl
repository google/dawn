SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupShuffleDown_1b530f() {
  int3 arg_0 = (1).xxx;
  uint arg_1 = 1u;
  int3 v = arg_0;
  uint v_1 = arg_1;
  int3 res = WaveReadLaneAt(v, (WaveGetLaneIndex() + v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleDown_1b530f()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupShuffleDown_1b530f()));
}

FXC validation failure:
C:\src\dawn\Shader@0x00000141E4AE4800(8,33-50): error X3004: undeclared identifier 'WaveGetLaneIndex'

