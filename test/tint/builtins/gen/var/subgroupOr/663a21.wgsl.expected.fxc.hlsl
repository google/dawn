SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint3 subgroupOr_663a21() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveActiveBitOr(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_663a21()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000021C1887D010(5,15-36): error X3004: undeclared identifier 'WaveActiveBitOr'

