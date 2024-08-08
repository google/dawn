SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupBroadcast_13f36c() {
  vector<float16_t, 2> res = WaveReadLaneAt((float16_t(1.0h)).xx, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupBroadcast_13f36c());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000027E59DAF440(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

