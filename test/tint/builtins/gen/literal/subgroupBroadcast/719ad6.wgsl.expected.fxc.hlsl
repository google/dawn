SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupBroadcast_719ad6() {
  float16_t res = WaveReadLaneAt(float16_t(1.0h), 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupBroadcast_719ad6());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupBroadcast_719ad6());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000027117026390(3,1-9): error X3000: unrecognized identifier 'float16_t'

