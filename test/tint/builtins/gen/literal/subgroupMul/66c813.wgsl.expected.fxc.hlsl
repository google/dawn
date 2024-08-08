SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float4 subgroupMul_66c813() {
  float4 res = WaveActiveProduct((1.0f).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_66c813()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001E48CF7C360(4,16-45): error X3004: undeclared identifier 'WaveActiveProduct'

