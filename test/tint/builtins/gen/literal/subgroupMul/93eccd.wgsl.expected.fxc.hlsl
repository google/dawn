SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupMul_93eccd() {
  float3 res = WaveActiveProduct((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_93eccd()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001952347BF90(4,16-44): error X3004: undeclared identifier 'WaveActiveProduct'

