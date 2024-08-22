SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupAdd_ba53f9() {
  int res = WaveActiveSum(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_ba53f9()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_ba53f9()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000001C73484E8F0(4,13-28): error X3004: undeclared identifier 'WaveActiveSum'

