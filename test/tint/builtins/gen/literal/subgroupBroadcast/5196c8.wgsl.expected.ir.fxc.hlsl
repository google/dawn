SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupBroadcast_5196c8() {
  float2 res = WaveReadLaneAt((1.0f).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupBroadcast_5196c8()));
}

FXC validation failure:
c:\src\dawn\Shader@0x00000143556D38F0(4,16-44): error X3004: undeclared identifier 'WaveReadLaneAt'

