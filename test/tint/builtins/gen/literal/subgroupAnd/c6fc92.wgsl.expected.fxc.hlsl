SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupAnd_c6fc92() {
  int res = asint(WaveActiveBitAnd(asuint(1)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAnd_c6fc92()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001D230F277C0(4,19-45): error X3004: undeclared identifier 'WaveActiveBitAnd'

