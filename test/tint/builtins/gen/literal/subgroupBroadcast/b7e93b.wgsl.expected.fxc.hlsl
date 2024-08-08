SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupBroadcast_b7e93b() {
  float4 res = WaveReadLaneAt((1.0f).xxxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_b7e93b()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002A4C62002F0(4,16-46): error X3004: undeclared identifier 'WaveReadLaneAt'

