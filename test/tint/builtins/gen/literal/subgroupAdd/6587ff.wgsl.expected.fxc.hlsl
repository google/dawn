SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupAdd_6587ff() {
  uint3 res = WaveActiveSum((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_6587ff()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000029ADEE61DA0(4,15-37): error X3004: undeclared identifier 'WaveActiveSum'

