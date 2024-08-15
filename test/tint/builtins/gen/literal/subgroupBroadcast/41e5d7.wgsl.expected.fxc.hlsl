SKIP: INVALID

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 3> subgroupBroadcast_41e5d7() {
  vector<float16_t, 3> res = WaveReadLaneAt((float16_t(1.0h)).xxx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 3> >(0u, subgroupBroadcast_41e5d7());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000213C7AF8740(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

