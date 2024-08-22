SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupShuffleUp_23c7ca() {
  float arg_0 = 1.0f;
  uint arg_1 = 1u;
  float v = arg_0;
  uint v_1 = arg_1;
  float res = WaveReadLaneAt(v, (WaveGetLaneIndex() - v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_23c7ca()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupShuffleUp_23c7ca()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000027272D24620(8,34-51): error X3004: undeclared identifier 'WaveGetLaneIndex'

