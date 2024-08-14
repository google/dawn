SKIP: FAILED


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  float2x4 m1 = float2x4((0.0f).xxxx, (0.0f).xxxx);
  m1[uniforms[0u].x] = (1.0f).xxxx;
}

FXC validation failure:
C:\src\dawn\Shader@0x000001C5F22D16E0(8,3-20): error X3500: array reference cannot be used as an l-value; not natively addressable

