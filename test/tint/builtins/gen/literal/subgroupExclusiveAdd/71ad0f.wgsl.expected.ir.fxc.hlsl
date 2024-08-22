SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupExclusiveAdd_71ad0f() {
  float4 res = WavePrefixSum((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_71ad0f()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupExclusiveAdd_71ad0f()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000001471ADD46A0(4,16-41): error X3004: undeclared identifier 'WavePrefixSum'

