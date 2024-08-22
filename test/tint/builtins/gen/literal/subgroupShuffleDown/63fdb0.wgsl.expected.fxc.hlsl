SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupShuffleDown_63fdb0() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, (WaveGetLaneIndex() + 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleDown_63fdb0());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleDown_63fdb0());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000023AE2DA0CA0(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

