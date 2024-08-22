SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint4 subgroupXor_9d77e4() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveBitXor(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_9d77e4()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_9d77e4()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001FA5C5B7BD0(5,15-37): error X3004: undeclared identifier 'WaveActiveBitXor'

