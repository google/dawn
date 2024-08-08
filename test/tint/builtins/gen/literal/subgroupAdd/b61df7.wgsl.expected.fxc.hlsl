SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupAdd_b61df7() {
  uint res = WaveActiveSum(1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_b61df7()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000022108E12040(4,14-30): error X3004: undeclared identifier 'WaveActiveSum'

