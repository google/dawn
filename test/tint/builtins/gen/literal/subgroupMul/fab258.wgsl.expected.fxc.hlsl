SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupMul_fab258() {
  int4 res = WaveActiveProduct((1).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMul_fab258()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002584C071870(4,14-40): error X3004: undeclared identifier 'WaveActiveProduct'

