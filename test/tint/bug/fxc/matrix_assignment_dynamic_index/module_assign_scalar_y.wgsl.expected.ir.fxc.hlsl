SKIP: FAILED


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
static float2x4 m1 = float2x4((0.0f).xxxx, (0.0f).xxxx);
[numthreads(1, 1, 1)]
void main() {
  m1[0][uniforms[0u].y] = 1.0f;
}

FXC validation failure:
<scrubbed_path>(8,3-23): error X3500: array reference cannot be used as an l-value; not natively addressable


tint executable returned error: exit status 1
