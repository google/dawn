Texture2D<float4> t : register(t0, space1);
SamplerState s : register(s1, space1);

void main() {
  float4 res = t.GatherRed(s, float2(0.0f, 0.0f));
  return;
}
