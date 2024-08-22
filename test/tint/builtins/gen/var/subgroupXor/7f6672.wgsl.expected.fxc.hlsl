SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupXor_7f6672() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveActiveBitXor(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupXor_7f6672()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupXor_7f6672()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000024659D37020(5,15-37): error X3004: undeclared identifier 'WaveActiveBitXor'

