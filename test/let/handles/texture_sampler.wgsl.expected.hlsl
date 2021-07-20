Texture2D<float4> t : register(t0, space0);
SamplerState s : register(s1, space0);

void main() {
  const Texture2D<float4> x = t;
  const SamplerState a = s;
  1;
  const Texture2D<float4> y = x;
  const SamplerState b = a;
  2;
  y.Sample(b, float2(1.0f, 2.0f));
  return;
}
