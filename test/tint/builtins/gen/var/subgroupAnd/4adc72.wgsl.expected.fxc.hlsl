SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupAnd_4adc72() {
  int2 arg_0 = (1).xx;
  int2 res = asint(WaveActiveBitAnd(asuint(arg_0)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAnd_4adc72()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001D7E87F1CA0(5,20-50): error X3004: undeclared identifier 'WaveActiveBitAnd'

