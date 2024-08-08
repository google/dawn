SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupExclusiveAdd_95e984() {
  vector<float16_t, 4> res = WavePrefixSum((float16_t(1.0h)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupExclusiveAdd_95e984());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000019C4A627D30(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

