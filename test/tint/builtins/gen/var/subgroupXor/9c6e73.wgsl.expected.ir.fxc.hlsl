SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupXor_9c6e73() {
  int3 arg_0 = (1).xxx;
  int3 res = asint(WaveActiveBitXor(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupXor_9c6e73()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupXor_9c6e73()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000001D5345D4690(5,20-50): error X3004: undeclared identifier 'WaveActiveBitXor'

