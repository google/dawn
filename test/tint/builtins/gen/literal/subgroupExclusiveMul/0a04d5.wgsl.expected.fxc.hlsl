SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupExclusiveMul_0a04d5() {
  float3 res = WavePrefixProduct((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupExclusiveMul_0a04d5()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001566884C550(4,16-44): error X3004: undeclared identifier 'WavePrefixProduct'

