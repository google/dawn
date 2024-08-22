SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int2 subgroupAnd_4adc72() {
  int2 tint_tmp = (1).xx;
  int2 res = asint(WaveActiveBitAnd(asuint(tint_tmp)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupAnd_4adc72()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAnd_4adc72()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000017D50A774D0(5,20-53): error X3004: undeclared identifier 'WaveActiveBitAnd'

