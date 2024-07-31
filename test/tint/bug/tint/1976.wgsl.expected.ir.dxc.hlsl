
Texture2DMS<float4> texture0 : register(t0);
RWByteAddressBuffer results : register(u2);
[numthreads(1, 1, 1)]
void main() {
  Texture2DMS<float4> v = texture0;
  int2 v_1 = int2((0).xx);
  results.Store(0u, asuint(float4(v.Load(v_1, int(0)))[0u]));
}

