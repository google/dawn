SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupAdd_3854ae() {
  float res = WaveActiveSum(1.0f);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAdd_3854ae()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001B8C9424340(4,15-33): error X3004: undeclared identifier 'WaveActiveSum'

