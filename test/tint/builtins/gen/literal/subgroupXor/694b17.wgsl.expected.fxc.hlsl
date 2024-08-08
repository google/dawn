SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int subgroupXor_694b17() {
  int res = asint(WaveActiveBitXor(asuint(1)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupXor_694b17()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000270803DB1B0(4,19-45): error X3004: undeclared identifier 'WaveActiveBitXor'

