SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupAdd_28db2c() {
  int4 res = WaveActiveSum((1).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupAdd_28db2c()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupAdd_28db2c()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000234498B0260(4,14-36): error X3004: undeclared identifier 'WaveActiveSum'

