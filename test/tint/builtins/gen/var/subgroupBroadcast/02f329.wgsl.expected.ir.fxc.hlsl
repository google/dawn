SKIP: INVALID


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 4> subgroupBroadcast_02f329() {
  vector<float16_t, 4> arg_0 = (float16_t(1.0h)).xxxx;
  vector<float16_t, 4> res = WaveReadLaneAt(arg_0, int(1));
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupBroadcast_02f329());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 4> >(0u, subgroupBroadcast_02f329());
}

FXC validation failure:
<scrubbed_path>(3,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
