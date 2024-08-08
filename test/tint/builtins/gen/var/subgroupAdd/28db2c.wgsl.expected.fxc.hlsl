SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupAdd_28db2c() {
  int4 arg_0 = (1).xxxx;
  int4 res = WaveActiveSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupAdd_28db2c()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000019DD2C63750(5,14-33): error X3004: undeclared identifier 'WaveActiveSum'

