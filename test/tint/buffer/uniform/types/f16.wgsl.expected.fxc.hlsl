SKIP: FAILED

cbuffer cbuffer_u : register(b0, space0) {
  uint4 u[1];
};

[numthreads(1, 1, 1)]
void main() {
  const float16_t x = float16_t(f16tof32(((u[0].x) & 0xFFFF)));
  return;
}
FXC validation failure:
D:\Projects\RampUp\dawn\test\tint\buffer\Shader@0x000001B3B36433A0(7,9-17): error X3000: unrecognized identifier 'float16_t'

