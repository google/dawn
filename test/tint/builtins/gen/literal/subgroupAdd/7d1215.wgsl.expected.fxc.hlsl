SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float3 subgroupAdd_7d1215() {
  float3 res = WaveActiveSum((1.0f).xxx);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupAdd_7d1215()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001CCCB8544D0(4,16-40): error X3004: undeclared identifier 'WaveActiveSum'

