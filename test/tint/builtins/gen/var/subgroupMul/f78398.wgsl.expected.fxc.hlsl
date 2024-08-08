SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float2 subgroupMul_f78398() {
  float2 arg_0 = (1.0f).xx;
  float2 res = WaveActiveProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_f78398()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000018B58DD0AC0(5,16-39): error X3004: undeclared identifier 'WaveActiveProduct'

