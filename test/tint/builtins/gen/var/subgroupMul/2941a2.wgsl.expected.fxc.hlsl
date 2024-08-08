SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupMul_2941a2() {
  float16_t arg_0 = float16_t(1.0h);
  float16_t res = WaveActiveProduct(arg_0);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupMul_2941a2());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000011FF4B81950(3,1-9): error X3000: unrecognized identifier 'float16_t'

