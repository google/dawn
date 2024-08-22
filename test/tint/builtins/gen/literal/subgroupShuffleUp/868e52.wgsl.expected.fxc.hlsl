SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupShuffleUp_868e52() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, (WaveGetLaneIndex() - 1u));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleUp_868e52());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupShuffleUp_868e52());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001C8A7F25CF0(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

