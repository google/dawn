SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float subgroupMax_1a1a5f() {
  float res = WaveActiveMax(1.0f);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_1a1a5f()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(subgroupMax_1a1a5f()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000017018EF8FC0(4,15-33): error X3004: undeclared identifier 'WaveActiveMax'

