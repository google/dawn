SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float3 quadSwapY_d1ab4d() {
  float3 res = QuadReadAcrossY((1.0f).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_d1ab4d()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_d1ab4d()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000014F1A465CD0(4,16-42): error X3004: undeclared identifier 'QuadReadAcrossY'

