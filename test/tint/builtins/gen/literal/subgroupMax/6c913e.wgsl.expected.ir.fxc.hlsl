SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int subgroupMax_6c913e() {
  int res = WaveActiveMax(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_6c913e()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_6c913e()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000002A002AD0150(4,13-28): error X3004: undeclared identifier 'WaveActiveMax'

