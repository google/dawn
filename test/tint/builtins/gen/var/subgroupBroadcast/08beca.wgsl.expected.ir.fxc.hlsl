SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupBroadcast_08beca() {
  float arg_0 = 1.0f;
  float res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupBroadcast_08beca()));
}

FXC validation failure:
c:\src\dawn\Shader@0x00000118CE016020(5,15-39): error X3004: undeclared identifier 'WaveReadLaneAt'

