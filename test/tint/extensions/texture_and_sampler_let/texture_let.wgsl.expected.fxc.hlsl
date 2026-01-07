
Texture2D<float4> tex : register(t0);
RWTexture2D<float4> store : register(u0, space1);
void main() {
  float4 res = tex.Load(int3((int(1)).xx, int(0)));
  store[(int(0)).xx] = res;
}

