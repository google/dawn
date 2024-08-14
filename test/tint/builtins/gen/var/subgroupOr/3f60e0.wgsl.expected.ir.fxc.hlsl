SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int2 subgroupOr_3f60e0() {
  int2 arg_0 = (1).xx;
  int2 res = asint(WaveActiveBitOr(asuint(arg_0)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_3f60e0()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupOr_3f60e0()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000017C61388F30(5,20-49): error X3004: undeclared identifier 'WaveActiveBitOr'

