SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupExclusiveMul_e88d1c() {
  vector<float16_t, 2> res = WavePrefixProduct((float16_t(1.0h)).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupExclusiveMul_e88d1c());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000022F6AC68B40(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

