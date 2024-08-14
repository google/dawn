SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float4 subgroupMin_bbd9b0() {
  float4 res = WaveActiveMin((1.0f).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_bbd9b0()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_bbd9b0()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000001D16B11F330(4,16-41): error X3004: undeclared identifier 'WaveActiveMin'

