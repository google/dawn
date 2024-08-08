SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupXor_9c6e73() {
  int3 tint_tmp = (1).xxx;
  int3 res = asint(WaveActiveBitXor(asuint(tint_tmp)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupXor_9c6e73()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001B02A250E00(5,20-53): error X3004: undeclared identifier 'WaveActiveBitXor'

