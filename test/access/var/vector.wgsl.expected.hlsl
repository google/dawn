[numthreads(1, 1, 1)]
void main() {
  float3 v = float3(0.0f, 0.0f, 0.0f);
  const float scalar = v.y;
  const float2 swizzle2 = v.xz;
  const float3 swizzle3 = v.xzy;
  return;
}
