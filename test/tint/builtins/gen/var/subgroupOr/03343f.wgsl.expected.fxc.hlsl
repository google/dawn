SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupOr_03343f() {
  int3 arg_0 = (1).xxx;
  int3 res = asint(WaveActiveBitOr(asuint(arg_0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupOr_03343f()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001F388990A60(5,20-49): error X3004: undeclared identifier 'WaveActiveBitOr'

