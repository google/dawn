SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

uint2 subgroupAdd_1280c8() {
  uint2 arg_0 = (1u).xx;
  uint2 res = WaveActiveSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(subgroupAdd_1280c8()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002CC18BBFCC0(5,15-34): error X3004: undeclared identifier 'WaveActiveSum'

