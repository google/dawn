SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupShuffle_5dfeab() {
  float4 arg_0 = (1.0f).xxxx;
  int arg_1 = 1;
  float4 res = WaveReadLaneAt(arg_0, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_5dfeab()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupShuffle_5dfeab()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001C1062A60F0(6,16-43): error X3004: undeclared identifier 'WaveReadLaneAt'

