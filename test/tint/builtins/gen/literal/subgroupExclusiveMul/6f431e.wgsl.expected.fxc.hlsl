SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

float16_t subgroupExclusiveMul_6f431e() {
  float16_t res = WavePrefixProduct(float16_t(1.0h));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, subgroupExclusiveMul_6f431e());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000021D20F50BB0(3,1-9): error X3000: unrecognized identifier 'float16_t'

