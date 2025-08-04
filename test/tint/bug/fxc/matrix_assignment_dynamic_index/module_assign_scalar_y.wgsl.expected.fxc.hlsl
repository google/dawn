
cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
static float2x4 m1 = float2x4((0.0f).xxxx, (0.0f).xxxx);
[numthreads(1, 1, 1)]
void main() {
  uint v = uniforms[0u].y;
  float4 v_1 = m1[0u];
  float4 v_2 = float4((1.0f).xxxx);
  uint4 v_3 = uint4((v).xxxx);
  m1[0u] = (((v_3 == uint4(0u, 1u, 2u, 3u))) ? (v_2) : (v_1));
}

