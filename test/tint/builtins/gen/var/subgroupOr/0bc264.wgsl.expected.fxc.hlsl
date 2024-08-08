SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupOr_0bc264() {
  uint arg_0 = 1u;
  uint res = WaveActiveBitOr(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupOr_0bc264()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000276B3B416D0(5,14-35): error X3004: undeclared identifier 'WaveActiveBitOr'

