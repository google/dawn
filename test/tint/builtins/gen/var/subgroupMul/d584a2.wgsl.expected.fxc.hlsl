SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupMul_d584a2() {
  int2 arg_0 = (1).xx;
  int2 res = WaveActiveProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_d584a2()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001A764FA2270(5,14-37): error X3004: undeclared identifier 'WaveActiveProduct'

