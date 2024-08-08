SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupAdd_22d041() {
  int3 arg_0 = (1).xxx;
  int3 res = WaveActiveSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_22d041()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000281DEC92CF0(5,14-33): error X3004: undeclared identifier 'WaveActiveSum'

