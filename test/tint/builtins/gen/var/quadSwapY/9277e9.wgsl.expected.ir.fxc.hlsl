SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
float16_t quadSwapY_9277e9() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t res = QuadReadAcrossY(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, quadSwapY_9277e9());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, quadSwapY_9277e9());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000021526C9AB60(3,1-9): error X3000: unrecognized identifier 'float16_t'

