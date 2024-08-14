SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupAdd_22d041() {
  int3 res = WaveActiveSum((1).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_22d041()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_22d041()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000001DEA7D7F0A0(4,14-35): error X3004: undeclared identifier 'WaveActiveSum'

