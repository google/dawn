SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupMin_2493ab() {
  uint arg_0 = 1u;
  uint res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_2493ab()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMin_2493ab()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002099D898D50(5,14-33): error X3004: undeclared identifier 'WaveActiveMin'

