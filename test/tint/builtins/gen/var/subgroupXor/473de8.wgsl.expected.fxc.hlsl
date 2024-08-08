SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupXor_473de8() {
  int2 arg_0 = (1).xx;
  int2 res = asint(WaveActiveBitXor(asuint(arg_0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupXor_473de8()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000023FB7462020(5,20-50): error X3004: undeclared identifier 'WaveActiveBitXor'

