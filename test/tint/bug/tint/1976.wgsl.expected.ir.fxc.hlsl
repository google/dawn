
Texture2DMS<float4> texture0 : register(t0);
RWByteAddressBuffer results : register(u2);
[numthreads(1, 1, 1)]
void main() {
  int2 v = int2((int(0)).xx);
  results.Store(0u, asuint(float4(texture0.Load(v, int(int(0))))[0u]));
}

