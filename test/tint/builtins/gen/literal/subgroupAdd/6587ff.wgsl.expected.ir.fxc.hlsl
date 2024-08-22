SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupAdd_6587ff() {
  uint3 res = WaveActiveSum((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupAdd_6587ff());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupAdd_6587ff());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000014DFDADDD50(4,15-37): error X3004: undeclared identifier 'WaveActiveSum'

