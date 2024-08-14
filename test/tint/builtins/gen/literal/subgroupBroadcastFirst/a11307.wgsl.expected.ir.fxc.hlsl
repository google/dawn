SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
vector<float16_t, 2> subgroupBroadcastFirst_a11307() {
  vector<float16_t, 2> res = WaveReadLaneFirst((float16_t(1.0h)).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupBroadcastFirst_a11307());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<vector<float16_t, 2> >(0u, subgroupBroadcastFirst_a11307());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000022031DA1C40(3,8-16): error X3000: syntax error: unexpected token 'float16_t'

