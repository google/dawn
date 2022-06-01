[numthreads(1, 1, 1)]
void main() {
  const float3 v = float3(1.0f, 2.0f, 3.0f);
  const float scalar = float3(1.0f, 2.0f, 3.0f).y;
  const float2 swizzle2 = float3(1.0f, 2.0f, 3.0f).xz;
  const float3 swizzle3 = float3(1.0f, 2.0f, 3.0f).xzy;
  return;
}
