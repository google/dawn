SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupAdd_22d041() {
  int3 res = WaveActiveSum((1).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_22d041()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002FC41D5EFF0(4,14-35): error X3004: undeclared identifier 'WaveActiveSum'

