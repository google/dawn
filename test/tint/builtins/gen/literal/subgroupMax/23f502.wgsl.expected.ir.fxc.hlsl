SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupMax_23f502() {
  uint3 res = WaveActiveMax((1u).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupMax_23f502());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupMax_23f502());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000022F44FB9140(4,15-37): error X3004: undeclared identifier 'WaveActiveMax'

