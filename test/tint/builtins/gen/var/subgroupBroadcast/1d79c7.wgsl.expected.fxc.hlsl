SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupBroadcast_1d79c7() {
  int arg_0 = 1;
  int res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_1d79c7()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002205DFD0120(5,13-37): error X3004: undeclared identifier 'WaveReadLaneAt'

