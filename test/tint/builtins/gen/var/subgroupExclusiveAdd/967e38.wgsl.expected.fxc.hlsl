SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float subgroupExclusiveAdd_967e38() {
  float arg_0 = 1.0f;
  float res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupExclusiveAdd_967e38()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002CD1519A860(5,15-34): error X3004: undeclared identifier 'WavePrefixSum'

