SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupMul_53aee2() {
  vector<float16_t, 3> res = WaveActiveProduct((float16_t(1.0h)).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupMul_53aee2());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000025DDFFBF430(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

