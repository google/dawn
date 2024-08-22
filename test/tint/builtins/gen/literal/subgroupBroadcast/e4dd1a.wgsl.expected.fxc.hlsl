SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

vector<float16_t, 2> subgroupBroadcast_e4dd1a() {
  vector<float16_t, 2> res = WaveReadLaneAt((float16_t(1.0h)).xx, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupBroadcast_e4dd1a());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupBroadcast_e4dd1a());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000266EC52E1D0(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

