SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupMul_f78398() {
  float2 res = WaveActiveProduct((1.0f).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_f78398()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000188FB59CE00(4,16-43): error X3004: undeclared identifier 'WaveActiveProduct'

