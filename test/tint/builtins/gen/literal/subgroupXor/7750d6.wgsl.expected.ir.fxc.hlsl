SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint subgroupXor_7750d6() {
  uint res = WaveActiveBitXor(1u);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, subgroupXor_7750d6());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, subgroupXor_7750d6());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000022EE19590C0(4,14-33): error X3004: undeclared identifier 'WaveActiveBitXor'

