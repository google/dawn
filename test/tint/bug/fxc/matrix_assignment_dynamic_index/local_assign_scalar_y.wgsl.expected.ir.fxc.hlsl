
cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
static float2x4 m1 = float2x4((0.0f).xxxx, (0.0f).xxxx);
[numthreads(1, 1, 1)]
void main() {
  float4 v = m1[int(0)];
  float4 v_1 = uniforms[0u].y.xxxx;
  m1[int(0)] = (((v_1 == float4(int(0), int(1), int(2), int(3)))) ? (1.0f.xxxx) : (v));
}

