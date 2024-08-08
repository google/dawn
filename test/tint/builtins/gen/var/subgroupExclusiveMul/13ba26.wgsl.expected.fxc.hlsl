SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupExclusiveMul_13ba26() {
  vector<float16_t, 3> arg_0 = (float16_t(1.0h)).xxx;
  vector<float16_t, 3> res = WavePrefixProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupExclusiveMul_13ba26());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002043BD7D1A0(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

