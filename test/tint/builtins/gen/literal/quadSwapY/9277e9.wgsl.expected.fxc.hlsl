SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float16_t quadSwapY_9277e9() {
  float16_t res = QuadReadAcrossY(float16_t(1.0h));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, quadSwapY_9277e9());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, quadSwapY_9277e9());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000002A448AB5FC0(3,1-9): error X3000: unrecognized identifier 'float16_t'

