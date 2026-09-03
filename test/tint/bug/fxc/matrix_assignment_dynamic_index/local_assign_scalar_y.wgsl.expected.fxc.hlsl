
cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
static float2x4 m1 = float2x4((0.0f).xxxx, (0.0f).xxxx);
[numthreads(1, 1, 1)]
void main() {
  float4 v = m1[int(0)];
  m1[int(0)] = (((uint4((uniforms[0u].y).xxxx) == uint4(0u, 1u, 2u, 3u))) ? ((1.0f).xxxx) : (v));
}

