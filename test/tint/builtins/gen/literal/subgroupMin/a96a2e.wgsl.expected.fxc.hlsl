SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupMin_a96a2e() {
  int res = WaveActiveMin(1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_a96a2e()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_a96a2e()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001EF2534B0B0(4,13-28): error X3004: undeclared identifier 'WaveActiveMin'

