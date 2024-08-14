SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float2 subgroupExclusiveAdd_4c8024() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WavePrefixSum(arg_0);
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
C:\src\dawn\Shader@0x00000215F4E2EF20(5,16-35): error X3004: undeclared identifier 'WavePrefixSum'

