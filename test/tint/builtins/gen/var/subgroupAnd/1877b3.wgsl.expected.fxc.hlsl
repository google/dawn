SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupAnd_1877b3() {
  int3 arg_0 = (1).xxx;
  int3 res = asint(WaveActiveBitAnd(asuint(arg_0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAnd_1877b3()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002D167C11700(5,20-50): error X3004: undeclared identifier 'WaveActiveBitAnd'

