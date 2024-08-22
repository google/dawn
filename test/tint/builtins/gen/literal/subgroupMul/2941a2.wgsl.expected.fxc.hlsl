SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupMul_2941a2() {
  float16_t res = WaveActiveProduct(float16_t(1.0h));
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, subgroupMul_2941a2());
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupMul_2941a2());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000238002AF970(3,1-9): error X3000: unrecognized identifier 'float16_t'

