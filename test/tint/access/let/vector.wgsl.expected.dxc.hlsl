
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void main() {
  float3 v = float3(1.0f, 2.0f, 3.0f);
  float scalar = v.y;
  float2 swizzle2 = v.xz;
  float3 swizzle3 = v.xzy;
  float3 v_1 = float3((scalar).xxx);
  s.Store3(0u, asuint(((v_1 + float3(swizzle2, 1.0f)) + swizzle3)));
}

