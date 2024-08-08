SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupBroadcast_5196c8() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_5196c8()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000028954BBF3F0(5,16-40): error X3004: undeclared identifier 'WaveReadLaneAt'

