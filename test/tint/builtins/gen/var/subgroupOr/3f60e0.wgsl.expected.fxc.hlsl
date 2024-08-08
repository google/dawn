SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupOr_3f60e0() {
  int2 arg_0 = (1).xx;
  int2 res = asint(WaveActiveBitOr(asuint(arg_0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_3f60e0()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000163904B6B50(5,20-49): error X3004: undeclared identifier 'WaveActiveBitOr'

