SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 quadSwapY_be4e72() {
  int3 arg_0 = (1).xxx;
  int3 res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_be4e72()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(quadSwapY_be4e72()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000240A752BE30(5,14-35): error X3004: undeclared identifier 'QuadReadAcrossY'

