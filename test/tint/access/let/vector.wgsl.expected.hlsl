[numthreads(1, 1, 1)]
void main() {
  const float3 v = float3(1.0f, 2.0f, 3.0f);
  const float scalar = v.y;
  const float2 swizzle2 = v.xz;
  const float3 swizzle3 = v.xzy;
  return;
}
