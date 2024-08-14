SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupXor_83b1f3() {
  int4 arg_0 = (1).xxxx;
  int4 res = asint(WaveActiveBitXor(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_83b1f3()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupXor_83b1f3()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000026AA8139140(5,20-50): error X3004: undeclared identifier 'WaveActiveBitXor'

