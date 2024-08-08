SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupMul_5a8c86() {
  int3 arg_0 = (1).xxx;
  int3 res = WaveActiveProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupMul_5a8c86()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002DD34B2CA40(5,14-37): error X3004: undeclared identifier 'WaveActiveProduct'

