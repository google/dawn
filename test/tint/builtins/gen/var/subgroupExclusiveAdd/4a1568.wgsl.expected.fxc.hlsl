SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupExclusiveAdd_4a1568() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t res = WavePrefixSum(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupExclusiveAdd_4a1568());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002893BF7CE00(3,1-9): error X3000: unrecognized identifier 'float16_t'

