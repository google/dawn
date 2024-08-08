SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupAnd_4df632() {
  uint arg_0 = 1u;
  uint res = WaveActiveBitAnd(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupAnd_4df632()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001DBF3300A70(5,14-36): error X3004: undeclared identifier 'WaveActiveBitAnd'

