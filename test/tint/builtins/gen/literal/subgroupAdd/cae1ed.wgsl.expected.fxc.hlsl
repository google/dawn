SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupAdd_cae1ed() {
  vector<float16_t, 2> res = WaveActiveSum((float16_t(1.0h)).xx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupAdd_cae1ed());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000022F6A42F430(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

