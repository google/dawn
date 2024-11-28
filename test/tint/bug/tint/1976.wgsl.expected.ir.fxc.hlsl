
Texture2DMS<float4> texture0 : register(t0);
RWByteAddressBuffer results : register(u2);
[numthreads(1, 1, 1)]
void main() {
  uint3 v = (0u).xxx;
  texture0.GetDimensions(v.x, v.y, v.z);
  uint2 v_1 = (v.xy - (1u).xx);
  int2 v_2 = int2(min(uint2((int(0)).xx), v_1));
  results.Store(0u, asuint(float4(texture0.Load(v_2, int(int(0)))).x));
}

