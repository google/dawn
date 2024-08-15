SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 4> subgroupExclusiveMul_a07956() {
  vector<float16_t, 4> res = WavePrefixProduct((float16_t(1.0h)).xxxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupExclusiveMul_a07956());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002299ABC1D50(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

