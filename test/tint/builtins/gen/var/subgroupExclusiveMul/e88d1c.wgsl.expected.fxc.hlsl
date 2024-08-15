SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupExclusiveMul_e88d1c() {
  vector<float16_t, 2> arg_0 = (float16_t(1.0h)).xx;
  vector<float16_t, 2> res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupExclusiveMul_e88d1c());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000143BC5A8A30(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

