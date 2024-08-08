SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupExclusiveAdd_4c8024() {
  float2 res = WavePrefixSum((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_4c8024()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001FCA2BDF3F0(4,16-39): error X3004: undeclared identifier 'WavePrefixSum'

