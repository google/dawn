SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float3 subgroupMul_93eccd() {
  float3 res = WaveActiveProduct((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_93eccd()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_93eccd()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000025D44C9F0A0(4,16-44): error X3004: undeclared identifier 'WaveActiveProduct'

