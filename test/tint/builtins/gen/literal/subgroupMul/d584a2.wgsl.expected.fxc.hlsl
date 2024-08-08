SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupMul_d584a2() {
  int2 res = WaveActiveProduct((1).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_d584a2()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000020E62ABFFC0(4,14-38): error X3004: undeclared identifier 'WaveActiveProduct'

