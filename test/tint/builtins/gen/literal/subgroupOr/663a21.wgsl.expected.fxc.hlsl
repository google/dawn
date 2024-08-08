SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupOr_663a21() {
  uint3 res = WaveActiveBitOr((1u).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_663a21()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000244C44786C0(4,15-39): error X3004: undeclared identifier 'WaveActiveBitOr'

