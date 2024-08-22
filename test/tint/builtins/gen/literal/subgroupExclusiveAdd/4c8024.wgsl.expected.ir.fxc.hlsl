SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupExclusiveAdd_4c8024() {
  float2 res = WavePrefixSum((1.0f).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_4c8024()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupExclusiveAdd_4c8024()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000020745DF46C0(4,16-39): error X3004: undeclared identifier 'WavePrefixSum'

