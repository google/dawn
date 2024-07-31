SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupBroadcast_08beca() {
  float res = WaveReadLaneAt(1.0f, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_08beca()));
}

FXC validation failure:
c:\src\dawn\Shader@0x0000020683E50380(4,15-38): error X3004: undeclared identifier 'WaveReadLaneAt'

