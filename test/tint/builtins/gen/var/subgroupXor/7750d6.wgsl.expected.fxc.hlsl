SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint subgroupXor_7750d6() {
  uint arg_0 = 1u;
  uint res = WaveActiveBitXor(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_7750d6()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000020C9D4871C0(5,14-36): error X3004: undeclared identifier 'WaveActiveBitXor'

