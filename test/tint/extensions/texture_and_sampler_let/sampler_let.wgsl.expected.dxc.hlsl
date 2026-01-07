
Texture2D<float4> tex : register(t0);
SamplerState sam : register(s1);
RWTexture2D<float4> store : register(u0, space1);
void main() {
  float4 res = tex.SampleLevel(sam, (1.0f).xx, 0.0f, (int(1)).xx);
  store[(int(0)).xx] = res;
}

