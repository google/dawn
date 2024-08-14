SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int3 subgroupOr_03343f() {
  int3 arg = (1).xxx;
  int3 res = asint(WaveActiveBitOr(asuint(arg)));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_03343f()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_03343f()));
}

FXC validation failure:
C:\src\dawn\Shader@0x00000159F0A6DD10(5,20-47): error X3004: undeclared identifier 'WaveActiveBitOr'

