SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupMul_d584a2() {
  int2 res = WaveActiveProduct((1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_d584a2()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupMul_d584a2()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000025D2EE5F6A0(4,14-38): error X3004: undeclared identifier 'WaveActiveProduct'

