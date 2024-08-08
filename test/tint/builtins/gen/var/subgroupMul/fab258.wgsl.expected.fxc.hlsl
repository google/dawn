SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupMul_fab258() {
  int4 arg_0 = (1).xxxx;
  int4 res = WaveActiveProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_fab258()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000026A1C714740(5,14-37): error X3004: undeclared identifier 'WaveActiveProduct'

