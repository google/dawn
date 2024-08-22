SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> subgroupMin_cd3b9d() {
  vector<float16_t, 4> res = WaveActiveMin((float16_t(1.0h)).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupMin_cd3b9d());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupMin_cd3b9d());
}

FXC validation failure:
C:\src\dawn\Shader@0x00000251A9B1BBB0(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

