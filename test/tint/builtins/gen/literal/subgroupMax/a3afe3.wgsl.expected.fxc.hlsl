SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupMax_a3afe3() {
  vector<float16_t, 2> res = WaveActiveMax((float16_t(1.0h)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupMax_a3afe3());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupMax_a3afe3());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002F2D466FB70(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

