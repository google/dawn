SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupBroadcastFirst_151e52() {
  float16_t res = WaveReadLaneFirst(float16_t(1.0h));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupBroadcastFirst_151e52());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupBroadcastFirst_151e52());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000016DBC1F6210(3,1-9): error X3000: unrecognized identifier 'float16_t'

