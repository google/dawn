
Texture2DMS<float4> texture0 : register(t0);
RWByteAddressBuffer results : register(u2);
[numthreads(1, 1, 1)]
void main() {
  uint3 v = (0u).xxx;
  texture0.GetDimensions(v.x, v.y, v.z);
  results.Store(0u, asuint(texture0.Load((int(0)).xx, int(min(0u, (v.z - 1u)))).x));
}

